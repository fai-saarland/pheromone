#!/usr/bin/env python3

import os.path
import sys

# pyphrm needs to be in sys.path
script_dir = os.path.dirname(os.path.abspath(__file__))
pheromone_dir = os.path.dirname(script_dir)
pheromone_path = os.path.join(pheromone_dir, 'pyphrm', 'policy_pb2_grpc.py')
if os.path.exists(pheromone_path):
    sys.path.append(pheromone_dir)
    print(f'Found pyphrm library')
else:
    print('pheromone not found. Have you build it (e.g. make in the top level directory)?')
    exit(1)


def get_task():
    """
    function returning the task in FD input format as a string
    FD input format is here: https://www.fast-downward.org/latest/documentation/translator-output-format/
    """
    print("Providing sas task")
    sas_file = os.path.join(script_dir, "example.sas")
    with open(sas_file, "r") as f:
        return f.read()


def apply_policy(state):
    """
    function taking an FD state as a list of ints and returning an action index (int)
    the action index must be consistent with the previously returned sas task
    """
    print(f"Providing action for state {state}") # you don't want such a printout for a real server
    return 0


def fdr_state_operators_prob(state):
    """
    a function taking an FD state as a list of ints 
    and returning a list of pair (operator-id, probability-assigned-by-the-policy)
    """
    print(f"Providing action prob distribution for state {state}") # you don't want such a printout for a real server
    return [(0,1)]


if __name__ == "__main__":
    from pyphrm import policyServer
    url = "127.0.0.1:0" # localhost with free port
    policyServer(url, get_task, apply_policy, fdr_state_operators_prob)

