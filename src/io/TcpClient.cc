#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "MarketData.hh"

#include "io/TcpClient.hh"

// TCPSubscriber::TCPSubscriber()
//     : _ip("0.0.0.0"), _port{1337}
// {}

// TCPSubscriber::TCPSubscriber(const std::string& ip, const uint16_t port)
//     : _ip{ip}, _port{port}
// { }

// std::optional<bool> TCPSubscriber::init_socket()
// {
//     _sock = socket(AF_INET, SOCK_STREAM, 0);

//     if (_sock < 0)
//     {
//         perror("socket");
//         return false;
//     }
//     sockaddr_in hint{};
//     hint.sin_family = AF_INET;
//     hint.sin_port = htons(_port);
//     if (inet_pton(AF_INET, _ip.c_str(), std::addressof(hint.sin_addr)) <= 0)
//     {
//         perror("inet_pton");
//         return false;
//     }

//     if (connect(_sock, (struct sockaddr*) std::addressof(hint), sizeof(hint)) < 0)
//     {
//         // std::cerr << "Could not establish connection." << std::endl;
//         perror("connect");
//         return false;
//     }

//     MarketData tick{};
//     while (true) 
//     {
//         ssize_t n = recv(_sock, &tick, sizeof(tick), MSG_WAITALL);
//         if (n == 0) 
//         {
//             std::cout << "[Client] Server closed connection\n";
//             break;
//         } else if (n < 0) {
//             perror("recv");
//             break;
//         }

//         std::cout << "[Client] Tick " << tick.seq_num
//                   << " " << std::string(tick.symbol, 4)
//                   << " bid=" << tick.bid
//                   << " ask=" << tick.ask
//                   << " ts=" << tick.timestamp_ns
//                   << std::endl;
//     }
//     return true; // Socket sock is destroyed by RAII if we haven't returned by now
// }