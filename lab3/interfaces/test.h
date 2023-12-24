#ifndef LAB3_TESTS_H
#define LAB3_TESTS_H

enum class SetType
{
    LAZY
};

enum class DataType
{
    RANDOM = 0,
    FIXED
};

enum class TestType
{
    TEST_WRITERS = 0,
    TEST_READERS,
    TEST_COMMON
};

void RunTest(SetType setType, DataType dataType, TestType testType);

#endif // LAB3_TESTS_H
