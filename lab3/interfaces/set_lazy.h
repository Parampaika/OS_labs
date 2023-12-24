#ifndef LAB3_SET_LAZY_H
#define LAB3_SET_LAZY_H

#include "set.h"

template<typename T, typename Limits = limits_t<T>>
class SetLazy : public Set<T, Limits>
{
public:
    SetLazy();

    ~SetLazy();

    bool add(const T &item);
    bool remove(const T &item);
    bool contains(const T &item);

protected:
    class NodeLazy: public Set<T, Limits>::Node {
    public:
        NodeLazy(const T &item);

        bool _marked;
    };

private:
    static NodeLazy * create_head();
    bool validate(NodeLazy *pred, NodeLazy *curr);

    std::vector<NodeLazy*> _removed;
    pthread_mutex_t _mutex;
};

#include "set_lazy.hpp"

#endif // LAB3_SET_LAZY_H
