#include <iostream>
#include "wolf.h"

int main()
{
    std::cout << "Starting host..." << std::endl;
    Wolf& wolf = Wolf::GetInstance();
    if (wolf.OpenConnection())
    {
        wolf.Start();
    }
    return 0;
}