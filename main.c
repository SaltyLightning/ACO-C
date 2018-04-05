#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "random.c"
#include "graph.c"
#include "statistics.c"
#include "ant.c"


/**********************/
int usage (char *exec) {
	printf("\nUsage :\n\n\t%s ",exec);
	printf("-a (alpha: int) -B (size of the maximum clique: int) -r (rho: float) ");
	printf("-c (max nb cycle: int) -n (nb ants: int) -m (tau min: float) ");
	printf("-M (tau max: float) -i (DIMACS graph filename) -v display-frequency ");
	printf("[-p (mustRepair set to 1)] -s (seed: positive int) -t (mode: v if AntClique(Vertex); c if AntClique(Clique))\n\n");
	return(0);
}

/**********************/
int parse(int* alpha, 
		  int* best, 
		  float* rho, 
		  int* maxCycles, 
		  int* nbAnts, 
		  float* tauMin, 
		  float* tauMax, 
		  int* verbose, 
		  int* displayFreq, 
		  char* fileName, 
		  int* mustRepair, 
		  int* iseed, 
		  char* mode, 
		  char* argv[], int argc){
	// set parameters wrt argv and argc
	char ch;
	extern char* optarg;
	while ( (ch = getopt(argc, argv, "a:B:r:p:c:n:m:M:v:i:?:h:s:t:"))!=-1 ) {
		switch(ch) {
			case 'a': *alpha=atoi(optarg); break;
			case 'B': *best=atoi(optarg); break;
			case 'r': *rho=atof(optarg); break;
			case 'p': *mustRepair=atoi(optarg); break;
			case 's': *iseed=atoi(optarg); break;
			case 'c': *maxCycles=atoi(optarg); break;
			case 'n': *nbAnts=atoi(optarg); break;
			case 'm': *tauMin=atof(optarg); break;
			case 'M': *tauMax=atof(optarg); break;
			case 'v': *verbose=1; *displayFreq=atoi(optarg); break;
			case 'i': strncpy(fileName, optarg, 254); break;
			case 't': *mode=optarg[0]; break;
			case '?':
			case 'h':
			default: usage(argv[0]); return(1);
		}
	}
	return(0);
}



