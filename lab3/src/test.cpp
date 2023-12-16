#include <pthread.h>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

#include "../interfaces/test.h"
#include "../interfaces/set_lazy.h"
#include "../interfaces/config.h"

class ThreadInfo
{
public:
    ThreadInfo() = default;
    Set<int> *set;
    std::vector<int> *data, *array;
    int numberWriters, numberReaders, numberRecords, numberReadings, index;

    ThreadInfo(Set<int> *s, std::vector<int> *d, std::vector<int> *a, int nWriters, int nReaders, int nRecords, int nReadings, int i)
            : set(s), data(d), array(a), numberWriters(nWriters), numberReaders(nReaders), numberRecords(nRecords), numberReadings(nReadings), index(i)
    {};
};

std::vector<ThreadInfo> CreateThreadsInfo(int numberThreads, Set<int> *set, std::vector<int> *data, std::vector<int> *array, int numberWriters, int numberReaders, int numberRecords, int numberReadings)
{
    std::vector<ThreadInfo> threadsInfo(numberThreads);
    for (int i = 0; i < numberThreads; i++)
        threadsInfo[i] = ThreadInfo(set, data, array, numberWriters, numberReaders, numberRecords, numberReadings, i);

    return threadsInfo;
}

std::vector<pthread_t> CreateThreads(int numberThreads, void *(*startRoutine) (void *), std::vector<ThreadInfo> &threadsInfo)
{
    std::vector<pthread_t> threads(numberThreads);
    for (int i = 0; i < numberThreads; i++)
        pthread_create(&threads[i], nullptr, startRoutine, &threadsInfo[i]);

    return threads;
}

void JoinThreads(const std::vector<pthread_t> &threads)
{
    for (auto thread : threads) {
        pthread_join(thread, nullptr);
    }
}

void* Write(void *arg)
{
    ThreadInfo *threadInfo = (ThreadInfo*)arg;
    for (int i = 0; i < threadInfo->numberRecords; i++)
        threadInfo->set->add(threadInfo->data->at(threadInfo->index + i * threadInfo->numberWriters));

    return nullptr;
}

void* Read(void *arg)
{
    ThreadInfo *threadInfo = (ThreadInfo*)arg;
    for (int i = 0; i < threadInfo->numberReadings; i++)
    {
        while (!threadInfo->set->remove(threadInfo->data->at(threadInfo->index + i * threadInfo->numberReaders)))
            sched_yield();

        threadInfo->array->at(threadInfo->data->at(threadInfo->index + i * threadInfo->numberReaders)) += 1;
    }

    return nullptr;
}

bool CheckTestWriters(Set<int> &set, std::vector<int> &array)
{
    for (auto i : array)
        if (!set.contains(i))
            return false;

    return true;
}

bool CheckTestReaders(Set<int> &set, std::vector<int> &array)
{
    for (auto i : array)
        if (i != 1)
            return false;

    return set.empty();
}

Set<int>* CreateSet()
{
    return new SetLazy<int>();
}

std::vector<int> CreateDataFixed(int n)
{
    std::vector<int> data(n);
    iota(data.begin(), data.end(), 0);

    return data;
}

std::vector<int> CreateDataRandom(int n)
{
    std::vector<int> data = CreateDataFixed(n);
    random_shuffle(data.begin(), data.end());

    return data;
}

std::vector<int> CreateData(DataType dataType, int n)
{
    if (dataType == DataType::RANDOM)
        return CreateDataRandom(n);

    return CreateDataFixed(n);
}

std::string GetNameTest(TestType testType, SetType setType, DataType dataType)
{
    std::string tt;
    if (testType == TestType::TEST_READERS)
        tt = "Test Readers";
    else if (testType == TestType::TEST_WRITERS)
        tt = "Test Writers";
    else
        tt = "Test Common";
    return (setType == SetType::LAZY ? "Lazy " : "") + tt + (dataType == DataType::RANDOM ? " random" : " fixed");

}

void PrintTestResult(TestType testType, SetType setType, DataType dataType, bool success, double time)
{
    std::cout << GetNameTest(testType, setType, dataType) << (success ? ": SUCCESS " : ": FAIL ") << time << std::endl;
}

void GetTime(struct timespec *time)
{
    clock_gettime(CLOCK_REALTIME, time);
}

void GetResultTime(const struct timespec &start_time, double &time)
{
    struct timespec cur_time;
    clock_gettime(CLOCK_REALTIME, &cur_time);
    __time_t sec = cur_time.tv_sec - start_time.tv_sec;
    __syscall_slong_t nsec = cur_time.tv_nsec - start_time.tv_nsec;
    time = sec + (double) nsec / 1e9;
}

