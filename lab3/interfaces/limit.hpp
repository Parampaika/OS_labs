#include <limits>

#include "limit.h"

template<typename T>
T limits_t<T>::max()
{
    return std::numeric_limits<T>().max();
}

template<typename T>
T limits_t<T>::min()
{
    return std::numeric_limits<T>().lowest();
}
