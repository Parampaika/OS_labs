#include <iostream>
#include "goat.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "ERROR: wrong args. Expected: ./client_<connection_type> <host pid>" << std::endl;
        return 1;
    }
    int pid;
    try
    {
        pid = std::stoi(argv[1]);
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: can't get int pid from argument" << std::endl;
        return 1;
    }
    std::cout << "Starting client..." << std::endl;
    Goat& goat = Goat::GetInstance(pid);
    if (goat.OpenConnection())
    {
        goat.Start();
    }
    return 0;
}

