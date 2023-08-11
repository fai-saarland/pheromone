/***
 * Copyright (c)2023 Daniel Fiser <danfis@danfis.cz>. All rights reserved.
 * This file is part of pheromone licensed under 3-clause BSD License (see file
 * LICENSE, or https://opensource.org/licenses/BSD-3-Clause)
 */

#include "pheromone/policy_client.h"
#include "rpc/policy.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

struct phrm_policy {
    std::unique_ptr<phrm::policy::Policy::Stub> stub;
    explicit phrm_policy(const std::string &addr)
    {
        grpc::ChannelArguments args;
        args.SetMaxReceiveMessageSize(1024*1024*1024);
        std::shared_ptr<grpc::Channel> custom_channel(grpc::CreateCustomChannel(addr, grpc::InsecureChannelCredentials(), args));
        stub = phrm::policy::Policy::NewStub(custom_channel);
    }
};

phrm_policy_t *phrmPolicyConnect(const char *url)
{
    phrm_policy_t *p = new phrm_policy_t(url);
    return p;
}

void phrmPolicyDel(phrm_policy_t *p)
{
    delete p;
}

char *phrmPolicyFDRTaskFD(phrm_policy_t *p)
{
    grpc::ClientContext ctx;
    phrm::policy::RequestFDRTaskFD req;
    phrm::policy::ResponseFDRTaskFD res;
    grpc::Status st = p->stub->GetFDRTaskFD(&ctx, req, &res);
    if (!st.ok()){
        // TODO: Refactor
        std::cerr << "Error: Cannot get response for GetFDRTaskFD"
            << (st.error_code() == grpc::StatusCode::UNIMPLEMENTED
                    ? " (UNIMPLEMENTED)" : "")
            << (st.error_code() == grpc::StatusCode::INTERNAL
                    ? " (INTERNAL)" : "")
            << " :: " << st.error_message()
            << std::endl;
        return NULL;
    }

    if (!res.has_task()){
        std::cerr << "Error: Invalid format of the response to GetFDRTaskFD" << std::endl;
        return NULL;
    }

    int size = res.task().size();
    char *out = (char *)malloc(sizeof(char) * (size + 1));
    strcpy(out, res.task().c_str());
    out[size] = '\x0';
    return out;
}

int phrmPolicyFDRStateOperator(phrm_policy_t *p, const int *state, int state_size)
{
    grpc::ClientContext ctx;
    phrm::policy::RequestFDRStateOperator req;
    phrm::fdr::State *req_state = req.mutable_state();
    for (int i = 0; i < state_size; ++i)
        req_state->add_val(state[i]);
    phrm::policy::ResponseFDRStateOperator res;
    grpc::Status st = p->stub->GetFDRStateOperator(&ctx, req, &res);
    if (!st.ok()){
        // TODO: Refactor
        std::cerr << "Error: Cannot get response for GetFDRStateOperator"
            << (st.error_code() == grpc::StatusCode::UNIMPLEMENTED
                    ? " (UNIMPLEMENTED)" : "")
            << (st.error_code() == grpc::StatusCode::INTERNAL
                    ? " (INTERNAL)" : "")
            << " :: " << st.error_message()
            << std::endl;
        return -1;
    }

    if (!res.has_operator_()){
        std::cerr
            << "Error: Invalid format of the response to GetFDRStateOperator"
            << std::endl;
        return -1;
    }
    return res.operator_();
}
