#include "../interfaces/conn.h"
#include "../interfaces/message.h"

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>


bool Conn::Open(size_t id, bool create)
{
    bool success = false;
    _owner = create;
    int shm_flg = 0666;
    if (create)
    {
        std::cout << "Creating connection with id: " << id << std::endl;
        shm_flg |= IPC_CREAT;
    }
    else
    {
        std::cout << "Getting connection with id: " << id << std::endl;
    }
    _id = shmget(id, sizeof(Message), shm_flg);
    if (_id < 0)
    {
        std::cout << "ERROR: shmget failed" << std::endl;
    }
    else
    {
        success = true;
    }
    return success;
}

bool Conn::Read(void* buf, size_t count)
{
    bool success = true;
    void* shm_return = shmat(_id, nullptr, 0);
    if (shm_return == (void*)-1) {
        std::cout << "ERROR: shmat can't attach to memory";
        success = false;
    }
    else
    {
        memcpy(buf, shm_return, count);
        if (shmdt(shm_return) == -1) {
            std::cout << "ERROR: shmdt can't deattach";
            success = false;
        }
    }
    return success;
}

bool Conn::Write(void* buf, size_t count)
{
    bool success = true;
    void* shm_return = shmat(_id, nullptr, 0);
    if (shm_return == (void*)-1) {
        std::cout << "ERROR: shmat can't attach to memory";
        success = false;
    }
    else
    {
        memcpy(shm_return, buf, count);
        if (shmdt(shm_return) == -1) {
            std::cout << "ERROR: shmdt can't deattach";
            success = false;
        }
    }
    return success;
}

bool Conn::Close()
{
    bool success = true;
    if (_owner && shmctl(_id, IPC_RMID, nullptr) < 0)
    {
        success = false;
    }
    return success;
}