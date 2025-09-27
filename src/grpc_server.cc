#include "trading.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <thread>
#include <cmath>
#include <cstdlib>
#include <ctime>

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
        std::cout << "Starting market data stream for symbol: " << req->symbol() << std::endl;
        
        uint32_t counter = 0;
        while (!ctx->IsCancelled()) {
            MarketData data;
            data.set_symbol(req->symbol());
            
            double base_price = 100.0;
            double price_variation = 5.0 * sin(counter * 0.1) + (rand() % 10 - 5) * 0.1;
            data.set_price(base_price + price_variation);
            data.set_timestamp(time(nullptr));
            
            if (!writer->Write(data)) {
                break;
            }
            
            std::cout << "Sent market data: " << req->symbol() 
                     << " @ $" << data.price() << std::endl;
            
            counter++;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        
        std::cout << "Market data stream ended for: " << req->symbol() << std::endl;
        return Status::OK;
    }
};

    void RunServer(std::string port) {
        std::string addr("0.0.0.0");
        std::string socket = addr;
        std::ostringstream ss;
        ss << addr << ":" << port;
        TradingServiceImpl service;

        ServerBuilder builder;


        // std::string addr_str("0.0.0.0:1234");
        builder.AddListeningPort(ss.str(), grpc::InsecureServerCredentials());
        // builder.AddListeningPort(addr_str, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());

        std::cout << "Market server listening on " << addr << ":" << port << std::endl;
        
        server->Wait();
    }

// int main() {
//     RunServer();
//     return 0;
// }