CFLAGS = -O3 -Wall -fstrict-aliasing -std=c99 -DEBUG -g

test:main
	./main -i C500.9.clq -B 57


main:main.c ant.c graph.c random.c statistics.c
	$(CC) $(CFLAGS) main.c -o $@
