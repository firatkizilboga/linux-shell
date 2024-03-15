.PHONY: build run debug

build:
	sudo rm -rf build
	sudo mkdir build
	sudo gcc -std=c2x -g -c src/tokenizer.c -o build/tokenizer.o -I./include
	sudo gcc -std=c2x -g -c src/interpreter.c -o build/interpreter.o -I./include
	sudo gcc -std=c2x -g src/main.c build/tokenizer.o build/interpreter.o -o build/shellp -I./include
	sudo chmod +x build/shellp

run: 
	./build/shellp script.sh

debug: 
	sudo gdb ./build/shellp

all: build run
