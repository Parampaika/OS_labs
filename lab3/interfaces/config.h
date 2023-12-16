#ifndef LAB3_CONFIG_H
#define LAB3_CONFIG_H

#include <string>
#include <map>

enum class ParamType
{
    WRITERS_NUM = 1,
    READERS_NUM = 2,
    THREADS_WRITERS = 3,
    THREADS_READERS = 4,
    TIME_ITERATIONS = 5
};

class Config
{
public:
    static Config * get_instance();
    static void destroy();
    
    bool load(int argc, char ** argv);

    size_t get_value(ParamType type) const;

    Config(Config const&) = delete;
    Config& operator=(Config const&) = delete;

private:
    Config();
    ~Config() = default;

    static bool str_to_value(std::string &s, size_t &res);

    static Config * _instance;

    std::map<ParamType, size_t> _values;
};

#endif // LAB3_CONFIG_H
