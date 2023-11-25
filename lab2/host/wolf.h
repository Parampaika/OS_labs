#ifndef OS_LABS_WOLF_H
#define OS_LABS_WOLF_H

#include <csignal>
#include "../interfaces/member.h"
#include "client_info.h"

class Wolf: public Member
{
public:
    void Start();
    bool OpenConnection();

    static Wolf& GetInstance();
    Wolf(Wolf&) = delete;
    Wolf(const Wolf&) = delete;
    Wolf& operator=(const Wolf&) = delete;
private:
    ClientInfo client_info;
    int curr_num;
    static const int RAND_LIMIT = 100;

    void Terminate(int signum);

    Wolf();
    Message Step(Message& msg);
    void GetNumber();
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);
};

#endif //OS_LABS_WOLF_H
