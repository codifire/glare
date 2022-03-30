#ifndef DOUBLY_LINK_LIST_H
#define DOUBLY_LINK_LIST_H

#include "engine_common.h"
#include <list>
#include <assert.h>


namespace glare
{

    template <typename T, typename Alloc = default_allocator<T> >
    class DLinkList
    {
    public:

        typedef T                   value_type;
        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;
        typedef value_type&         reference;
        typedef const value_type&   const_reference;
        typedef std::size_t         size_type;
        typedef std::ptrdiff_t      difference_type;

        class DListNode;
        typedef DListNode           node_type;
        typedef DListNode*          node_pointer;
        typedef const DListNode*    const_node_pointer;

    protected:

        struct DListNode
        {
            value_type   m_data;
            node_pointer m_next;
            node_pointer m_prev;

            DListNode(const value_type& ref, node_pointer nxt, node_pointer prv):
                m_data(ref), 
                m_next(nxt), 
                m_prev(prv) 
                {}
        };
        DLinkList(const DLinkList&);
    
    public:

        class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type>
        {
            friend class DLinkList;
            node_pointer ptr;
            node_pointer next() { ptr = ptr->m_next; return ptr; }
            node_pointer prev() { ptr = ptr->m_prev; return ptr; }

        public:
            iterator(node_pointer p=0) :ptr(p) {}
            iterator(const iterator& itr) : ptr(itr.ptr) {}

            iterator& operator++(/*Pre-increment*/) {next(); return *this;}
            iterator operator++(int /*Post-increment*/) {iterator tmp(*this); next(); return tmp;}

            iterator& operator--(/*Pre-decrement*/) {prev(); return *this;}
            iterator operator--(int /*Post-decrement*/) {iterator tmp(*this); prev(); return tmp;}

            bool operator==(const iterator& rhs) const {return ptr == rhs.ptr;}
            bool operator!=(const iterator& rhs) const {return ptr != rhs.ptr;}

            reference operator*() { return ptr->m_data; }
            pointer operator->() { return &(ptr->m_data); }
        };

        class const_iterator : public std::iterator<std::bidirectional_iterator_tag, value_type>
    {
        const_node_pointer ptr;
        const_node_pointer next() { ptr = ptr->m_next; return ptr; }
        const_node_pointer prev() { ptr = ptr->m_prev; return ptr; }

    public:
        const_iterator(node_pointer p) :ptr(p) {}
        const_iterator(const const_iterator& itr) : ptr(itr.ptr) {}

        const_iterator& operator++(/*Pre-increment*/) {next(); return *this;}
        const_iterator operator++(int /*Post-increment*/) {const_iterator tmp(*this); next(); return tmp;}

        const_iterator& operator--(/*Pre-decrement*/) {prev(); return *this;}
        const_iterator operator--(int /*Post-decrement*/) {const_iterator tmp(*this); prev(); return tmp;}

        bool operator==(const const_iterator& rhs) const {return ptr == rhs.ptr;}
        bool operator!=(const const_iterator& rhs) const {return ptr != rhs.ptr;}

        const_reference operator*() const { return ptr->m_data; }
        const_pointer operator->() const { return &(ptr->m_data); }
    };

        DLinkList() : m_root(nullptr), m_tail(nullptr), m_size(0) {}
        ~DLinkList()
        {
            destroy();
        }

        iterator begin() { return iterator(m_root); }
        iterator end() const { return iterator(nullptr); }

        const_iterator cbegin(){ return const_iterator(m_root); }
        const_iterator cend() const { return const_iterator(nullptr); }

