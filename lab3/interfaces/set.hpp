#include "set.h"

template<typename T, typename L>
Set<T, L>::Set(Node* head) : _head(head)
{}

template<typename T, typename L>
Set<T, L>::Set()
{
    L limits;
    T min = limits.min();
    T max = limits.max();
    _head = new Node(min);
    _head->_next = new Node(max);
}

template<typename T, typename L>
Set<T, L>::~Set()
{
    Node *to_del;
    while (_head)
    {
        to_del = _head;
        _head = _head->_next;
        delete to_del;
    }
}

template<typename T, typename L>
bool Set<T, L>::empty()
{
    L limits;
    T min = limits.min();
    T max = limits.max();
    Node *next = _head->_next;
    return (!(_head->_item < min) && !(min < _head->_item) && !(next->_item < max) && !(max < next->_item));
}

template<typename T, typename L>
void Set<T, L>::unlock(typename Set<T, L>::Node *pred, typename Set<T, L>::Node *curr)
{
//    std::cout << "pred unlock: " << pred->_item << std::endl;
    pred->unlock();
//    std::cout << "curr unlock: " << curr->_item << std::endl;
    curr->unlock();
}

template<typename T, typename L>
void Set<T, L>::loop(typename Set<T, L>::Node *&pred, typename Set<T, L>::Node *&curr, const T &item)
{
    while (curr->_item < item)
    {
        pred = curr;
        curr = pred->_next;
    }
    pred->lock();
    curr->lock();
}

template<typename T, typename L>
Set<T, L>::Node::Node(const T &item) : _item(item), _next(nullptr), _mutex(PTHREAD_MUTEX_INITIALIZER)
{}

template<typename T, typename L>
Set<T, L>::Node::~Node()
{
    pthread_mutex_destroy(&_mutex);
}

template<typename T, typename L>
int Set<T, L>::Node::lock()
{
    return pthread_mutex_lock(&_mutex);
}

template<typename T, typename L>
int Set<T, L>::Node::unlock()
{
    return pthread_mutex_unlock(&_mutex);
}

