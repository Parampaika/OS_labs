#ifndef OS_LABS_GOAT_H
#define OS_LABS_GOAT_H

#include "../interfaces/member.h"

class Goat: public Member
{
public:
    void Start();
    bool OpenConnection();
    static Goat& GetInstance(int host_pid);
    Goat(Goat&) = delete;
    Goat(const Goat&) = delete;
    Goat& operator=(const Goat&) = delete;
private:
    int host_pid;
    static const int RAND_LIMIT_ALIVE = 100;
    static const int RAND_LIMIT_DEAD = 50;

    void Terminate(int signum);

    Goat(int host_pid);
    static void SignalHandler(int signum);
};

#endif //OS_LABS_GOAT_H
