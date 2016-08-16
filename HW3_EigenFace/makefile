
# makefile used to build OpenCV Demo

all: predict trainer

predict: predict.cpp
	g++ -o predict predict.cpp `pkg-config ！-cflags ！-libs opencv`

trainer: trainer.cpp
	g++ -o trainer trainer.cpp `pkg-config ！-cflags ！-libs opencv`