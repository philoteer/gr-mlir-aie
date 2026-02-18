# CMake generated Testfile for 
# Source directory: /media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/python/mlir_aie
# Build directory: /media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/build/python/mlir_aie
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(qa_mlir_aie_python_uint8 "/usr/bin/sh" "qa_mlir_aie_python_uint8_test.sh")
set_tests_properties(qa_mlir_aie_python_uint8 PROPERTIES  _BACKTRACE_TRIPLES "/usr/lib/x86_64-linux-gnu/cmake/gnuradio/GrTest.cmake;119;add_test;/media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/python/mlir_aie/CMakeLists.txt;38;GR_ADD_TEST;/media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/python/mlir_aie/CMakeLists.txt;0;")
subdirs("bindings")
