#include <math.h>

int iseed;

/**************************/
double randFloat(void) {
    /* Uniform random number generator x(n+1)= a*x(n) mod c
    with a = pow(7,5) and c = pow(2,31)-1.
	Copyright (c) Tao Pang 1997. */
    const int ia=16807,ic=2147483647,iq=127773,ir=2836;
    int il,ih,it;
    double rc;
    //extern int iseed;
    ih = iseed/iq;
    il = iseed%iq;
    it = ia*il-ir*ih;
    if (it > 0)	iseed = it;
    else iseed = ic+it;
    rc = ic;
    return(iseed/rc);
}

/**************************/
int getNextRand(int limit) {
	return (int)(100000*randFloat()) % limit;
}

/**************************/
void seed(unsigned int seed){
	iseed = seed;
}
