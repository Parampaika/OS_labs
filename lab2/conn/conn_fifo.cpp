#include "../interfaces/conn.h"
#include "../interfaces/message.h"

#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>


bool Conn::Open(size_t id, bool create)
{
    bool res = false;
    _owner = create;
    _name = "/tmp/lab2_fifo";
    int fifo_flg = 0777;
    if (create)
    {
        std::cout << "Creating connection with id: " << id << std::endl;
    } else {
        std::cout << "Getting connection with id: " << id << std::endl;
    }
    if (_owner && mkfifo(_name.c_str(), fifo_flg) == -1)
    {
        std::cout << "ERROR: mkfifo failed: " << strerror(errno) << std::endl;
    } else {
        _id = open(_name.c_str(), O_RDWR);
        if (_id == -1)
        {
            std::cout << "ERROR: open failed: " << strerror(errno) << std::endl;
        }
        else
        {
            res = true;
        }
    }
    return res;
}

bool Conn::Read(void* buf, size_t count)
{
    bool success = false;
    if (read(_id, buf, count) == -1)
    {
        std::cout << "ERROR: reading failed: " << strerror(errno) << std::endl;
    } else {
        success = true;
    }
    return success;
}

bool Conn::Write(void* buf, size_t count)
{
    bool success = false;
    if (write(_id, buf, count) == -1)
    {
        std::cout << "ERROR: writing failed: " << strerror(errno) << std::endl;
    } else {
        success = true;
    }
    return success;
}

bool Conn::Close()
{
    bool res = true;
    if ((close(_id) < 0) || (_owner && remove(_name.c_str()) < 0))
    {
        std::cout << "ERROR: close failed: " << strerror(errno) << std::endl;
        res = false;
    }
    return res;
}
