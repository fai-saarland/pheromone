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

class PolicyImpl final : public phrm::policy::Policy::Service {
  private:
    phrm_policy_req_fdr_task_fd fdr_task_fd;
    phrm_policy_req_fdr_operator fdr_op;
    void *userdata;

  public:
    explicit PolicyImpl(phrm_policy_req_fdr_task_fd fdr_task_fd,
                        phrm_policy_req_fdr_operator fdr_op,
                        void *userdata)
        : fdr_task_fd(fdr_task_fd),
          fdr_op(fdr_op),
          userdata(userdata)
    {
    }

    grpc::Status GetFDRTaskFD(grpc::ServerContext *ctx,
                              const phrm::policy::RequestFDRTaskFD *req,
                              phrm::policy::ResponseFDRTaskFD *res) override
    {
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
};

int phrmPolicyServer(const char *url,
                        phrm_policy_req_fdr_task_fd fdr_task_fd,
                        phrm_policy_req_fdr_operator fdr_op,
                        void *userdata)
{
    PolicyImpl service(fdr_task_fd, fdr_op, userdata);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(url, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << url << std::endl;
    server->Wait();

    return 0;
}
