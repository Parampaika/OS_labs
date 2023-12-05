#ifndef OS_LABS_MESSAGE_H
#define OS_LABS_MESSAGE_H

enum class Status
{
    ALIVE,
    DEAD
};

struct Message
{
    Status status;
    int number;

    Message(Status st = Status::ALIVE, int num = 0) : status(st), number(num)
    {
    }
};

#endif //OS_LABS_MESSAGE_H