void RunTestWriters(SetType setType, DataType dataType, int numberWriters, int numberRecords, int numIterations)
{
    bool success = true;
    double timeTotal = 0;
    double timeCurr;
    struct timespec start_time;

    for (int i = 0; i < numIterations; i++)
    {
        Set<int> *set = CreateSet();
        std::vector<int> data = CreateData(dataType, numberWriters * numberRecords);
        std::vector<ThreadInfo> threadsInfo = CreateThreadsInfo(numberWriters, set, &data, nullptr, numberWriters, 0, numberRecords, 0);

        GetTime(&start_time);
        std::vector<pthread_t> threads = CreateThreads(numberWriters, Write, threadsInfo);
        JoinThreads(threads);
        GetResultTime(start_time, timeCurr);

        success &= CheckTestWriters(*set, data);
        timeTotal += timeCurr;

        delete set;
    }

    PrintTestResult(TestType::TEST_WRITERS, setType, dataType, success, timeTotal / numIterations);
}

void RunTestReaders(SetType setType, DataType dataType, int numberReaders, int numberReadings, int numIterations)
{
    bool success = true;
    double timeTotal = 0;
    double timeCurr;
    struct timespec start_time;

    for (int i = 0; i < numIterations; i++)
    {
        Set<int> *set = CreateSet();
        std::vector<int> data = CreateData(dataType, numberReaders * numberReadings);
        std::vector<int> array = std::vector<int>(numberReaders * numberReadings, 0);
        std::vector<ThreadInfo> threadsInfo = CreateThreadsInfo(numberReaders, set, &data, &array, 0, numberReaders, 0, numberReadings);

        for (auto d : data)
            set->add(d);
        GetTime(&start_time);
        std::vector<pthread_t> threads = CreateThreads(numberReaders, Read, threadsInfo);
        JoinThreads(threads);
        GetResultTime(start_time, timeCurr);

        success &= CheckTestReaders(*set, array);
        timeTotal += timeCurr;

        delete set;
    }

    PrintTestResult(TestType::TEST_READERS, setType, dataType, success, timeTotal / numIterations);
}

void RunTestCommon(SetType setType, DataType dataType, int numberWriters, int numberReaders, int numberRecords, int numberReadings, int numIterations)
{
    bool success = true;
    double timeTotal = 0;
    double timeCurr;
    struct timespec start_time;

    for (int i = 0; i < numIterations; i++)
    {
        Set<int> *set = CreateSet();
        std::vector<int> data = CreateData(dataType, numberWriters * numberRecords);
        std::vector<int> array = std::vector<int>(numberWriters * numberRecords, 0);

        std::vector<ThreadInfo> writersInfo = CreateThreadsInfo(numberWriters, set, &data, nullptr, numberWriters, 0, numberRecords, 0);
        std::vector<ThreadInfo> readersInfo = CreateThreadsInfo(numberReaders, set, &data, &array, 0, numberReaders, 0, numberReadings);

        GetTime(&start_time);
        std::vector<pthread_t> writers = CreateThreads(numberWriters, Write, writersInfo);
        std::vector<pthread_t> readers = CreateThreads(numberReaders, Read, readersInfo);
        JoinThreads(writers);
        JoinThreads(readers);
        GetResultTime(start_time, timeCurr);

        success &= CheckTestReaders(*set, array);
        timeTotal += timeCurr;

        delete set;
    }

    PrintTestResult(TestType::TEST_COMMON, setType, dataType, success, timeTotal / numIterations);
}

void RunTest(SetType setType, DataType dataType, TestType testType) {
    Config *config = Config::get_instance();
    size_t numIterations = config->get_value(ParamType::TIME_ITERATIONS);
    int numThreads, numOperations;
    switch (testType) {
        case TestType::TEST_WRITERS:
            numThreads = config->get_value(ParamType::THREADS_WRITERS);
            numOperations = config->get_value(ParamType::WRITERS_NUM);
            RunTestWriters(setType, dataType, numThreads, numOperations, numIterations);
            break;
        case TestType::TEST_READERS:
            numThreads = config->get_value(ParamType::THREADS_READERS);
            numOperations = config->get_value(ParamType::READERS_NUM);
            RunTestReaders(setType, dataType, numThreads, numOperations, numIterations);
            break;
        case TestType::TEST_COMMON:
            RunTestCommon(setType, dataType, config->get_value(ParamType::THREADS_WRITERS), config->get_value(ParamType::THREADS_READERS),
                           config->get_value(ParamType::WRITERS_NUM), config->get_value(ParamType::READERS_NUM), numIterations);
            break;
    }
}
