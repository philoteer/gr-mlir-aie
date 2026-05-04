# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT mlir_aie_sources)
    message(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT mlir_aie_sources)

########################################################################
# Check for pygccxml
########################################################################
gr_python_check_module_raw("pygccxml" "import pygccxml" PYGCCXML_FOUND)

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND mlir_aie_python_files
    mlir_aie_cpp_uint8_python.cc
    mlir_aie_cpp_int32_python.cc
    mlir_aie_cpp_bfloat16_python.cc
    mlir_aie_cpp_int16_python.cc
    float32_to_bfloat16_python.cc
    bfloat16_to_float32_python.cc
    complex64_to_cbfloat_python.cc
    cbfloat_to_complex64_python.cc
    mlire_aie_cpp_int32_to_int16_python.cc python_bindings.cc)

gr_pybind_make_oot(mlir_aie ../../.. gr::mlir_aie "${mlir_aie_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(
    TARGET mlir_aie_python
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mlir_aie_python>
            ${PROJECT_BINARY_DIR}/test_modules/gnuradio/mlir_aie/)

install(
    TARGETS mlir_aie_python
    DESTINATION ${GR_PYTHON_DIR}/gnuradio/mlir_aie
    COMPONENT pythonapi)
