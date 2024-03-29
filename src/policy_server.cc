/***
 * Copyright (c)2023 Daniel Fiser <danfis@danfis.cz>. All rights reserved.
 * This file is part of pheromone licensed under 3-clause BSD License (see file
 * LICENSE, or https://opensource.org/licenses/BSD-3-Clause)
 */

#include "pheromone/policy_server.h"
#include "rpc/policy.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <string>

class PolicyImpl final : public phrm::policy::Policy::Service {
  private:
    const phrm_policy_req_fdr_task_fd fdr_task_fd;
    const phrm_policy_req_fdr_state_operator fdr_op;
    const phrm_policy_req_fdr_state_operators_prob fdr_ops_prob;
    void *userdata;

  public:
    explicit PolicyImpl(phrm_policy_req_fdr_task_fd fdr_task_fd,
                        phrm_policy_req_fdr_state_operator fdr_op,
                        phrm_policy_req_fdr_state_operators_prob fdr_ops_prob,
                        void *userdata)
        : fdr_task_fd(fdr_task_fd),
          fdr_op(fdr_op),
          fdr_ops_prob(fdr_ops_prob),
          userdata(userdata)
    {
    }

    grpc::Status GetFDRTaskFD(grpc::ServerContext *ctx,
                              const phrm::policy::RequestFDRTaskFD *req,
                              phrm::policy::ResponseFDRTaskFD *res) override
    {
#ifndef NDEBUG
        std::cout << "Request: GetFDRTaskFD " << std::endl;
#endif

        if (fdr_task_fd == NULL)
            return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Unimplemented");

        size_t size;
        char *task = fdr_task_fd(&size, userdata);
        if (task == NULL)
            return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "No FDR task.");

        std::string task_str(task, size);
        res->set_task(task_str);
        free(task);

        return grpc::Status::OK;
    }

    grpc::Status GetFDRStateOperator(grpc::ServerContext *ctx,
                                     const phrm::policy::RequestFDRStateOperator *req,
                                     phrm::policy::ResponseFDRStateOperator *res) override
    {
#ifndef NDEBUG
        std::cout << "Request: GetFDRStateOperator " << std::endl;
#endif

        const phrm::fdr::State &bstate = req->state();
        int state_size = bstate.val_size();
        if (state_size <= 0){
            return grpc::Status(grpc::StatusCode::INTERNAL,
                                "Invalid request: empty state");
        }

        int *state = (int *)alloca(sizeof(int) * state_size);
        for (int i = 0; i < state_size; ++i)
            state[i] = bstate.val(i);

        int op_id = fdr_op(state, userdata);

        res->set_operator_(op_id);
        return grpc::Status::OK;
    }

    grpc::Status GetFDRStateOperatorsProb(grpc::ServerContext *ctx,
                                          const phrm::policy::RequestFDRStateOperatorsProb *req,
                                          phrm::policy::ResponseFDRStateOperatorsProb *res) override
    {
#ifndef NDEBUG
        std::cout << "Request: GetFDRStateOperatorsProb " << std::endl;
#endif
        const phrm::fdr::State &bstate = req->state();
        int state_size = bstate.val_size();
        if (state_size <= 0){
            return grpc::Status(grpc::StatusCode::INTERNAL,
                                "Invalid request: empty state");
        }

        int *state = (int *)alloca(sizeof(int) * state_size);
        for (int i = 0; i < state_size; ++i)
            state[i] = bstate.val(i);

        int op_size = 0;
        int *op_ids = NULL;
        float *op_probs = NULL;

        int st = fdr_ops_prob(state, &op_size, &op_ids, &op_probs, userdata);
        if (st == 0){
            for (int opi = 0; opi < op_size; ++opi){
                phrm::policy::OperatorProb *op_prob = res->add_operator_();
                op_prob->set_operator_(op_ids[opi]);
                op_prob->set_prob(op_probs[opi]);
            }

            if (op_ids != NULL)
                free(op_ids);
            if (op_probs != NULL)
                free(op_probs);

            return grpc::Status::OK;
        }

        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Error occurred when obtaining operator probabilities");
    }
};

int runPolicyService(PolicyImpl &service, const char *url)
{
    grpc::ServerBuilder builder;
    // selected_port will be populated in builder.BuildAndStart() if this is successful
    // can be different from port specified in url, i.e., url could be localhost:0 and port could be 12345
    int selected_port = -1;
    builder.AddListeningPort(url, grpc::InsecureServerCredentials(), &selected_port);
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server){
        std::cerr << "Error: Failed to create a server." << std::endl;
        return 1;
    }

    std::cout << "Successfully started server using url " << url << std::endl;
    std::cout << "Server listening on port " << selected_port << std::endl;
    server->Wait();

    return 0;
}

int phrmPolicyServer(const char *url,
                     phrm_policy_req_fdr_task_fd fdr_task_fd,
                     phrm_policy_req_fdr_state_operator fdr_op,
                     phrm_policy_req_fdr_state_operators_prob fdr_ops_prob,
                     void *userdata)
{
    PolicyImpl service(fdr_task_fd, fdr_op, fdr_ops_prob, userdata);
    return runPolicyService(service, url);
}
