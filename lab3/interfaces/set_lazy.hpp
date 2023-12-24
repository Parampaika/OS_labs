#include "set_lazy.h"

template<typename T, typename L>
SetLazy<T, L>::SetLazy(): Set<T, L>(create_head()), _mutex(PTHREAD_MUTEX_INITIALIZER)
{}

template<typename T, typename L>
SetLazy<T, L>::~SetLazy()
{
    for (NodeLazy *to_del : _removed)
        delete to_del;
    pthread_mutex_destroy(&_mutex);
}

template<typename T, typename L>
typename SetLazy<T, L>::NodeLazy* SetLazy<T, L>::create_head() {
    L limits;
    T min = limits.min(), max = limits.max();
    NodeLazy *head = new (std::nothrow) NodeLazy(min);
    if (!head)
        return nullptr;
    head->_next = new (std::nothrow) NodeLazy(max);
    if (!head->_next)
    {
        delete head;
        return nullptr;
    }
    return head;
}

template<typename T, typename L>
bool SetLazy<T, L>::add(const T &item)
{
    bool success = false;
    while (true)
    {
        typename Set<T, L>::Node *pred = this->_head, *curr = pred->_next;
        this->loop(pred, curr, item);
        if (validate((NodeLazy*)pred, (NodeLazy*)curr))
        {
            if (item < curr->_item)
            {
                typename Set<T, L>::Node *node = new (std::nothrow) NodeLazy(item);
                if (node)
                {
                    node->_next = curr;
                    pred->_next = node;
                    success = true;
                }
            }
            this->unlock(pred, curr);
            break;
        }
        this->unlock(pred, curr);
    }
    return success;
}

template<typename T, typename L>
bool SetLazy<T, L>::remove(const T &item)
{
    bool success = false;
    while (true)
    {
        typename Set<T, L>::Node *pred = this->_head, *curr = pred->_next;
        this->loop(pred, curr, item);
        if (validate((NodeLazy*)pred, (NodeLazy*)curr))
        {
            if (!(item < curr->_item))
            {
                pred->_next = curr->_next;
                pthread_mutex_lock(&_mutex);
                ((NodeLazy*)curr)->_marked = true;
                _removed.push_back((NodeLazy*)curr);
                pthread_mutex_unlock(&_mutex);
                success = true;
            }
            this->unlock(curr, pred);
            break;
        }
        this->unlock(curr, pred);
    }
    return success;
}

template<typename T, typename L>
bool SetLazy<T, L>::contains(const T &item)
{
    typename Set<T, L>::Node *curr = this->_head;
    while (curr->_item < item)
        curr = curr->_next;
    NodeLazy *lazyCurr = (NodeLazy*)curr;
    return !(lazyCurr->_item < item) && !(item < lazyCurr->_item) && !lazyCurr->_marked;
}

template<typename T, typename L>
bool SetLazy<T, L>::validate(NodeLazy *pred, NodeLazy *curr)
{
    return !pred->_marked && !curr->_marked && pred->_next == curr;
}

template<typename T, typename L>
SetLazy<T, L>::NodeLazy::NodeLazy(const T &item): Set<T, L>::Node(item) {
    _marked = false;
}

