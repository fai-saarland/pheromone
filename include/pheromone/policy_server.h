/***
 * Copyright (c)2023 Daniel Fiser <danfis@danfis.cz>. All rights reserved.
 * This file is part of pheromone licensed under 3-clause BSD License (see file
 * LICENSE, or https://opensource.org/licenses/BSD-3-Clause)
 */

#ifndef __PHEROMONE_POLICY_SERVER_H__
#define __PHEROMONE_POLICY_SERVER_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Returns FDR task encoded in FastDownward's format as (not necessarily
 * zero-terminated) string, and it returns the size of the returned string
 * via the output argument {size}.
 */
typedef char *(*phrm_policy_req_fdr_task_fd)(size_t *size, void *userdata);

/**
 * Reads an FDR state and returns ID of the selected operator or -1 if policy
 * cannot find any operator.
 */
typedef int (*phrm_policy_req_fdr_state_operator)(const int *state, void *ud);

/**
 * Reads an FDR state and returns a set of applicable operators along with
 * a probability (confidence) they are assigned by the policy.
 * Output argument {op_size} is filled with the number of returned
 * operators, {op_ids} will point to an allocated array of operator IDs,
 * {op_probs} will point to an allocated array of probabilities (each
 * op_probs[i] is a probability of the operator op_ids[i]). It is
 * responsibility of the caller to use free(3) to free the memory allocated
 * for {op_ids} and {op_probs}.
 * Returns 0 on success, -1 otherwise.
 */
typedef int (*phrm_policy_req_fdr_state_operators_prob)(const int *state,
                                                        int *op_size,
                                                        int **op_ids,
                                                        float **op_probs,
                                                        void *userdata);


/**
 * Runs the server and blocks as long as the server is running.
 */
int phrmPolicyServer(const char *url,
                     phrm_policy_req_fdr_task_fd req_fdr_fd,
                     phrm_policy_req_fdr_state_operator req_fdr_state_op,
                     phrm_policy_req_fdr_state_operators_prob req_fdr_state_ops_prob,
                     void *userdata);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PHEROMONE_POLICY_SERVER_H__ */
