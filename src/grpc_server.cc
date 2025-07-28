#include "trading.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <thread>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using trading::TradingService;
using trading::Order;
using trading::OrderAck;
using trading::MarketRequest;
using trading::MarketData;

class TradingServiceImpl final : public TradingService::Service {
    Status SendOrder(ServerContext* ctx, const Order* req,
                     OrderAck* res) override {
        std::cout << "Order: " << req->symbol() << " " << req->side()
                  << " " << req->quantity() << " " << req->price() << std::endl;

        res->set_success(true);
        res->set_message("Order received");
        return Status::OK;
    }

    Status StreamMarketData(ServerContext* ctx, const MarketRequest* req,
                            grpc::ServerWriter<MarketData>* writer) override {
        
            uint32_t max_market_streams = 5;
            for (uint32_t i = 0; i < max_market_streams; i++) {
                MarketData data;
                data.set_symbol(req->symbol());
                data.set_price(100 + i);
                data.set_timestamp(time(nullptr));
                writer->Write(data);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return Status::OK;
    }
};

    void RunServer() {
        std::string addr("0.0.0.0:50051");
        TradingServiceImpl service;

        ServerBuilder builder;
        builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());

        std::cout << "Market server listening on" << addr << std::endl;
        
        server->Wait();
    }

// int main() {
//     RunServer();
//     return 0;
// }