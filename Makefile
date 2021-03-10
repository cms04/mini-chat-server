CC = gcc
CFLAGS = -Wall -std=gnu11 -g
OUTPUT = minichatserver

$(OUTPUT): objects
	$(CC) -o $(OUTPUT) obj/*.o $(CFLAGS)

objects: src/main.c src/server.c src/server.h src/client.c src/client.h src/macros.h
	mkdir -p obj
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS)
	$(CC) -c src/client.c -o obj/client.o $(CFLAGS)
	$(CC) -c src/server.c -o obj/server.o $(CFLAGS)

clean:
	rm -rf obj
	rm -f $(OUTPUT)
