/** 
    Calculates 1d coordinates from 3d coordinates
    @param i position on x axis
    @param j position on y axis
    @param k position on z axis
    @param N size of x axis
    @param M size of y axis
    @param D size of z axis
*/
int worldIdx(int i, int j, int k, const int N, const int M, const int D){
    k = (k + D) % D;
    i = (i + N) % N;
    j = (j + M) % M;
	return k * N * M + i * M + j;
}

/** 
    Calculates a step on conway's game of life
    @param currentGlobal global array representing current state of world
    @param next global array representing next state of world
    @param N size of world's x axis
    @param M size of world's y axis
    @param D size of world's z axis
*/
__kernel void calcStep(global int *current, global int *next, int N, int M, int D){

    // global position
    int gindex = get_global_id(0);

    // global position in 3 dimensions  
    int k = gindex / (N * M);  
    int i = (gindex % (N * M)) / M;
    int j = (gindex % (N * M)) % M;

    //get number of neighbours
    int neighbours = current[worldIdx(i - 1, j - 1, k, N, M, D)] + current[worldIdx(i - 1, j, k, N, M, D)] + current[worldIdx(i - 1, j + 1, k, N, M, D)] + // same k
                    current[worldIdx(i, j - 1, k, N, M, D)] + current[worldIdx(i, j + 1, k, N, M, D)] +
                    current[worldIdx(i + 1, j - 1, k, N, M, D)] + current[worldIdx(i + 1, j, k, N, M, D)] + current[worldIdx(i + 1, j + 1, k, N, M, D)] +
                    current[worldIdx(i - 1, j - 1, k+1, N, M, D)] + current[worldIdx(i - 1, j, k+1, N, M, D)] + current[worldIdx(i - 1, j + 1, k+1, N, M, D)] + // next k
                    current[worldIdx(i, j - 1, k+1, N, M, D)] + current[worldIdx(i, j + 1, k+1, N, M, D)] + current[worldIdx(i, j, k+1, N, M, D)] +
                    current[worldIdx(i + 1, j - 1, k+1, N, M, D)] + current[worldIdx(i + 1, j, k+1, N, M, D)] + current[worldIdx(i + 1, j + 1, k+1, N, M, D)] +
                    current[worldIdx(i - 1, j - 1, k-1, N, M, D)] + current[worldIdx(i - 1, j, k-1, N, M, D)] + current[worldIdx(i - 1, j + 1, k-1, N, M, D)] + // last k
                    current[worldIdx(i, j - 1, k-1, N, M, D)] + current[worldIdx(i, j + 1, k-1, N, M, D)] + current[worldIdx(i, j, k-1, N, M, D)] +
                    current[worldIdx(i + 1, j - 1, k-1, N, M, D)] + current[worldIdx(i + 1, j, k-1, N, M, D)] + current[worldIdx(i + 1, j + 1, k-1, N, M, D)];

    //set next step 
    next[gindex] = current[gindex] && (5 <= neighbours && neighbours <= 6) || !current[gindex] && neighbours == 4;
}