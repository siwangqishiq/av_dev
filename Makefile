CC := g++

DIR := .
SRC_DIR := src
ASSET_DIR := assets
BUILD_DIR := build
INCLUDE_DIR := include
STD := c++14
OPTPARAMS := -O1

.PHONY: clean

build_dir:
	mkdir -p ${BUILD_DIR}

compile: build_dir \
		${BUILD_DIR}/main.o

${BUILD_DIR}/main.o:${SRC_DIR}/main.cpp
	${CC} -std=${STD} -c ${SRC_DIR}/main.cpp \
		-o ${BUILD_DIR}/main.o \
		-I ${INCLUDE_DIR} \
		-I ${SRC_DIR}
	
link:compile
	${CC} ${BUILD_DIR}/*.o \
		-Llib \
		-lavformat.dll -lavcodec.dll -lavfilter.dll -lavutil.dll -lswscale.dll -lswresample.dll \
		-lavdevice.dll \
		-o ${BUILD_DIR}/app

run:link
	${BUILD_DIR}/app

clean:
	rm -f ${BUILD_DIR}/*.o 
	rm -f ${BUILD_DIR}/app

