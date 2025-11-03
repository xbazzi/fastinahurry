#include "trading.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using trading::TradingService;
using trading::Order;
using trading::OrderAck;
using trading::MarketRequest;
using trading::MarketData;

class TraderClient {
 public:
  TraderClient(std::shared_ptr<Channel> channel)
      : stub_(TradingService::NewStub(channel)) {}

  // void SendOrder() {
  //   Order order;
  //   order.set_symbol("AAPL");
  //   order.set_quantity(100);
  //   order.set_price(150.0);
  //   order.set_side();

  //   OrderAck ack;
  //   ClientContext context;

  //   Status status = stub_->SendOrder(&context, order, &ack);
  //   if (status.ok()) {
  //     std::cout << "Order ack: " << ack.message() << std::endl;
  //   } else {
  //     std::cerr << "SendOrder failed: " << status.error_message() << std::endl;
  //   }
  // }

  // void StreamMarketData() {
  //   MarketRequest req;
  //   req.set_symbol("AAPL");

  //   ClientContext context;
  //   std::unique_ptr<grpc::ClientReader<MarketData>> reader(
  //       stub_->StreamMarketData(&context, req));

  //   MarketData data;
  //   while (reader->Read(&data)) {
  //     std::cout << "Market: " << data.symbol() << " $" << data.price()
  //               << " @ " << data.timestamp() << std::endl;
  //   }
  // }

 private:
  std::unique_ptr<TradingService::Stub> stub_;
};

// int main() {
//   TraderClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
//   client.SendOrder();
//   client.StreamMarketData();
//   return 0;
// }