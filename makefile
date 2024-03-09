.PHONY: build run

build:
	sudo gcc main.c -o shellp
	sudo chmod +x shellp

run: 
	./shellp script.sh

all: build run