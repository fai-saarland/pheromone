#include <stdio.h>
#include <string.h>
#include <assert.h>
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
    if (task == NULL){
        fprintf(stderr, "Error: Did not receive the FDR task description.\n");
        phrmPolicyDel(p);
        return -1;
    }

    const char *cur = task;
    const char *tmp;

    // Extract the size of state
    int state_size = 0;
    while ((tmp = strstr(cur, "\nbegin_variable\n")) != NULL){
        cur = tmp + 16;
        ++state_size;
    }
    assert(state_size > 0);

    printf("State size: %d\n", state_size);
    // Extract the initial state
    int state[state_size];
    cur = strstr(task, "\nbegin_state\n");
    assert(cur != NULL);
    cur += 13;
    assert(cur[0] >= '0' && cur[0] <= '9');
    for (int i = 0; i < state_size; ++i){
        char *end;
        state[i] = strtol(cur, &end, 10);
        cur = end;
    }

    printf("Initial state:");
    for (int i = 0; i < state_size; ++i){
        printf(" %d", state[i]);
    }
    printf("\n");

    int op = phrmPolicyFDRStateOperator(p, state, state_size);
    printf("Operator: %d\n", op);

    for (int i = 0; (tmp = strstr(cur, "\nbegin_operator\n")) != NULL; ++i){
        cur = tmp + 16;
        if (i == op){
            printf("Operator name: (");
            for (; *cur != '\n' && *cur != '\x0'; ++cur)
                printf("%c", *cur);
            printf(")\n");
            break;
        }
    }

    //printf("%s\n", task);
    free(task);
    phrmPolicyDel(p);
    return 0;
}

