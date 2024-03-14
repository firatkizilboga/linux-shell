.PHONY: build run debug


build:
	sudo gcc -g -c src/tokenizer.c -o build/tokenizer.o -I./include -Wall
	sudo gcc -g -c src/interpreter.c -o build/interpreter.o -I./include -Wall
	sudo gcc -g src/main.c build/tokenizer.o build/interpreter.o -o build/shellp -I./include -Wall
	sudo chmod +x build/shellp

run: 
	./build/shellp script.sh

debug: 
	sudo gdb ./build/shellp

all: build run
