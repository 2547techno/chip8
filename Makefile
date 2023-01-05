all: build run

b: build

build: bin/out.exe

bin/out.exe: $(shell find src -type f -name *.cpp)
	gcc -g src/main.cpp $(shell find src -type f -name *.cpp ! -path "src/main.cpp") -Iinclude/ -Wall -lstdc++ -o bin/out.exe

run:
	./bin/out.exe

clean:
	rm -rf bin/
	mkdir bin/