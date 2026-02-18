#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2026 gr-mlir_aie author.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

#TODO0: generalize better
#TODO1: make vector size adjustable
#TODO2: implement forecast() (currently left as 1:1)

import numpy
from gnuradio import gr

from aie.dialects.aie import *
from aie.extras.context import mlir_mod_ctx
from aie.iron import Program, Runtime, Worker, ObjectFifo
from aie.iron.placers import SequentialPlacer
from aie.iron.controlflow import range_

import aie.iron as iron
from aie.utils import NPUKernel, DefaultNPURuntime, get_current_device

DEV = iron.get_current_device()
DTYPE=np.int8

class mlir_aie_python_uint8(gr.basic_block):
    """
    You know the rules and so do I 
    """
    def __init__(self, path_xclbin="aie-kernel-src/build/final.xclbin",path_insts_bin="aie-kernel-src/build/insts.bin", VECTOR_SIZE = 16384):
        gr.basic_block.__init__(self,
            name="mlir_aie_python_uint8",
            in_sig=[DTYPE],
            out_sig=[DTYPE])
            
        npu_kernel = NPUKernel(
            path_xclbin,
            path_insts_bin,
            kernel_name="MLIR_AIE",
        )
         
        self.kernel_handle = DefaultNPURuntime.load(npu_kernel)
        self.out_buf = iron.zeros(VECTOR_SIZE, dtype=DTYPE)
        self.VECTOR_SIZE = VECTOR_SIZE

    #TODO implement forecast()
    def forecast(self, noutput_items, ninputs):
        ninput_items_required = [min(self.VECTOR_SIZE, noutput_items)] * ninputs
        return ninput_items_required

    def general_work(self, input_items, output_items):
        if(len(input_items[0]) < self.VECTOR_SIZE):
            return 0
        buffers = [iron.tensor(input_items[0][:self.VECTOR_SIZE], dtype=DTYPE), self.out_buf] 
        DefaultNPURuntime.run(self.kernel_handle, buffers)        
        output_items[0][:self.VECTOR_SIZE] = self.out_buf.numpy()
        
        self.consume_each(self.VECTOR_SIZE)
        return self.VECTOR_SIZE

