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

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Usage: %s url\n", argv[0]);
        return -1;
    }

    return phrmPolicyServer(argv[1], reqFDRTaskFD, reqFDROp, NULL);
}
