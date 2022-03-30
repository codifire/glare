#ifndef SINGLE_LINK_LIST_H
#define SINGLE_LINK_LIST_H

#include "memory\allocators.h"
#include <list>
#include <assert.h>

template <typename T, typename Alloc = glare::Allocator<T> >
class SLinkList
{
protected:
    
    struct SListNode
    {
        T m_data;
        SListNode* m_next;
    };
    SLinkList(const SLinkList&);
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef SListNode* node_pointer;
    typedef const SListNode* const_node_pointer;

    class iterator : public std::iterator<std::forward_iterator_tag, value_type>
    {
        friend class SLinkList;
        node_pointer ptr;
        node_pointer next() { ptr = ptr->m_next; return ptr; }
    public:
        iterator(node_pointer p) :ptr(p) {}
        iterator(const iterator& itr) : ptr(itr.ptr) {}

        iterator& operator++(/*Pre-increment*/) {next(); return *this;}
        iterator operator++(int /*Post-increment*/) {iterator tmp(*this); next(); return tmp;}

        bool operator==(const iterator& rhs) const {return ptr == rhs.ptr;}
        bool operator!=(const iterator& rhs) const {return ptr != rhs.ptr;}

        reference operator*() { return ptr->m_data; }
        pointer operator->() { return &(ptr->m_data); }
    };
    class const_iterator : public std::iterator<std::forward_iterator_tag, value_type>
    {
        const_node_pointer ptr;
        node_pointer next() { ptr = ptr->m_next; return ptr; }
    public:
        const_iterator(node_pointer p) :ptr(p) {}
        const_iterator(const const_iterator& itr) : ptr(itr.ptr) {}

        const_iterator& operator++(/*Pre-increment*/) {next(); return *this;}
        const_iterator operator++(int /*Post-increment*/) {const_iterator tmp(*this); next(); return tmp;}

        bool operator==(const const_iterator& rhs) const {return ptr == rhs.ptr;}
        bool operator!=(const const_iterator& rhs) const {return ptr != rhs.ptr;}

        const_reference operator*() const { return ptr->m_data; }
        const_pointer operator->() const { return &(ptr->m_data); }
    };

    SLinkList() : m_root(nullptr), m_size(0) {}
    ~SLinkList() 
    {
        while(m_root)
        {
            node_pointer deletePtr = m_root;
            m_root = m_root->m_next;
            destroyNode(deletePtr);
        }
    }

    iterator begin() { return iterator(m_root); }
    iterator end() const { return iterator(nullptr); }

    const_iterator cbegin(){ return const_iterator(m_root); }
    const_iterator cend() const { return const_iterator(nullptr); }

    void push_back(const_reference ref)
    {
        if (m_root == nullptr)
        {
            push_front(ref);
            return;
        }

        node_pointer node = constructNode(ref);
        node->m_next = nullptr;

        node_pointer end = endNode();
        end->m_next = node;
        m_size++;
    }
    void push_front(const_reference ref)
    {
        node_pointer node = constructNode(ref);
        node->m_next = m_root;
        m_root = node;
        m_size++;
    }
    void insert_after(const_reference ref, size_t idx)
    {
        assert(idx < m_size && m_size > 0);
        node_pointer beforeNode = getNode(idx);
        node_pointer afterNode = beforeNode->m_next;
        node_pointer currentNode = constructNode(ref);

        beforeNode->m_next = currentNode;
        currentNode->m_next = afterNode;
        m_size++;
    }

    value_type pop_front()
    {
        assert(m_size>0);
        value_type ret(m_root->m_data);
        node_pointer delPtr = m_root;
        m_root = m_root->m_next;
        m_size--;
        destroyNode(delPtr);
        return ret;
    }
    value_type pop_back()
    {
        assert(m_size>0);
        node_pointer temp = m_root;
        node_pointer prev = m_root;

        while (temp != nullptr)
        {
            if (temp->m_next)
            {
                prev = temp;
                temp = temp->m_next;
            }
            else
            {
                break;
            }
        }

        if (temp == prev)
        {
            // Then its the root node that is to be deleted.
            m_root = nullptr;
        }
        else
        {
            prev->m_next = nullptr;
        }
        
        value_type ret(temp->m_data);
        destroyNode(temp);
        --m_size;
        return ret;
    }
    value_type pop(size_t idx)
    {
        assert(m_size > 0 && idx < m_size);
        node_pointer temp = m_root;
        node_pointer prev = m_root;
        size_t cnt = 0;

        while (temp != nullptr)
        {
            if (cnt == idx)
            {
                break;
            }
            prev = temp;
            temp = temp->m_next;
            cnt++;
        }

        if (temp == prev)
        {
            // If first node.
            m_root = m_root->m_next;
        }
        else
        {
            prev->m_next = temp->m_next;
        }

        value_type ret(temp->m_data);
        destroyNode(temp);
        --m_size;
        return ret;
    }
    void erase(iterator itr)
    {
        node_pointer temp = m_root;
        node_pointer prev = m_root;
        const node_pointer cmpNode = itr.ptr;
        
        while(temp != nullptr)
        {
            if (temp == cmpNode)
            {
                break;
            }
            else
            {
                prev = temp;
                temp = temp->m_next;
            }
        }

        if (temp)
        {
            if (temp == prev)
            {
                // If first node.
                m_root = m_root->m_next;
            }
            else
            {
                prev->m_next = temp->m_next;
            }
            destroyNode(temp);
            --m_size;
        }
    }
    void insert(iterator itr, const_reference ref)
    {
        assert(m_size > 0);
        node_pointer beforeNode = getNode(itr);
        node_pointer afterNode = beforeNode->m_next;
        node_pointer currentNode = constructNode(ref);

        beforeNode->m_next = currentNode;
        currentNode->m_next = afterNode;
        m_size++;
    }

    reference front() { assert(m_size>0); return m_root->m_data; }
    reference back() { assert(m_size>0); return endNode()->m_data; }
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
    // Protected Members
protected:

    void destroyNode(node_pointer node)
    {
        m_allocator.destroy(&node->m_data);
        m_nodeAllocator.deallocate(node, 1);
    }
    node_pointer constructNode()
    {
        return m_nodeAllocator.allocate(1);
    }
    node_pointer constructNode(const_reference ref)
    {
        node_pointer node = constructNode();
        m_allocator.construct( &node->m_data, ref);
        return node;
    }
    node_pointer endNode()
    {
        node_pointer temp = m_root;
        node_pointer prev = nullptr;

        while (temp != nullptr)
        {
            prev = temp;
            temp = temp->m_next;
        }
        return prev;
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
    node_pointer getNode(iterator itr) 
    {
        node_pointer temp = m_root;
        const node_pointer cmpNode = itr.ptr;

        while(temp)
        {
            if (temp == cmpNode)
            {
                break;
            }
            else
            {
                temp = temp->m_next;
            }
        }

        return temp;
    }
    node_pointer m_root;
    size_t       m_size;

    Alloc        m_allocator;
    typename Alloc::template rebind<SListNode>::other m_nodeAllocator;
};

#endif
