all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o
	gcc -o bin/sdstored obj/sdstored.o -g

obj/sdstored.o: src/sdstored.c
	gcc -o obj/sdstored.o src/sdstored.c -Wall -g -c

bin/sdstore: obj/sdstore.o
	gcc -o bin/sdstore obj/sdstore.o -g

obj/sdstore.o: src/sdstore.c
	gcc -o obj/sdstore.o src/sdstore.c -Wall -g -c

clean:
	rm -rf obj/*.o tmp/* bin/*