#include "Publisher.hh"
#include <iostream>


Publisher::Publisher(std::shared_ptr<grpc::Channel> channel) 
    : _stub(trading::TradingService::NewStub(channel)) {}

grpc::Status Publisher::send_order(const trading::Order& order) {
    grpc::ClientContext context;
    trading::OrderAck ack;

    grpc::Status status = _stub->SendOrder(&context, order, &ack);

    if (!ack.success()) {
        std::cerr << "[Error] Order rejected by server: " << order.symbol() << std::endl;
    }
    std::cout << "Ack received: " << ack.message() << std::endl;
    return status;
}

// void StreamMarketData() {
//     MarketRequest req;
//     req.set_symbol("AAPL");

//     ClientContext context;
//     std::unique_ptr<grpc::ClientReader<MarketData>> reader(
//         stub_->StreamMarketData(&context, req));

//     MarketData data;
//     while (reader->Read(&data)) {
//     std::cout << "Market: " << data.symbol() << " $" << data.price()
//                 << " @ " << data.timestamp() << std::endl;
//     }
// }