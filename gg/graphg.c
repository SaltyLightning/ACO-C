#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

int graph_file_generator(char * filename, int v, int e);

int main(int argc, char** argv){
	if (argc != 4) {
		printf("Usage: #ofvert #ofedges filename (argc = %d)\n", argc);
	}
	else {
		char * end;
		unsigned long long input1 = strtol(argv[1], &end, 10);
		unsigned long long input2 = strtol(argv[2], &end, 10);
		if (input2 > INT_MAX || input1 > INT_MAX){
			printf("Graphs are limited to int size\n");
		}
		else{
			int v = (int) input1;
			int e = (int) input2;
			if (v == 0 || e == 0){
				printf("Usage: #ofvert #ofedges filename (v != 0, e != 0)\n");
			}
			else{
				printf("Vertex Count: %d Edge Count: %d\n", v, e);
				int ret = graph_file_generator(argv[3], v, e);
				return ret;
			}
		}
	}
}

int graph_file_generator(char * filename, int v, int e){
	FILE* g_file;
	//int mat[v+1][v+1];
	int rem = e;

	int ret;

	// for (int outer = 1; outer < v + 1; outer++){
	// 		for (int inner = 1; inner < v + 1; inner++){
	// 			mat[outer][inner] = (int)0;
	// 		}
	// 	}
	if ((g_file = fopen(filename, "w")) == NULL){
		ret = -1;
	}
	else{
		fprintf(g_file, "p edge %d %d\n", v, e);
		if (-1 == -1)
		{
			for (int outer = 1, total = 0; outer < v/2; outer++){
				for (int inner = v; inner > (v/2)  + 1 
					&& total < e; inner--, total++){
					fprintf(g_file, "e %d %d\n", outer, inner);
				}
			}
		}
		else{
			// for (int it = 1; it < e + 1; it++){
			// 	// int cur = (rand() % v) % rem;
			// 	// rem -= cur;
			// 	// printf("cur = %d\trem = %d\tv = %d\n", cur, rem, it);
			// 	// for (int it2 = 0; it2 < cur; it2++){
			// 		int v1 = (abs((rand() * rand())) % v) + 1;
			// 		int v2 = (abs((rand() * rand())) % v) + 1;
			// 		while (mat[v1][v2] == (int)1 ||
			// 		mat[v2][v1] == (int)1 ){
			// 			//printf("v1 = %d\tv2 = %d\n", v1, v2);
			// 		// 	//srand(v % e + 4);
			// 			srand(time(0));
			// 			v1 = (abs((rand() * rand())) % v) + 1;
			// 			srand(time(0) + time(0));
			// 			v2 = (abs((rand() * rand())) % v) + 1;
			// 		}
			// 		mat[v1][v2] = (int)1;
			// 	// }
			// 		// printf("e = %d\n", it);
			// 	//printf("Wrote to %d %d\n", v1, v2);
			// }
			// for (int outer = 1; outer < v + 1; outer++){
			// 	for (int inner = 1; inner < v + 1; inner++){
			// 		if (mat[outer][inner] == 1)
			// 			fprintf(g_file, "e %d %d\n", outer, inner);
			// 	}
			// 	//printf("%d\n", outer);
			// }
		}

		ret = 1;
	}
	fclose(g_file);
	return ret;
}