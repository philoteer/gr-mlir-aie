##===- Makefile -----------------------------------------------------------===##
# 
# This file licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Copyright (C) 2024, Advanced Micro Devices, Inc.
# 
##===----------------------------------------------------------------------===##

srcdir := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

######## Makefile common end
# VITIS related variables
AIETOOLS_DIR ?= $(shell realpath $(dir $(shell which xchesscc))/../)
AIE_INCLUDE_DIR ?= ${AIETOOLS_DIR}/data/versal_prod/lib
AIE2_INCLUDE_DIR ?= ${AIETOOLS_DIR}/data/aie_ml/lib

MLIR_AIE_DIR ?= $(shell python3 -c "from aie.utils.config import root_path; print(root_path())")

WARNING_FLAGS = -Wno-parentheses -Wno-attributes -Wno-macro-redefined -Wno-empty-body -Wno-missing-template-arg-list-after-template-kw

CHESSCC1_FLAGS = -f -p me -P ${AIE_INCLUDE_DIR} -I ${AIETOOLS_DIR}/include
CHESSCC2_FLAGS = -f -p me -P ${AIE2_INCLUDE_DIR} -I ${AIETOOLS_DIR}/include
CHESS_FLAGS = -P ${AIE_INCLUDE_DIR}

CHESSCCWRAP1_FLAGS = aie -I ${AIETOOLS_DIR}/include
CHESSCCWRAP2_FLAGS = aie2 -I ${AIETOOLS_DIR}/include
CHESSCCWRAP2P_FLAGS = aie2p -I ${AIETOOLS_DIR}/include
PEANOWRAP2_FLAGS = -O2 -std=c++20 --target=aie2-none-unknown-elf ${WARNING_FLAGS} -DNDEBUG -I ${MLIR_AIE_DIR}/include
PEANOWRAP2P_FLAGS = -O2 -std=c++20 --target=aie2p-none-unknown-elf ${WARNING_FLAGS} -DNDEBUG -I ${MLIR_AIE_DIR}/include

TEST_POWERSHELL := $(shell command -v powershell.exe >/dev/null 2>&1 && echo yes || echo no)
ifeq ($(TEST_POWERSHELL),yes)
	powershell = powershell.exe
	getwslpath = wslpath -w
else
	powershell =
	getwslpath = echo
endif

######## Makefile common end

devicename ?= $(if $(filter 1,$(NPU2)),npu2,npu) 	#npu2
VPATH := ${srcdir}/../../../aie_kernels/generic
data_size = 4096
PASSTHROUGH_SIZE = ${data_size}

aie_py_src=passthrough_pykernel.py

.PHONY: all template clean
.DEFAULT_GOAL := compile

all: build/final.xclbin

build/aie.mlir: ${srcdir}/${aie_py_src}
	mkdir -p ${@D}
	python3 $<  > $@

build/final.xclbin: build/aie.mlir
	mkdir -p ${@D}
	cd ${@D} && aiecc.py --aie-generate-xclbin --aie-generate-npu-insts --no-compile-host \
		--no-xchesscc --no-xbridge \
		--xclbin-name=${@F} --npu-insts-name=insts.bin $(<:%=../%)

clean:
	rm -rf build _build

compile: clean build/final.xclbin build/insts.bin

