
CC=c++
CFLAGS=-O3 -std=c++14 -ffast-math -Wall -mtune=native

all: reduce genrangeops pipedcalls app app-asm

reduce:
	$(CC) src/main.cpp -o reduce.asm -D PROGRAM_REDUCE -S $(CFLAGS)

genrangeops:
	$(CC) src/main.cpp -o genops.asm -D PROGRAM_GENRANGE_OPS -S $(CFLAGS)

pipedcalls:
	$(CC) src/main.cpp -o pipedcalls.asm -D PROGRAM_PIPED_CALLS -S $(CFLAGS)

app:
	$(CC) src/main.cpp -std=c++14 -Wall -O0 -g -o functionalpipelines

app-asm:
	$(CC) src/main.cpp -std=c++14 -Wall -O3 -o functionalpipelines.asm -S

clean:
	rm -rf functionalpipelines functionalpipelines.asm reduce.asm genops.asm pipedcalls.asm

