#ifndef OS_LABS_CLIENT_INFO_H
#define OS_LABS_CLIENT_INFO_H

struct ClientInfo
{
    int pid;
    bool attached;
    int count_dead;

    explicit ClientInfo(int pid) : pid(pid), attached(pid != 0), count_dead(0)
    {
    }
};

#endif //OS_LABS_CLIENT_INFO_H
