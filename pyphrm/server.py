import sys
from concurrent import futures
import grpc
from . import policy_pb2
from . import policy_pb2_grpc

class PolicyServer(policy_pb2_grpc.PolicyServicer):
    def __init__(self, fn_fdr_task_fd, fn_fdr_state_operator,
                       fn_fdr_state_operators_prob):
        self.fn_fdr_task_fd = fn_fdr_task_fd
        self.fn_fdr_state_operator = fn_fdr_state_operator
        self.fn_fdr_state_operators_prob = fn_fdr_state_operators_prob

    def GetFDRTaskFD(self, request, ctx):
        task = self.fn_fdr_task_fd()
        return policy_pb2.ResponseFDRTaskFD(task = task)

    def GetFDRStateOperator(self, request, ctx):
        op_id = self.fn_fdr_state_operator(request.state.val)
        return policy_pb2.ResponseFDRStateOperator(operator = op_id)

    def GetFDRStateOperatorsProb(self, request, ctx):
        op_probs = self.fn_fdr_state_operators_prob(request.state.val)
        res = policy_pb2.ResponseFDRStateOperatorsProb()
        for op, prob in op_probs:
            res.operator.add(operator = op, prob = prob)
        return res

def policyServer(url, fn_fdr_task_fd, fn_fdr_state_operator,
                 fn_fdr_state_operators_prob):
    """
    Start and run a pheromone policy server, given a URL and two callback functions
    :param url: the url the pheromone server should listen to
    :param fn_fdr_task_fd: a function taking no arguments and returning a planning task in FDR (.sas) format as a string
    :param fn_fdr_state_operator: a function taking an FD state as a list of ints and returning an action index (int)
    :param fn_fdr_state_operators_prob: a function taking an FD state as a
        list of ints and returning a tuple where each element is a pair of
        (operator-id, probability-assigned-by-the-policy)
    :return:
    """
    policy_server = PolicyServer(fn_fdr_task_fd, fn_fdr_state_operator,
                                 fn_fdr_state_operators_prob)
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    policy_pb2_grpc.add_PolicyServicer_to_server(policy_server, server)
    selected_port = server.add_insecure_port(url)
    server.start()
    print(f"Successfully started server using url {url}", file = sys.stdout)
    print(f"Server listening on port {selected_port}", file = sys.stdout)
    sys.stdout.flush();
    server.wait_for_termination()
