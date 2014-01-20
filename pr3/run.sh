export NVCC=/usr/bin/nvcc
export GCC=/usr/bin/gcc
export GENCODE_SM10=""
export GENCODE_SM20=""
export GENCODE_SM30=""
export LDFLAGS="-lcuda -lcudart"
export CUDA_PATH=""
export CUDA_INC_PATH="/usr/include/" 

make -C matrixMul/0_Simple/matrixMul/
