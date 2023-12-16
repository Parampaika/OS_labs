#include <cstdlib>
#include <iostream>

#include "../interfaces/test.h"
#include "../interfaces/config.h"

void load_config(int argc, char **argv)
{
    Config * config = nullptr;
    if (argc > 1)
        config = Config::get_instance();
    if (!config || !config->load(argc, argv))
    {
        std::cout << "Couldn't parse arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void TestSet(SetType setType) {
    std::cout << "Readers tests:" << std::endl;
    RunTest(setType, DataType::FIXED, TestType::TEST_READERS);
    RunTest(setType, DataType::RANDOM, TestType::TEST_READERS);
    std::cout << "Writers tests:" << std::endl;
    RunTest(setType, DataType::FIXED, TestType::TEST_WRITERS);
    RunTest(setType, DataType::RANDOM, TestType::TEST_WRITERS);
    std::cout << "Common tests:" << std::endl;
    RunTest(setType, DataType::FIXED, TestType::TEST_COMMON);
    RunTest(setType, DataType::RANDOM, TestType::TEST_COMMON);
}

int main(int argc, char **argv)
{
    load_config(argc, argv);

    std::cout << "------------------Tests for SetLazy------------------------------" << std::endl;
    TestSet(SetType::LAZY);

    Config::destroy();
    return 0;
}
