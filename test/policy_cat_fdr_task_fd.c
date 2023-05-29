#include <stdio.h>
#include <pheromone/policy_client.h>

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Usage: %s url\n", argv[0]);
        return -1;
    }

    phrm_policy_t *p = phrmPolicyConnect(argv[1]);
    if (p == NULL)
        return -1;

    char *task = phrmPolicyFDRTaskFD(p);
    if (task != NULL){
        fprintf(stdout, "Got:\n%s\n", task);
        free(task);
    }
    phrmPolicyDel(p);
    return 0;
}

