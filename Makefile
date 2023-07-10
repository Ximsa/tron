tron: tron.c
	gcc -Wall -O0 -o tron tron.c -lSDL2

clean:
	rm tron

run: tron
	./tron

lib: tron.c
	gcc  -Wall -O2 -shared -fPIC tron.c -o ./libtron.so -lSDL2

release: tron.c
	gcc -Wall -O3 -ffast-math -o ./tron tron.c -lSDL2
