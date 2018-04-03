
#include <stdlib.h>
#include <stdio.h>

// structure used to compute statistics during a run
typedef struct{
	int nbVertices;		// number of vertices in the graph
	int* freq;			// forall i in 0..nbVertices-1, freq[i] = number of times 
						// vertex i has been selected since the last reset
	int intersections;	// = sum_{i in 0..nbVertices-1} freq[i]*(freq[i]-1)/2
	int totVertices;	// sum of the sizes of all cliques built since the last reset
	int nbWalks;		// number of cliques built since the last reset
	int nbConflicts;	// number of conflicts in the hashing table 
						// -> gives a lower bound on the number of times a same clique has been recomputed
	int prime;          // size of the first dimension of hashing table
	int sizeOfMem;		// size of the second dimension of the hashing tables
	int **hashingTable;	// forall i in 0..prime-1, forall j in 0..sizeOfMem-1
						// if hashingTable[i][j] != -1, then a walk has computed a clique c such that
						// the first hash key of c if i and the second hash key of c is hashingTable[i][j]
	int *pow3;			// forall i in 0..nbVertices-1, pow3[i] = 3^i (no care of capacity overflow)
	int *perm;			// perm[0..nbVertices-1] is a permutation of 0..nbVertices used to compute the second hash key
} statistics;

double getAverageDispersion(int n, int k){
	// returns (1/k) * sum_{i=1}^k{i*C(k,i)*C(n-k,k-i)/C(n,k)}
	// where C(n,k) is the nb of different sets of k elements that can
	// be built from a set of n elements, i.e., C(n,k) = n!/((n-k)!k!)
	int i, j;
	double p_i, prod;
	if (2*k-n < 1) i=1; else i=2*k-n;
	p_i = (double)(k*k)/(double)(i);
	for (j=1; j<i; j++) p_i = p_i*(double)(k-j)*(double)(k-j)/(double)(j);
	for (j=k-i; j<=k-1; j++) p_i = p_i/(double)(n-j);
	for (j=0; j<k-i; j++) p_i = p_i*(double)(n-k-j)/(double)(n-j);
	prod = (double)(i)*p_i;
	//  printf("i=%d, p_i=%f et prod=%f\n", i,p_i,prod);
	for (i++; i<=k; i++){
		p_i = p_i*(double)((k-i+1)*(k-i+1))/(double)(i*(n-2*k+i));
		// p_i = C(k,i)*C(n-k,k-i)/C(n,k)
		//     = probability that |S1 intersection S2| = i 
		//       where S1 and S2 are 2 subsets of {1,2,...,n} such that |S1|=|S2|
		//     = (k^2/i) * pi_{j=1}^{i-1}{(k-j)^2/j} 
		//               * pi_{j=0}{k-i-1}{(n_k-j)/(n-j)}
		//               * pi_{j=k-i}^{k-1}{1/(n-j)}
		prod += (double)(i)*p_i;
		// printf("i=%d, p_i=%f et prod=%f\n", i,p_i,prod);
	}
	prod = prod/(double)(k);
	//  printf("prod = %f\n",prod);
	return prod;
}

void reset(statistics *S){
	// reset data structures used to compute the dispersion rate and the average quality
	int i;
	for (i=0; i<S->nbVertices; i++) S->freq[i]=0;
	S->intersections = 0;
	S->totVertices = 0;
	S->nbWalks = 0;
}

float getSimilarity(statistics *S){
	// return the average similarity of all cliques computed since the last reset
	if (S->nbWalks>1) 
		return (float)(2*S->intersections)/(float)((S->nbWalks-1)*S->totVertices);
	else return 0;
}

float getAverageSize(statistics *S){
	// returns the average size of all cliques since the last reset
	if (S->nbWalks>0) 
		return (float)(S->totVertices)/(float)(S->nbWalks);
	else return 0;
}

int getNbConflicts(statistics *S){
	// returns the number of conflicts that have occur in the hashing table
	// this gives an upper bound on the number of times a same clique has been recomputed
	return S->nbConflicts;
}

void createStatistics(int nbV, statistics *S){
	// initialize data structure used to compute statistics
	int i,j,aux;
	
	S->nbConflicts = 0;
	S->nbVertices = nbV;
	S->freq = (int*)calloc(nbV, sizeof(int));
	reset(S);
	
	S->prime = 452279;
	S->sizeOfMem = 10;
	// initialization of the hashing hashing table: 
	// forall i in 0..prime-1, foralll j in 0..sizeOfMem-1, hashingTable[i][j]=-1
	S->hashingTable = (int**)calloc( S->prime, sizeof(int*) );
	for (i=0; i<S->prime; i++){
		S->hashingTable[i] = (int*)calloc( S->sizeOfMem, sizeof(int) );
		for (j=0; j<S->sizeOfMem; j++) 
			S->hashingTable[i][j]=-1;
	}
	// initialization of pow3: forall i in 0..nbVertices-1, pow3[i]=(2^i) % prime
	S->pow3 = (int*)calloc( (S->nbVertices), sizeof(int) );
	S->pow3[0]= 1;
	for (i=1; i<S->nbVertices; i++) 
		S->pow3[i] = S->pow3[i-1]*3;
	// initialization of perm[0..nbVertices-1] to a permutation of [0..nbVertices-1]
	S->perm = (int*)calloc( S->nbVertices, sizeof(int) );
	for (i=0; i<S->nbVertices; i++) S->perm[i]=i;
	for (i=0; i<S->nbVertices; i++){
		j = i+getNextRand(S->nbVertices-i);
		aux = S->perm[i];
		S->perm[i] = S->perm[j];
		S->perm[j] = aux;
	}
}


void update(int* clique, int cliqueSize, statistics *S){
	// update statistics wrt clique[0..cliqueSize-1]
	int i;
	int h1 = 0;
	int h2 = 0;
	
	S->nbWalks++;
	for (i=0; i<cliqueSize; i++){
		S->intersections += S->freq[clique[i]];
		S->freq[clique[i]]++;
		h1 = h1+S->pow3[clique[i]];
		h2 = h2+S->pow3[S->perm[clique[i]]];
	}
	if (h1 < 0) h1 = -h1;
	h1 = h1 % S->prime;
	S->totVertices += cliqueSize;
	for (i=0; ((i<S->sizeOfMem) && (S->hashingTable[h1][i]!=h2) && (S->hashingTable[h1][i]>=0)); i++);
	if (i==S->sizeOfMem)
		printf("Memory exceeded... should realloc !\n");
	else if (S->hashingTable[h1][i]==h2) 
		S->nbConflicts++; // conflict detected
	else // hashingTable[h1][i]<0) -> add (h1,h2) to the hashing hashingTablele
		S->hashingTable[h1][i]=h2;
}
