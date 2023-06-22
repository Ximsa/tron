tron: tron.c
	gcc -Wall -O0 -o tron tron.c -lSDL2

clean:
	rm tron

run: tron
	./tron

lib: tron.c
	gcc -Wall -O2 -shared -fPIC -lSDL2 tron.c -o ./libtron.so

release: comic.c
	gcc -Wall -O2 -ffast-math -o ./tron tron.c -lSDL2
