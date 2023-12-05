#include "goat.h"

#include <ctime>
#include <cerrno>
#include <iostream>
#include "../utils/utils.h"
#include <cstring>
#include <csignal>
#include <unistd.h>

void Goat::Start()
{
    Message msg;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += TIMEOUT;
    if (sem_timedwait(semaphore_client, &ts) == -1)
    {
        Terminate(errno);
    }
    msg.number = GetRand(RAND_LIMIT_ALIVE);
    connection.Write(&msg, sizeof(msg));
    sem_post(semaphore_host);
    while (true)
    {
        sem_wait(semaphore_client);
        if (connection.Read(&msg, sizeof(Message)))
        {
            std::cout << "--------------------------------" << std::endl;
            std::cout << "Status: " << ((msg.status == Status::ALIVE) ? "alive" : "dead") << std::endl;
            std::cout << "Wolf number: " << msg.number << std::endl;
            if (msg.status == Status::ALIVE)
            {
                msg.number = GetRand(RAND_LIMIT_ALIVE);
            }
            else
            {
                msg.number = GetRand(RAND_LIMIT_DEAD);
            }
            std::cout << "Goat number: " << msg.number << std::endl;
            connection.Write(&msg, sizeof(msg));
        }
        sem_post(semaphore_host);
    }
}

bool Goat::OpenConnection()
{
    bool res = false;
    if (connection.Open(host_pid, false))
    {
        semaphore_host = sem_open(SEMAPHORE_HOST_NAME, 0);
        semaphore_client = sem_open(SEMAPHORE_CLIENT_NAME, 0);
        if (semaphore_host == SEM_FAILED || semaphore_client == SEM_FAILED)
        {
            std::cout << "ERROR: sem_open failed with error: " << strerror(errno) << std::endl;
        }
        else
        {
            res = true;
            std::cout << "pid of created client is: " << getpid() << std::endl;
            kill(host_pid, SIGUSR1);
        }
    }
    return res;
}

Goat& Goat::GetInstance(int host_pid)
{
    static Goat instance(host_pid);
    return instance;
}

void Goat::Terminate(int signum)
{
    kill(host_pid, SIGUSR2);
    std::cout << "Goat::Terminate()" << std::endl;
    if (sem_close(semaphore_client) == -1 || sem_close(semaphore_host) == -1)
    {
        std::cout << "Failed: " << strerror(errno) << std::endl;
        exit(errno);
    }
    if (!connection.Close())
    {
        std::cout << "Failed: " << strerror(errno) << std::endl;
        exit(errno);
    }
    exit(signum);
}

Goat::Goat(int pid)
{
    std::cout << "host pid: " << pid << std::endl;
    host_pid = pid;
    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
}

void Goat::SignalHandler(int signum)
{
    Goat& instance = Goat::GetInstance(13);
    instance.Terminate(signum);
}

