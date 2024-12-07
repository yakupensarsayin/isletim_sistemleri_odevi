all: bin/shell.out

bin/shell.out: lib/main.o lib/shell.o lib/signal_handler.o
	@mkdir -p bin
	gcc -Iinclude -o bin/shell.out lib/main.o lib/shell.o lib/signal_handler.o

lib/main.o: src/main.c
	@mkdir -p lib
	gcc -Iinclude -c src/main.c -o lib/main.o

lib/shell.o: src/shell.c
	@mkdir -p lib
	gcc -Iinclude -c src/shell.c -o lib/shell.o

lib/signal_handler.o: src/signal_handler.c
	@mkdir -p lib
	gcc -Iinclude -c src/signal_handler.c -o lib/signal_handler.o

clean:
	rm -f lib/*.o bin/shell.out

.PHONY: all clean
