
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float myPow(float x, int y){
	// precondition: y >= 0
	// returns x^y in O(log_2(y))
	float p;
	if (y==0) return 1;
	else if (y==1) return x;
	else if (y==2) return x*x;
	else {
		if (y%2==0){p = myPow(x,y/2); return p*p;}
		else{p = myPow(x,(y-1)/2); return p*p*x;};
	};
}

int choose(float *p, int nbCand){
	/* precondition: nbCand > 0 and for i in 0..nbCand-1, p[i] = sum_{j<i} tau[j]^alpha */
	/* returns k with probability (p[k]-p[k-1])/p[nbCand-1] */
	float f = randFloat();
	int left=0;
	int right=nbCand-1;
	int k;
	float total=p[nbCand-1];
	while (left<right){
		k=(left+right+1)/2;
		if (f<p[k-1]/total) right=k-1;
		else if (f>p[k]/total) left=k+1;
		else return k;
	}
	if (left>=0 && left<nbCand) return left;
	else printf("pb choose\n");
	return k;
}

int walkV(int alpha, int* clique, int firstVertex, graph* G) {
	// input: a graph G, an initial vertex firstVertex, and a parameter alpha
	// output: clique built in a greedy randomized way wrt strategy vertex, starting from initial vertex
	
	int i, v;
	int candidates[G->nbVertices];
	int nbCandidates;
	float total;
	float p[G->nbVertices];
	int cliqueSize = 1;
	
	// initializing the clique with firstVertex
	clique[0]=firstVertex;
	// initializing candidates lists and computing p for all candidates
	nbCandidates = G->degree[firstVertex];
	total = 0;
	for (i=0; i<nbCandidates; i++) {
		candidates[i] = G->succ[firstVertex][i];
		p[i] = myPow(G->tauV[candidates[i]], alpha) + total;
		total = p[i];
	}
	while (nbCandidates>0){
		// choice of the next vertex v within candidates, w.r.t. proba p
		i = choose(p,nbCandidates);
		v = candidates[i];
		// adding v to the clique
		clique[cliqueSize] = v;
		cliqueSize++;
		// removing v from the list of candidates
		nbCandidates--;
		candidates[i] = candidates[nbCandidates];
		// filtering the list of candidates and computing p
		i=0;
		total=0;
		while (i<nbCandidates){
			if ( isEdge(v, candidates[i], G) ) {
				// Candidates[i] is still a candidate -> compute p
				p[i] = myPow(G->tauV[candidates[i]], alpha)+total;
				total = p[i];
				i++;
			}
			else {
				// Candidates[i] is no longer a candidate
				nbCandidates--;
				candidates[i]=candidates[nbCandidates];
			}
		}
	}
	return cliqueSize;
}

int walkE(int alpha, int* clique, int firstVertex, graph* G) {
	// input: a graph G, an initial vertex firstVertex, and a parameter alpha
	// output: clique built in a greedy randomized way wrt strategy clique, starting from initial vertex
	// returns the size of clique
	
	int i, v;
	int candidates[G->degree[firstVertex]];
	int nbCandidates;
	float total;
	float p[G->degree[firstVertex]];
	float tauClique[G->nbVertices];
	int cliqueSize = 1;
	
	// initializing the clique with firstVertex
	clique[0]=firstVertex;
	// initializing candidates lists and computing p for all candidates
	nbCandidates = G->degree[firstVertex];
	total = 0;
	for (i=0; i<nbCandidates; i++) {
		candidates[i] = G->succ[firstVertex][i];
		tauClique[candidates[i]] = (G->tauE)[firstVertex][candidates[i]];
		p[i] = myPow(tauClique[candidates[i]], alpha) + total;
		total = p[i];
	}
	while (nbCandidates>0){
		// choice of the next vertex v within candidates, w.r.t. proba p
		i = choose(p,nbCandidates);
		v = candidates[i];
		// adding v to the clique
		clique[cliqueSize] = v;
		cliqueSize++;
		// removing v from the list of candidates
		nbCandidates--;
		candidates[i] = candidates[nbCandidates];
		// filtering the list of candidates and computing p
		i=0;
		total=0;
		while (i<nbCandidates){
			if ( isEdge(v, candidates[i], G) ) {
				// Candidates[i] is still a candidate -> compute p
				tauClique[candidates[i]] += (G->tauE)[v][candidates[i]];
				p[i] = myPow(tauClique[candidates[i]], alpha)+total;
				total = p[i];
				i++;
			}
			else {
				// Candidates[i] is no longer a candidate
				nbCandidates--;
				candidates[i]=candidates[nbCandidates];
			}
		}
	}
	return cliqueSize;
}


int walk(int alpha, int* clique, int firstVertex, graph* G) {
	if (G->mode=='v') return walkV(alpha,clique,firstVertex,G);
	else return walkE(alpha,clique,firstVertex,G);
}


int repair(int* clique, int* cliqueSize, graph* G) {
	// improves clique by greedy local search
	
	int vi, vj, vk, i, j, k, l, stop, found;
	int delta = 0;
	
	i = getNextRand(*cliqueSize);
	stop = i;
	while (1){
		vi = clique[i];
		// looking for (vj,vk) in nonSucc(vi) so that (vj,vk) in E 
		// and vj and vk are connected to all the vertices of the clique
		found = 0;
		for (j=0; ((j<G->nbVertices-G->degree[vi]-1) && (found==0)); j++){
			vj = G->nonSucc[vi][j];
			for (k=j+1; ((k<G->nbVertices-G->degree[vi]-1) && (found==0)); k++){
				vk = G->nonSucc[vi][k];
				if (isEdge(vj,vk,G)){
					for (l=0; ((l<*cliqueSize) && 
							   ((clique[l]==vi) || 
								((isEdge(vj,clique[l],G)) && (isEdge(vk,clique[l],G))))); 
						 l++);
					if (l==*cliqueSize) found = 1;
					else if (!isEdge(vj,clique[l],G)) k = G->nbVertices;
				}
			}
		}
		if (found==1){
			// remove vi from the clique, and add vj and vk
			clique[i] = vj;
			clique[*cliqueSize] = vk;
			(*cliqueSize)++;
			delta++;
			stop = i;
		}
		else{
			i++;
			if (i==*cliqueSize) i=0;
			if (i==stop) return delta;
		}
	}
}



  
