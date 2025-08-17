#pragma once

#include "trading.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>

class Publisher {
public:
    explicit Publisher(std::shared_ptr<grpc::Channel> channel);

    grpc::Status send_order(const trading::Order order);
private:
    std::unique_ptr<trading::TradingService::Stub> _stub;
};