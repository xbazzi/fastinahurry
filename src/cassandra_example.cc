// #include <cassandra.h>
// #include <iostream>
// #include <string>
// #include <memory>

// class CassandraManager {
// private:
//     CassCluster* cluster_;
//     CassSession* session_;
    
// public:
//     CassandraManager() : cluster_(nullptr), session_(nullptr) {
//         cluster_ = cass_cluster_new();
//         session_ = cass_session_new();
//     }
    
//     ~CassandraManager() {
//         if (session_) {
//             cass_session_free(session_);
//         }
//         if (cluster_) {
//             cass_cluster_free(cluster_);
//         }
//     }
    
//     bool connect(const std::string& hosts = "127.0.0.1") {
//         cass_cluster_set_contact_points(cluster_, hosts.c_str());
//         cass_cluster_set_port(cluster_, 9042);
        
//         CassFuture* connect_future = cass_session_connect(session_, cluster_);
        
//         if (cass_future_error_code(connect_future) == CASS_OK) {
//             std::cout << "Connected to Cassandra cluster at " << hosts << std::endl;
//             cass_future_free(connect_future);
//             return true;
//         } else {
//             const char* message;
//             size_t message_length;
//             cass_future_error_message(connect_future, &message, &message_length);
//             std::cerr << "Failed to connect: " << std::string(message, message_length) << std::endl;
//             cass_future_free(connect_future);
//             return false;
//         }
//     }
    
//     bool createKeyspace(const std::string& keyspace_name) {
//         std::string query = "CREATE KEYSPACE IF NOT EXISTS " + keyspace_name + 
//                            " WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}";
        
//         return executeQuery(query);
//     }
    
//     bool useKeyspace(const std::string& keyspace_name) {
//         std::string query = "USE " + keyspace_name;
//         return executeQuery(query);
//     }
    
//     bool createTable() {
//         std::string query = "CREATE TABLE IF NOT EXISTS trading_data ("
//                            "id UUID PRIMARY KEY, "
//                            "symbol TEXT, "
//                            "price DECIMAL, "
//                            "volume BIGINT, "
//                            "timestamp TIMESTAMP)";
        
//         return executeQuery(query);
//     }
    
//     bool insertTradingData(const std::string& symbol, double price, int64_t volume) {
//         std::string query = "INSERT INTO trading_data (id, symbol, price, volume, timestamp) "
//                            "VALUES (uuid(), ?, ?, ?, toTimestamp(now()))";
        
//         CassStatement* statement = cass_statement_new(query.c_str(), 3);
        
//         cass_statement_bind_string(statement, 0, symbol.c_str());
//         cass_statement_bind_double(statement, 1, price);
//         cass_statement_bind_int64(statement, 2, volume);
        
//         CassFuture* future = cass_session_execute(session_, statement);
        
//         bool success = (cass_future_error_code(future) == CASS_OK);
//         if (!success) {
//             const char* message;
//             size_t message_length;
//             cass_future_error_message(future, &message, &message_length);
//             std::cerr << "Insert failed: " << std::string(message, message_length) << std::endl;
//         }
        
//         cass_future_free(future);
//         cass_statement_free(statement);
        
//         return success;
//     }
    
//     bool selectTradingData() {
//         std::string query = "SELECT symbol, price, volume, timestamp FROM trading_data LIMIT 10";
        
//         CassStatement* statement = cass_statement_new(query.c_str(), 0);
//         CassFuture* future = cass_session_execute(session_, statement);
        
//         if (cass_future_error_code(future) == CASS_OK) {
//             const CassResult* result = cass_future_get_result(future);
//             CassIterator* iterator = cass_iterator_from_result(result);
            
//             std::cout << "\n--- Trading Data ---" << std::endl;
//             std::cout << "Symbol\t\tPrice\t\tVolume\t\tTimestamp" << std::endl;
//             std::cout << "------\t\t-----\t\t------\t\t---------" << std::endl;
            
//             while (cass_iterator_next(iterator)) {
//                 const CassRow* row = cass_iterator_get_row(iterator);
                
//                 const char* symbol;
//                 size_t symbol_length;
//                 cass_value_get_string(cass_row_get_column(row, 0), &symbol, &symbol_length);
                
//                 double price;
//                 cass_value_get_double(cass_row_get_column(row, 1), &price);
                
//                 cass_int64_t volume;
//                 cass_value_get_int64(cass_row_get_column(row, 2), &volume);
                
//                 cass_int64_t timestamp;
//                 cass_value_get_int64(cass_row_get_column(row, 3), &timestamp);
                
//                 std::cout << std::string(symbol, symbol_length) << "\t\t"
//                          << price << "\t\t"
//                          << volume << "\t\t"
//                          << timestamp << std::endl;
//             }
            
//             cass_result_free(result);
//             cass_iterator_free(iterator);
//         } else {
//             const char* message;
//             size_t message_length;
//             cass_future_error_message(future, &message, &message_length);
//             std::cerr << "Select failed: " << std::string(message, message_length) << std::endl;
//         }
        
//         cass_future_free(future);
//         cass_statement_free(statement);
        
//         return true;
//     }
    
// private:
//     bool executeQuery(const std::string& query) {
//         CassStatement* statement = cass_statement_new(query.c_str(), 0);
//         CassFuture* future = cass_session_execute(session_, statement);
        
//         bool success = (cass_future_error_code(future) == CASS_OK);
//         if (!success) {
//             const char* message;
//             size_t message_length;
//             cass_future_error_message(future, &message, &message_length);
//             std::cerr << "Query failed: " << std::string(message, message_length) << std::endl;
//         }
        
//         cass_future_free(future);
//         cass_statement_free(statement);
        
//         return success;
//     }
// };

// void demonstrateCassandraUsage() {
//     CassandraManager cassandra;
    
//     if (!cassandra.connect()) {
//         std::cerr << "Failed to connect to Cassandra" << std::endl;
//         return;
//     }
    
//     if (!cassandra.createKeyspace("trading")) {
//         std::cerr << "Failed to create keyspace" << std::endl;
//         return;
//     }
    
//     if (!cassandra.useKeyspace("trading")) {
//         std::cerr << "Failed to use keyspace" << std::endl;
//         return;
//     }
    
//     if (!cassandra.createTable()) {
//         std::cerr << "Failed to create table" << std::endl;
//         return;
//     }
    
//     std::cout << "Inserting sample trading data..." << std::endl;
//     cassandra.insertTradingData("AAPL", 150.25, 1000);
//     cassandra.insertTradingData("GOOGL", 2800.50, 500);
//     cassandra.insertTradingData("MSFT", 420.75, 750);
    
//     cassandra.selectTradingData();
// }