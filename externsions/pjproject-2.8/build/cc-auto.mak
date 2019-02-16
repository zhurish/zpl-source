ifeq ($(ARCH_TYPE),MIPS)
export CC = $(CROSS_COMPILE)gcc -c
export CXX = $(CROSS_COMPILE)g++ -c
#export AR = ar
#export AR_FLAGS = rv
export LD = $(CROSS_COMPILE)gcc
#export LDOUT = -o 
#export RANLIB = ranlib
else
export CC = gcc -c
export CXX = g++ -c
export AR = ar
export AR_FLAGS = rv
export LD = gcc
export LDOUT = -o 
export RANLIB = ranlib
endif
export OBJEXT := .o
export LIBEXT := .a
export LIBEXT2 := 

export CC_OUT := -o 
export CC_INC := -I
export CC_DEF := -D
export CC_OPTIMIZE := -O2
export CC_LIB := -l

export CC_SOURCES :=
export CC_CFLAGS := -Wall
export CC_LDFLAGS :=

