#include <stdio.h>
#include <stdlib.h>

int graph_file_generator(char * filename, unsigned long v, unsigned long e);

int main(int argc, char** argv){
	if (argc != 4) {
		printf("Usage: #ofvert #ofedges filename (argc = %d)\n", argc);
	}
	else {
		char * end;
		long v = strtol(argv[1], &end, 10);
		long e = strtol(argv[2], &end, 10);
		if (v == 0 || e == 0){
			printf("Usage: #ofvert #ofedges filename (v != 0, e != 0)\n");
		}
		else{
			int ret = graph_file_generator(argv[3], v, e);
			return ret;
		}
	}
}

int graph_file_generator(char * filename, unsigned long v, unsigned long e){
	FILE* g_file;
	unsigned long mat[v][v];
	unsigned long rem = e;

	int ret;
	if ((g_file = fopen(filename, "w")) == NULL){
		ret = -1;
	}
	else{
		fprintf(g_file, "p edge %ld %ld\n", v, e);
		for (unsigned long it = 1; it < v + 1; it++){
			unsigned long cur = (rand() % v) % rem;
			rem -= cur;
			printf("cur = %ld\trem = %ld\tv = %ld\n", cur, rem, it);
			for (unsigned long it2 = 0; it2 < cur; it2++){
				unsigned long v1 = it;
				unsigned long v2 = ((rand() * rand()) % v) + 1;
				while (mat[v1][v2] == 1){
					//printf("v1 = %ld\tv2 = %ld\n", v1, v2);
					//srand(v % e + 4);
					//v1 = ((rand() * rand()) % v) + 1;
					v2 = ((rand() * rand()) % v) + 1;
				}
				fprintf(g_file, "e %ld %ld\n", v1, v2);
				mat[v1][v2] = 1;
			}
			printf("here\n");
		}
		ret = 1;
	}
	fclose(g_file);
	return ret;
}