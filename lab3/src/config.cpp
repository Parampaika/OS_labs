#include <cstdlib>
#include <iostream>

#include "../interfaces/config.h"

Config * Config::_instance = nullptr;

Config::Config()= default;

Config * Config::get_instance()
{
    if (!_instance)
        _instance = new (std::nothrow) Config();
    return _instance;
}

void Config::destroy()
{
    delete _instance;
}

bool Config::load(int argc, char ** argv)
{
    if (argc < 6) {
        std::cout << "Wrong num of args. Expected: 6, got: " << argc << std::endl;
        return false;
    }
    size_t val;
    bool success = true;
    std::string s;
    for (int i = 1; i <= 5; i++) {
        s = argv[i];
        if (str_to_value(s, val)) {
            _values[(ParamType)i] = val;
        } else {
            success = false;
            break;
        }
    }
    std::cout << _values[ParamType::THREADS_READERS] << std::endl;
    if (_values[ParamType::THREADS_READERS] * _values[ParamType::READERS_NUM] != _values[ParamType::THREADS_WRITERS] * _values[ParamType::WRITERS_NUM]) {
        std::cout << "Wrong arguments: number_of_writers * number_of_records != number_of_readers * number_of_readings." << std::endl;
        success = false;
    }
    return success;
}

size_t Config::get_value(ParamType type) const
{
    auto it = _values.find(type);
    if (it != _values.cend())
        return it->second;
    return 0;
}

bool Config::str_to_value(std::string &s, size_t &res) {
    try
    {
        res = std::stoul(s);
    }
    catch (std::exception &e)
    {
        std::cout << "Couldn't get number from string." << std::endl;
        return false;
    }
    return true;
}
