#include "../interfaces/conn.h"
#include "../interfaces/message.h"

#include <iostream>
#include <sys/shm.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <mqueue.h>
#include <cerrno>


bool Conn::Open(size_t id, bool create)
{
    bool res = false;
    _owner = create;
    _name = "/LAB2_QUEUE";
    int mq_flg = O_RDWR;
    int mq_perm = 0666;
    if (_owner)
    {
        std::cout << "Creating connection with id: " << id << std::endl;
        mq_flg |= O_CREAT;
        int maxmsg = 1;
        int msgsize = sizeof(Message);
        struct mq_attr attr = ((struct mq_attr){0, maxmsg, msgsize, 0, {0}});
        _id = mq_open(_name.c_str(), mq_flg, mq_perm, &attr);
    }
    else
    {
        std::cout << "Getting connection with id: " << id << std::endl;
        _id = mq_open(_name.c_str(), mq_flg);
    }
    if (_id == -1)
    {
        std::cout << "ERROR: mq_open failed, errno: " << strerror(errno) << std::endl;
    }
    else
    {
        res = true;
    }
    return res;
}

bool Conn::Read(void* buf, size_t count)
{
    bool success = false;
    if (mq_receive(_id, (char*)buf, count, nullptr) == -1)
    {
        std::cout << "ERROR: mq_recieve failed, errno: " << strerror(errno) << std::endl;
    } else {
        success = true;
    }
    return success;
}

bool Conn::Write(void* buf, size_t count)
{
    bool success = false;
    if (mq_send(_id, (char*) buf, count, 0) == -1)
    {
        std::cout << "ERROR: mq_send failed, errno: " << strerror(errno) << std::endl;
    }
    else {
        success = true;
    }
    return success;
}

bool Conn::Close()
{
    bool res = false;
    if (mq_close(_id) == 0)
    {
        if (!_owner || (_owner && mq_unlink(_name.c_str()) == 0))
        {
            res = true;
        }
    }
    return res;
}
