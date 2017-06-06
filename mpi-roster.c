#define _GNU_SOURCE
#include <sched.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#define TRUE 1
#define FALSE 0

int update_position(char *buffer) {
    int count=0;

    while (*buffer != 0) {
        buffer++;
        count++;
    }
    return count;
}

void get_corelist(char *buffer, int size) {
    int i, j;
    int touched=FALSE;
    cpu_set_t mask;
    int offset=0;

    sched_getaffinity(0, sizeof(mask), &mask);

    for (i=0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &mask)) {
            if (touched) {
                sprintf(buffer, ",");
                offset += update_position(buffer);
            }
            int count=0;
            touched=TRUE;
            for (j=1; (i + j) < CPU_SETSIZE; j++) {
                if (! CPU_ISSET(i + j, &mask))
                    break;
                count++;
            }
            sprintf(buffer, "%d", i);
            offset += update_position(buffer);
            if (count == 1)
                sprintf(buffer, ",%d", i + 1);
            else if (count > 1)
                sprintf(buffer, "-%d", i + count);
            offset += update_position(buffer);
        }
        if (offset + CPU_SETSIZE > size)
            break;
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    char node[256];
    char core[8 * CPU_SETSIZE];

    memset(node, 0, sizeof(node));
    memset(core, 0, sizeof(core));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    gethostname(node, sizeof(node));
    get_corelist(core, sizeof(core));

    int pad = 1 + (int) (log(size) / log(10));
    printf("Rank %*d @ node %s, core: %s\n", pad, rank, node, core);

    MPI_Finalize();
    return(0);
}