int main (int argc, char *argv[]) {
	
	// Declare parameters and initialize them with default values
	char fileName[1024];	// name of the file which contains the graph
	int verbose = 1;		// when verbose=1, information is displayed during the solution process
	int displayFreq = 200;  // display frequency when verbose = 1
	int alpha = 1;          // pheromone factor weight
	int best = 10000;		// bound on the size of the maximum clique
	float rho = 0.01;		// persistence rate
	int maxCycles = 3000;	// maximum number of cycles
	int nbAnts = 30;		// number of ants
	float tauMin = 0.01;	// minimum bound on pheromone trails
	float tauMax = 6;		// maximum bound on pheromone trails
	int mustRepair = 1;		// when mustRepair=1, local search is performed on the best clique of each cycle
	int iseed = 3;			// seed of the random number generator
	char mode = 'v';		// 'v' for vertex strategy; 'c' for clique strategy
	
	clock_t tClock=clock();	// starting time
	float totalTime = 0;	// the total running time of the algorithm

	// get parameters
	if( parse(&alpha, 
			  &best, 
			  &rho, 
			  &maxCycles, 
			  &nbAnts, 
			  &tauMin, 
			  &tauMax, 
			  &verbose, 
			  &displayFreq, 
			  fileName, 
			  &mustRepair, 
			  &iseed, 
			  &mode, 
			  argv, argc) == 1) 
		return(1);
	if (verbose==1)
		printf("Params: alpha=%i best=%d rho=%f tauMin=%f tauMax=%f nbCycles=%d nbAnts=%d verbose=%d(%d) input=%s mustRepair=%d seed=%d mode=%c\n",alpha, best, rho, tauMin, tauMax, maxCycles, nbAnts, verbose, displayFreq, fileName, mustRepair, iseed, mode);
	
	// initialize data structures
	seed(iseed);
	graph G;
	createGraph(fileName,&G);
	if (verbose==1) 
		printf("Graph: %d vertices and %d edges\n", G.nbVertices, G.nbEdges);
	initPhero(mode,tauMax,&G);
	if (verbose==1) 
		printf("Pheromone trails initialized to %f\n",tauMax);
	if (best>G.nbVertices) 
		best=G.nbVertices;
	statistics S;
	createStatistics(G.nbVertices,&S);
	int bestCliqueSize = 0;
	int bestClique[G.nbVertices];
	int currentCliqueSize;
	int currentClique[G.nbVertices];
	int bestCliqueCycleSize;
	int bestCliqueCycle[G.nbVertices];
	int nbRepair = 0;
	float qty;
	int i, j;
	int nbIter = 0;
	int nbCycles;
	
	// start the solution process
	if (verbose==1) 
		printf("Starting solution process\n");
	for (nbCycles=0; ((nbCycles<maxCycles) && (bestCliqueSize<best)); nbCycles++){
		bestCliqueCycleSize = 0;
		// Each ant computes a clique
		for(i=0 ; ((i<nbAnts) && (bestCliqueSize<best)) ; i++) {
			currentCliqueSize = walk(alpha,currentClique,getNextRand(G.nbVertices),&G);
			if (verbose==1) 
				update(currentClique,currentCliqueSize,&S);
			nbIter += currentCliqueSize;
			if (currentCliqueSize > bestCliqueCycleSize) {
				for (j=0; j<currentCliqueSize; j++) 
					bestCliqueCycle[j] = currentClique[j];
				bestCliqueCycleSize = currentCliqueSize;
			}
		}
		if ((mustRepair==1) && (bestCliqueCycleSize<best)) 
			// repair the best clique of the cycle
			nbRepair += repair(bestCliqueCycle,&bestCliqueCycleSize,&G);
		// update best clique
		if (bestCliqueCycleSize > bestCliqueSize) {
			/* 
			 if(!checkClique(bestCliqueCycleSize,bestCliqueCycle,&G)){
				printf(" * WARNING * new clique is not a clique...\n");
				displayClique(bestCliqueCycleSize,bestCliqueCycle);
			}
			 */
			bestCliqueSize=bestCliqueCycleSize;
			for (j=0; j<bestCliqueSize; j++) 
				bestClique[j]=bestCliqueCycle[j];
			printf("Best clique size = %d at cycle %d (%f seconds)\n", bestCliqueSize, nbCycles, (float)(clock()-tClock)/CLOCKS_PER_SEC);
			if (bestCliqueSize >= best){
				displayClique(bestCliqueSize, bestClique);
				printf("Total Time spent: %f\n", totalTime);
				return 0;
			}
		}
		// Pheromone updating step
		evaporate(1-rho,tauMin,&G);
		qty = 1.0/(float)(bestCliqueSize + 1 - bestCliqueCycleSize);
		reinforce(bestCliqueCycle,bestCliqueCycleSize, qty, tauMax, &G);
		float currentTime = (float)((clock()-tClock)/CLOCKS_PER_SEC);
		totalTime += currentTime;
		// Display
		if((verbose==1) && ((nbCycles+1) % displayFreq == 0)) {
			printf("Statistics at cycle %d : ", nbCycles+1);
			printf("CPU time=%f ",currentTime);
			printf("Number of LS moves=%d ",nbRepair);
			printf("Average clique size=%f ",getAverageSize(&S));
			printf("Average similarity=%f ",getSimilarity(&S));
			printf("Number of resamplings=%d\n",getNbConflicts(&S));
			printf("default qty = %f\n", 1.0/(float)(bestCliqueSize + 1 - bestCliqueCycleSize));
			reset(&S);
		}
	}
	displayClique(bestCliqueSize, bestClique);
	printf("Total Time spent: %f\n", totalTime);

	return(0);
}



