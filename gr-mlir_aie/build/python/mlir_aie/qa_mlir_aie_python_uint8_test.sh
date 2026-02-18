#!/usr/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/python/mlir_aie
export GR_CONF_CONTROLPORT_ON=False
export PATH="/media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/build/python/mlir_aie":"$PATH"
export LD_LIBRARY_PATH="":$LD_LIBRARY_PATH
export PYTHONPATH=/media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/build/test_modules:$PYTHONPATH
/usr/bin/python3 /media/piloteer/SAMSUNG_EXT/XDNA/examples/gr-mlir_aie/python/mlir_aie/qa_mlir_aie_python_uint8.py 
