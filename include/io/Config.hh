#include <filesystem>
namespace io {

class Config {
private:
    std::filesystem::path _config_path;
public:
    Config() = default;
    Config(std::filesystem::path&);
    ~Config() = default;

    [[nodiscard]]
    virtual bool parse_config() noexcept;
};
} // End io namespace