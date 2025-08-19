#include "Publisher.hh"
#include <iostream>
#include <thread>
#include <random>

Publisher::Publisher(std::shared_ptr<grpc::Channel> channel) 
    : _stub(trading::TradingService::NewStub(channel)) {}

grpc::Status Publisher::send_order(const trading::Order order) {
    grpc::ClientContext context;
    trading::OrderAck ack;

    grpc::Status status = _stub->SendOrder(&context, order, &ack);

    if (!ack.success()) {
        std::cerr << "[Error] Order rejected by server: " << order.symbol() << std::endl;
    }
    std::cout << "Ack received: " << ack.message() << std::endl;
    return status;
}

void Publisher::start_market_data_stream(const std::string& symbol) {
    if (_streaming.load()) {
        std::cout << "Market data stream already running" << std::endl;
        return;
    }
    
    _streaming.store(true);
    _stream_thread = std::thread(&Publisher::stream_market_data, this, symbol);
    std::cout << "Started market data stream for " << symbol << std::endl;
}

void Publisher::stop_market_data_stream() {
    if (!_streaming.load()) {
        return;
    }
    
    _streaming.store(false);
    if (_stream_thread.joinable()) {
        _stream_thread.join();
    }
    std::cout << "Stopped market data stream" << std::endl;
}

void Publisher::stream_market_data(const std::string& symbol) {
    trading::MarketRequest req;
    req.set_symbol(symbol);

    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<trading::MarketData>> reader(
        _stub->StreamMarketData(&context, req));

    trading::MarketData data;
    while (reader->Read(&data) && _streaming.load()) {
        std::cout << "Received market data: " << data.symbol() 
                  << " @ $" << data.price() 
                  << " (timestamp: " << data.timestamp() << ")" << std::endl;
        
        trading::Order order = generate_order_from_market_data(data);
        grpc::Status status = send_order(order);
        
        if (!status.ok()) {
            std::cerr << "[Error] Failed to send order: " << status.error_message() << std::endl;
        }
    }
    
    grpc::Status status = reader->Finish();
    if (!status.ok()) {
        std::cerr << "[Error] Market data stream ended with error: " << status.error_message() << std::endl;
    }
    
    _streaming.store(false);
}

trading::Order Publisher::generate_order_from_market_data(const trading::MarketData& data) {
    trading::Order order;
    order.set_symbol(data.symbol());
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> quantity_dist(10, 100);
    static std::uniform_int_distribution<> side_dist(1, 2);
    static std::uniform_real_distribution<> price_adjustment(-2.0, 2.0);
    
    order.set_quantity(quantity_dist(gen));
    order.set_side(static_cast<trading::Side>(side_dist(gen)));
    
    double adjusted_price = data.price() + price_adjustment(gen);
    order.set_price(adjusted_price);
    
    std::cout << "Generated order: " << (order.side() == trading::BUY ? "BUY" : "SELL")
              << " " << order.quantity() << " " << order.symbol() 
              << " @ $" << order.price() << std::endl;
    
    return order;
}