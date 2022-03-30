#ifndef GLARE_PRIORITY_QUEUE_H
#define GLARE_PRIORITY_QUEUE_H

#include "Heap.h"

namespace glare{

    template<typename _KeyType, typename _ValType, typename _Pred = greater<_KeyType>, typename _Alloc = default_allocator<_ValType> >
    class CPriorityQueue
    {
        typedef _KeyType                key_type;
        typedef _Pred                   key_compare; // binary predicate.

        typedef _ValType                value_type;
        typedef value_type*             pointer;
        typedef const value_type*       const_pointer;
        typedef value_type&             reference;
        typedef const value_type&       const_reference;
        typedef std::size_t             size_type;
        typedef std::ptrdiff_t          difference_type;

        typedef _Alloc                  allocator_type;

    public:
        CPriorityQueue(int _size);
        ~CPriorityQueue();

        void enqueue(const key_type& _key, const_reference _val);
        void dequeue(pointer _val);

        void peek_copy(pointer _val) const;
        pointer peek();
        const_pointer peek() const;

        size_type size() const;
        void resize(size_type _size);
        void clear();

        bool empty() const;

    private:
        typedef CHeap<key_type, value_type, key_compare, allocator_type>  queue_type;
        queue_type  m_queue;
    };

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::CPriorityQueue(int _size) : m_queue(_size)
    {

    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::~CPriorityQueue()
    {

    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::enqueue(const key_type& _key, const_reference _val)
    {
        m_queue.insert(_key, _val);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::dequeue(pointer _val)
    {
        m_queue.getRoot(_val);
        m_queue.removeRoot();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::peek_copy(pointer _val) const
    {
        m_queue.getRoot(_val);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc> inline
    typename CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::pointer 
             CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::peek()
    {
        return m_queue.getRoot();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc> inline
    typename CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::const_pointer
             CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::peek() const
    {
        return m_queue.getRoot();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc> inline
    typename CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::size_type 
             CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::size() const
    {
        return m_queue.size();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::resize(size_type _size)
    {
        m_queue.resize(_size);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::clear()
    {
        m_queue.clear();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline bool CPriorityQueue<_KeyType, _ValType, _Pred, _Alloc>::empty() const
    {
        return m_queue.empty();
    }

} // End of namespace

#endif
