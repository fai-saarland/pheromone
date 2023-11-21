import sys
from concurrent import futures
import grpc
from . import policy_pb2
from . import policy_pb2_grpc

class PolicyServer(policy_pb2_grpc.PolicyServicer):
    def __init__(self, fn_fdr_task_fd, fn_fdr_state_operator):
        self.fn_fdr_task_fd = fn_fdr_task_fd
        self.fn_fdr_state_operator = fn_fdr_state_operator

    def GetFDRTaskFD(self, request, ctx):
        task = self.fn_fdr_task_fd()
        return policy_pb2.ResponseFDRTaskFD(task = task)

    def GetFDRStateOperator(self, request, ctx):
        op_id = self.fn_fdr_state_operator(request.state.val)
        return policy_pb2.ResponseFDRStateOperator(operator = op_id)

def policyServer(url, fn_fdr_task_fd, fn_fdr_state_operator):
    """
    Start and run a pheromone policy server, given a URL and two callback functions
    :param url: the url the pheromone server should listen to
    :param fn_fdr_task_fd: a function taking no arguments and returning a planning task in FDR (.sas) format as a string
    :param fn_fdr_state_operator: a function taking an FD state as a list of ints and returning an action index (int)
    :return:
    """
    policy_server = PolicyServer(fn_fdr_task_fd, fn_fdr_state_operator)
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    policy_pb2_grpc.add_PolicyServicer_to_server(policy_server, server)
    server.add_insecure_port(url)
    server.start()
    server.wait_for_termination()


if __name__ == '__main__':
    def fdr_task_fd():
        return 'This is FDR task'
    def fdr_state_op(state):
        return sum(state)
    policyServer(sys.argv[1], fdr_task_fd, fdr_state_op)
