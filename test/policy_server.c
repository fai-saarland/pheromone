#include <stdio.h>
#include <pheromone/policy_server.h>

static char *reqFDRTaskFD(size_t *size, void *userdata)
{
    *size = 10;
    char *t = malloc(*size);
    for (int i = 0; i < *size; ++i){
        t[i] = 'a' + (i % ('z' - 'a'));
    }
    return t;
}

static int reqFDROp(const int *state, void *userdata)
{
    return 123;
}

static int reqFDROpsProbs(const int *state,
                          int *op_size,
                          int **op_ids,
                          float **op_probs,
                          void *userdata)
{
    *op_size = 2;
    *op_ids = malloc(sizeof(int) * *op_size);
    *op_probs = malloc(sizeof(float) * *op_size);
    (*op_ids)[0] = 12;
    (*op_ids)[1] = 1;
    (*op_probs)[0] = 0.8;
    (*op_probs)[1] = 0.2;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Usage: %s url\n", argv[0]);
        return -1;
    }

    return phrmPolicyServer(argv[1], reqFDRTaskFD, reqFDROp,
                            reqFDROpsProbs, NULL);
}
