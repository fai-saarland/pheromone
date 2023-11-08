import ctypes
import os.path
import sys
import traceback

pheromone_dir = os.path.dirname(__file__)
pheromone_so_path = os.path.join(pheromone_dir, "libpheromone.so")

if not os.path.exists(pheromone_so_path):
    print("make sure libpheromone.so exists and is in the same folder as this script")

cdll = ctypes.CDLL(pheromone_so_path)

provide_task_t = ctypes.CFUNCTYPE(None)
req_op_t = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_char_p)
cdll.pythonPolicyServer.argtypes = (ctypes.POINTER(ctypes.c_char), provide_task_t, req_op_t)


def start_server(url, get_task_func, apply_policy_func):
    """
    Start and run a pheromone policy server, given a URL and two callback functions
    :param url: the url the pheromone server should listen to
    :param get_task_func: a function taking no arguments and returning a planning task in FDR (.sas) format as a string
    :param apply_policy_func: a function taking an FD state as a list of ints and returning an action index (int)
    :return:
    """
    def provide_task():
        fdr = get_task_func().encode()
        cdll.providePythonString.argtypes = (ctypes.c_char_p,)
        cdll.providePythonString.restype = None
        cdll.providePythonString(fdr)

    def apply(state_str):
        state_as_list = [int(x) for x in state_str.decode().split()]
        try:
            return apply_policy_func(state_as_list)
        except Exception:
            print(traceback.print_exc())
            sys.exit(1)

    cdll.pythonPolicyServer(url.encode(), provide_task_t(provide_task), req_op_t(apply))
