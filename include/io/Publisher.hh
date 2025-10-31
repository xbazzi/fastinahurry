#pragma once

#include "trading.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <atomic>
#include <thread>

class Publisher {
public:
    explicit Publisher(std::shared_ptr<grpc::Channel> channel);

    grpc::Status send_order(const trading::Order order);
    void start_market_data_stream(const std::string& symbol);
    void stop_market_data_stream();
    
private:
    std::unique_ptr<trading::TradingService::Stub> _stub;
    std::atomic<bool> _streaming{false};
    std::thread _stream_thread;
    
    void stream_market_data(const std::string& symbol);
    trading::Order generate_order_from_market_data(const trading::MarketData& data);
};