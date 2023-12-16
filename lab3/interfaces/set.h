#ifndef LAB3_SET_H
#define LAB3_SET_H

#include <functional>

#include "limit.h"

template<typename T, typename Limits = limits_t<T>>
class Set
{
public:
    virtual bool add(const T &item) = 0;
    virtual bool remove(const T &item) = 0;
    virtual bool contains(const T &item) = 0;

    bool empty();

    virtual ~Set();

    Set(Set const&) = delete;
    Set& operator=(Set const&) = delete;

protected:
    class Node
    {
    public:
        Node(const T &item);
        virtual ~Node();

        int lock();
        int unlock();

        T _item;
        Node *_next;

        Node() = delete;
        Node(Node const&) = delete;
        Node& operator=(Node const&) = delete;

    private:
        pthread_mutex_t _mutex;
    };

    Set();
    Set(Node* head);

    void unlock(typename Set<T, Limits>::Node *pred, typename Set<T, Limits>::Node *curr);
    void loop(typename Set<T, Limits>::Node *&pred, typename Set<T, Limits>::Node *&curr, const T &item);

    Node *_head;
};

#include "set.hpp"

#endif