        void push_back(const_reference ref)
        {
            node_pointer last = endNode();
            node_pointer node = constructNode(ref, nullptr, last);

            if(last)
            {
               last->m_next = node;
            }
            else
            {
                // Then it's a first node, head/tail points the same node.
                m_root = node;
            }
            m_tail = node;
            m_size++;
        }
        void push_front(const_reference ref)
        {
            node_pointer node = constructNode(ref, m_root, nullptr);

            if(m_root) { m_root->m_prev = node; }
            else { m_tail = node; }

            m_root = node;
            m_size++;
        }
        void insert_after(const_reference ref, size_t idx)
        {
            assert(idx < m_size && m_size > 0);
            node_pointer beforeNode = getNode(idx);
            node_pointer afterNode = beforeNode->m_next;

            node_pointer currentNode = constructNode(ref, afterNode, beforeNode);
            beforeNode->m_next = currentNode;

            if(afterNode) 
            {
               // Insertion is happening in the middle of two nodes.
               afterNode->m_prev = currentNode;
            }
            else 
            {
               // Insertion is happening at the end.
               m_tail = currentNode;
            }
            m_size++;
        }
        void insert(iterator itr, const_reference ref)
        {
            assert(m_size > 0);
            node_pointer beforeNode = itr.ptr;
            node_pointer afterNode = beforeNode->m_next;
            node_pointer node = constructNode(ref, afterNode, beforeNode);

            beforeNode->m_next = node;

            if(afterNode)
            {
                afterNode->m_prev = node;
            }
            else
            {
                m_tail = node;
            }
            m_size++;
        }

        value_type pop_front()
        {
            assert(m_size>0);
            return popNodeHelper(m_root);
        }
        value_type pop_back()
        {
            assert(m_size>0);
            return popNodeHelper(endNode());
        }
        value_type pop(size_t idx)
        {
            assert(m_size > 0 && idx < m_size);
            node_pointer temp = m_root;
            size_t cnt = 0;

            while (temp != nullptr)
            {
                if (cnt == idx)
                {
                    break;
                }
                temp = temp->m_next;
                cnt++;
            }
            return popNodeHelper(temp);
        }
        iterator erase(iterator itr)
        {
            iterator itrNextNode(itr.ptr->m_next);
            removeNode(itr.ptr);
            return itrNextNode;
        }

        reference front() { assert(m_size>0); return m_root->m_data; }
        reference back() { assert(m_size>0); return m_tail->m_data; }
                        reference at(size_t idx) 
    {
        assert(m_size>0); 
        return getNode(idx)->m_data; 
    }

        size_t size() const { return m_size; }
                                                size_t length() const 
    {
        const_node_pointer temp = m_root;
        size_t cnt = 0;

        while (temp != nullptr)
        {
            ++cnt;
            temp = temp->m_next;
        }
        return cnt;
    }

        bool empty() { return m_size == 0; }
        // Protected Members
    protected:

        void removeNode(node_pointer node)
        {
            node_pointer next = node->m_next;
            node_pointer prev = node->m_prev;

            if (next) { next->m_prev = prev; }
            else { m_tail = prev; }

            if (prev) { prev->m_next = next; }
            else { m_root = next; }

            destroyNode(node);
            --m_size;
        }
        value_type popNodeHelper(node_pointer node)
        {
            value_type ret(node->m_data);
            removeNode(node);
            return ret;
        }
        void destroyNode(node_pointer node)
    {
        // m_nodeAllocator.destroy(node); // Never called node ctor to avoid temporaries. Check constructNode helper.
        m_allocator.destroy(&node->m_data);
        m_nodeAllocator.deallocate(node, 1);
    }
        node_pointer constructNode(const_reference ref, node_pointer next, node_pointer prev)
        {
            node_pointer node = m_nodeAllocator.allocate(1);
            node->m_next = next;
            node->m_prev = prev;
            // m_nodeAllocator.construct(node, DListNode(ref, next, prev)); // Avoid temporary. Check destroyNode helper.
            m_allocator.construct( &node->m_data, ref);
            return node;
        }
        node_pointer traverseTillEnd()
    {
        node_pointer temp = m_root;
        while (temp && temp->m_next) temp = temp->m_next;
        return temp;
    }
        node_pointer getNode(size_t idx)
    {
        node_pointer temp = m_root;
        size_t cnt = 0;

        while (temp != nullptr)
        {
            if (cnt == idx)
            {
                break;
            }
            ++cnt;
            temp = temp->m_next;
        }

        return temp;
    }
        node_pointer endNode() { return m_tail; }
        void destroy()
        {
            while(m_root)
            {
                node_pointer deletePtr = m_root;
                m_root = m_root->m_next;
                destroyNode(deletePtr);
            }
            m_root = nullptr;
            m_tail = nullptr;
            m_size = 0;
        }

        node_pointer m_root;
        node_pointer m_tail;
        size_t       m_size;
        Alloc        m_allocator;
        typename Alloc::template rebind<DListNode>::other m_nodeAllocator;
    };

}// end of namespace
#endif
