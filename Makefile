CC = gcc
CFLAGS = -Wall -std=gnu11 -g
OUTPUT = minichatserver

$(OUTPUT): objects
	$(CC) -o $(OUTPUT) obj/*.o $(CFLAGS)

objects: src/main.c
	mkdir -p obj
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS)

clean:
	rm -rf obj
	rm -f $(OUTPUT)
