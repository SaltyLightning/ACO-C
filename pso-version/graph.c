#include <string.h>
#include <stdio.h>

typedef struct{
	int mode;		// 'c' if strategy is clique; 'v' otherwise
	float* tauV;	// pheromone matrix used when mode='v'
	float** tauE;	// pheromone matrix used when mode='c'
	float tauM;		// the maximum pheromone currently found in the graph
	int nbVertices;	// number of vertices in the graph
	int nbEdges;	// number of edges in the graph
	int* degree;	// forall i in 0..nbVertices-1, degree[i] = degree of ith vertex
	int** succ;		// forall i in 0..nbVertices-1, forall j in 0..degree[i]-1, succ[i][j] = jth successor of ith vertex
	int** nonSucc;	// forall i in 0..nbVertices-1, forall j in 0..nbVertices-degree[i]-1, nonsucc[i][j] = jth non successor of ith vertex
} graph;


/**********************/
void createGraph(char* file_name, graph* G) {
	char buffer[1024];
	FILE* fd;
	char type;
	char str[1024];
	int cp, src, dest;
	int edgesCounter=0;
	int i, j, nbSucc, nbNonSucc;
	
	// Opening file
	if ( (fd=fopen(file_name, "r"))==NULL){	
		printf("ERROR: Cannot open ascii input file %s", file_name); 
		exit(1);	
	}
	
	// Reading file
	while( (fgets(buffer, 1024, fd))!=NULL) {
		sscanf(buffer, "%c", &type);
		switch (type){
			case 'c': break; // Skip comments
			case 'p': // Problem description
				cp=sscanf(buffer, "%c%s%d%d", &type, str, &(G->nbVertices), &(G->nbEdges));
				if ( (strstr(str, "edge") != NULL) && (cp == 4) ) {
					G->degree = (int*)calloc(G->nbVertices,sizeof(int));
					G->tauE =(float**)calloc(G->nbVertices,sizeof(float*)); 
					G->succ = (int**)calloc(G->nbVertices,sizeof(int*));
					G->nonSucc = (int**)calloc(G->nbVertices,sizeof(int*));
					for (i=0; i<G->nbVertices; i++){
						G->degree[i] = 0;
						G->tauE[i] = (float*)calloc(G->nbVertices,sizeof(float));
						for (j=0; j<G->nbVertices; j++) G->tauE[i][j] = 0.0;
					}
				} 
				else {
					printf("-- Error while reading problem description --\n");
					exit(1);
				}
				break;
			case 'n': break; // Vertex description
			case 'e': // Edge description
				cp=sscanf(buffer, "%c%d%d", &type, &src, &dest);
				src--; 
				dest--;
				if ((cp==3) && (src>=0) && (src<G->nbVertices) && (dest>=0) && (dest<G->nbVertices)){
					edgesCounter++;
					G->tauE[src][dest]=1.0;
					G->tauE[dest][src]=1.0;
					G->degree[src]++;
					G->degree[dest]++;
				} 
				else {
					printf("-- Error while reading edge description #%d: (%d,%d)\n",edgesCounter,src,dest);
					exit(1);
				}
				break;
			default: break;
		}
	}
	fclose(fd);
	if (edgesCounter != G->nbEdges){
		printf("-- Abnormal number of edges --\n");
		exit(1);
	}
	for (src=0; src<G->nbVertices; src++){
		G->succ[src] = (int*)calloc(G->degree[src],sizeof(int));
		G->nonSucc[src] = (int*)calloc(G->nbVertices-G->degree[src]-1,sizeof(int));
		nbSucc = 0;
		nbNonSucc = 0;
		dest = 0;
		for (dest=0; dest<G->nbVertices; dest++){
			if (G->tauE[src][dest]>0){ G->succ[src][nbSucc] = dest; nbSucc++; }
			else if (src != dest){ G->nonSucc[src][nbNonSucc] = dest; nbNonSucc++; }
		}
	}
}

int isEdge(int i, int j, graph *G){
	// returns true if (i,j) is an edge of G
	return (G->tauE[i][j] > 0);
};



void initPhero(int m, float tauMax, graph* G){
	// Initialize pheromone trails to tauMax and initialize mode to m
	int i, j;
	G->mode = m;
	G->tauM = 0;
	if (m=='v'){
		G->tauV = (float*)calloc(G->nbVertices,sizeof(float));
		for (i=0; i<G->nbVertices; i++) G->tauV[i] = tauMax;
	}
	else{
		for (i=0; i<G->nbVertices; i++)
			for (j=0; j<G->degree[i]; j++) G->tauE[i][G->succ[i][j]] = tauMax;
	}
}

void evaporate(float rho, float tauMin, graph *G){
	// Evaporate pheromone trails wrt to persistence rate rho
	// ensure that no pheromone trail is lower than tauMin
	int i, j;
	if (G->mode=='v')
		for (i=0; i<G->nbVertices; i++){
			G->tauV[i] *= rho;
			if (G->tauV[i]<tauMin) 
				G->tauV[i]=tauMin;
		}
	else
		for (i=0; i<G->nbVertices; i++)
			for (j=0; j<G->degree[i]; j++){
				G->tauE[i][G->succ[i][j]] *= rho;
				if (G->tauE[i][G->succ[i][j]]<tauMin) 
					G->tauE[i][G->succ[i][j]] = tauMin;
			}
}


void reinforce(int* clique, int cliqueSize, float qty, float tauMax, graph *G){
	// increase pheromone components associated with clique[0..cliqueSize-1] by qty
	// ensure that no pheromone trail is greater than tauMax
	int i,j;
	if (G->mode=='v')
		for (i=0; i<cliqueSize; i++){
			G->tauV[clique[i]] += qty;
			if (G->tauV[clique[i]]>tauMax) G->tauV[clique[i]]=tauMax;
		}
	else
		for (i=0; i<cliqueSize; i++)
			for (j=i+1; j<cliqueSize; j++){
				G->tauE[clique[i]][clique[j]] += qty;
				if (G->tauE[clique[i]][clique[j]]>tauMax) G->tauE[clique[i]][clique[j]]=tauMax;
				G->tauE[clique[j]][clique[i]] = G->tauE[clique[i]][clique[j]];
			}
};


int compFunc(const void *x, const void *y) {
	// comparison function used to sort vertices in a clique
	int pp,qq;
	int t;
	pp = (int)(*(int *)x);
	qq = (int)(*(int *)y);
	if (pp < qq) t = -1;
	else if (pp == qq) t = 0;
	else  t = 1;
	return t;
}

int displayClique(int cliqueSize, int* clique){
	// displays vertices of clique[0..cliqueSize-1] by increasing order
	int i;
	qsort(clique, cliqueSize, sizeof(int), compFunc);
	printf("--------------------\nClique list dump:\n");
	printf("\t- clique size: %d vertices\n\t- clique list: ",cliqueSize);
	for(i=0 ; i<cliqueSize ; i++) printf("%d ", clique[i]+1);
	printf("\n--------------------\n");
	return(1);
}

int checkClique (int cliqueSize, int* clique, graph* G) {
	// check that clique[0..cliqueSize-1] actually is a clique
	int i, j;
	for (i=0 ; i<cliqueSize ; i++) {
		for (j=i+1 ; j<cliqueSize ; j++) {
			if (!isEdge(clique[i],clique[j],G)) return(0);
		}
	}
	return(1);
}

