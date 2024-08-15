CC := g++

DIR := .
SRC_DIR := .
BUILD_DIR := ./build
INCLUDE_DIR := ../include
STD := c++14
# OPTPARAMS := -O1

.PHONY: clean

build_dir:
	mkdir -p ${BUILD_DIR}

compile: build_dir \
		${BUILD_DIR}/play_audio.o

${BUILD_DIR}/play_audio.o:${SRC_DIR}/play_audio.cpp
	${CC} -std=${STD} -c ${SRC_DIR}/play_audio.cpp \
		-o ${BUILD_DIR}/play_audio.o \
		-I ${INCLUDE_DIR} \
		-I ${SRC_DIR}
	
link:compile
	${CC} ${BUILD_DIR}/play_audio.o \
		-Llib \
		-lavformat.dll -lavcodec.dll -lavfilter.dll -lavutil.dll -lswscale.dll -lswresample.dll \
		-lavdevice.dll \
		-o ${BUILD_DIR}/app

run:link
	${BUILD_DIR}/app E:/assets/mv/wanzi.mp4

clean:
	rm -f ${BUILD_DIR}/*.o 
	rm -f ${BUILD_DIR}/app

