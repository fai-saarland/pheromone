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

typedef char *(*phrm_policy_req_fdr_task_fd)(size_t *size, void *userdata);
typedef int (*phrm_policy_req_fdr_state_operator)(const int *state, void *ud);
typedef int (*python_phrm_policy_req_fdr_state_operator)(const char *state);
typedef void (*void_callback)();


/**
 * Runs the server and blocks as long as the server is running.
 */
int phrmPolicyServer(const char *url,
                     phrm_policy_req_fdr_task_fd req_fdr_fd,
                     phrm_policy_req_fdr_state_operator req_fdr_state_op,
                     void *userdata);


/**
 * To be called by python wrapper to copy a python string into a heap allocated string managed by pheromone.
 */
void providePythonString(const char *s);

/**
 * Alternative to phrmPolicyServer. To be used in python wrapper.
 */
int pythonPolicyServer(const char *url,
                       void_callback provide_fdr,
                       python_phrm_policy_req_fdr_state_operator req_fdr_state_op);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PHEROMONE_POLICY_SERVER_H__ */
