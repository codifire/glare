#ifndef _GLARE_HEAP_H_
#define _GLARE_HEAP_H_

#include "BTreeArrayNode.h"
#include "engine_common.h"
#include "GlareCoreUtility.h"
#include <algorithm>


namespace glare
{
    template<typename _KeyType, typename _ValType, typename _Pred = greater<_KeyType>, typename _Alloc = default_allocator<_ValType> >
    class CHeap
    {
    public:
        typedef _KeyType                key_type;
        typedef _Pred                   key_compare; // binary predicate.

        typedef _ValType                value_type;
        typedef value_type*             pointer;
        typedef const value_type*       const_pointer;
        typedef value_type&             reference;
        typedef const value_type&       const_reference;
        typedef std::size_t             size_type;
        typedef std::ptrdiff_t          difference_type;

        typedef GLARE_PAIR<key_type, value_type>                        pair_type;

        typedef typename BTreeArrayNode<pair_type>::node_type           node_type;
        typedef typename BTreeArrayNode<pair_type>::node_pointer        node_pointer;
        typedef typename BTreeArrayNode<pair_type>::const_node_pointer  const_node_pointer;
        typedef typename node_type::index_type                          index_type;

        typedef _Alloc                                                  allocator_type;
        typedef typename _Alloc::template rebind<node_type>             node_allocator_type;

        CHeap(size_type _size);
        virtual ~CHeap();

        // Copy
        CHeap(const CHeap&);
        CHeap& operator = (const CHeap&);

        // Remove
        void removeRoot();

        // Insert
        void insert(const key_type& _key, const_reference _data);
        void insert(const pair_type& _pair);

        // Get Root
        pointer getRoot();
        const_pointer getRoot() const;

        // Return reference to data corresponding to key.
        bool getRoot(pointer _data) const;

        bool empty() const     { return (size() == 0); }
        size_type size() const { return m_uSize; }
        size_type capacity() const { return m_uCapacity; }
        void resize(size_type _size);

        // All the resources are freed after this call.
        void clear();

    private:
        /*
        Function: Restores the order property of heap to the tree between root and bottom.
        Precondition: The order property of the heap may be violated only by the root node of the tree.
        Postcondition: The order property applies to all elements of the heap.
        */
        void reheapDown(index_type _nodeIndex);
        void reheapUp(index_type _nodeIndex);

        void swap(node_type& _first, node_type& _second);

        void makeCopy(const CHeap& _other) const;

        // Compare the nodes with the key_compare function and returns the index of the node for which
        // either key_compare returns true or the one which is valid among the two nodes.
        index_type compareNodes(index_type _leftNode, index_type _rightNode);

        node_type& getNode(index_type _index) { return m_pRoot[_index]; }
        bool isIndexValid(index_type _index) { return _index < m_uCapacity; }
        bool isNodeValid(index_type _index) { return (isIndexValid(_index) && getNode(_index).isValid()); }

        index_type getLeftChildIndex(index_type _index) const { return(_index*2 + 1); }
        index_type getRightChildIndex(index_type _index) const { return(_index*2 + 2); }
        index_type getParentIndex(index_type _index) const { return static_cast<index_type>((_index-1)/2); }

        bool hasLeftChild(index_type _index) const { return isNodeValid( getLeftChildIndex(_index) ); }
        bool hasRightChild(index_type _index) const { return isNodeValid( getRightChildIndex(_index) ); }
        bool isLeafNode(index_type _index) const { return (!hasLeftChild(_index) && !hasRightChild(_index)); }

        node_pointer        m_pRoot;
        size_type           m_uSize;
        size_type           m_uCapacity;
        key_compare         m_binPredicate;
        node_allocator_type m_nodeAllocator;
    }; // End Class Declaration.
    
    /// Public interface definition ///////////////////////////////////////////////////////////////////
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    CHeap<_KeyType, _ValType, _Pred, _Alloc>::CHeap(size_type _size) : m_pRoot(NULL)
                                                                     , m_uSize(0)
                                                                     , m_uCapacity(0)
                                                                     , m_binPredicate()
                                                                     , m_nodeAllocator()
    {
        resize(_size);
    }
    
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    CHeap<_KeyType, _ValType, _Pred, _Alloc>::~CHeap()
    {
        clear();
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    CHeap<_KeyType, _ValType, _Pred, _Alloc>::CHeap(const CHeap& _other) : m_pRoot(NULL)
                                                                         , m_uSize(0)
                                                                         , m_uCapacity(0)
                                                                         , m_binPredicate()
                                                                         , m_nodeAllocator()
    {
        makeCopy(_other);
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    CHeap<_KeyType, _ValType, _Pred, _Alloc>& 
        CHeap<_KeyType, _ValType, _Pred, _Alloc>::operator = (const CHeap& _other)
    {
        if (this != &_other)
        {
            clear();
            makeCopy(_other);
        }
        return *this;
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::makeCopy(const CHeap& _other) const
    {
        if(node_pointer nodePtr = m_nodeAllocator.allocate(_other.m_uCapacity))
        {
            // Copy construct all the nodes.
            for (unsigned i=0; i<_other.m_uCapacity; ++i)
            {
                m_nodeAllocator.construct(&nodePtr[i], _other.m_pRoot[i]);
            }

            m_uCapacity = _other.m_uCapacity;
            m_uSize = _other.m_uSize;
            m_pRoot = nodePtr;
        }
    }

    // Get Root
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc> 
    typename CHeap<_KeyType, _ValType, _Pred, _Alloc>::pointer 
             CHeap<_KeyType, _ValType, _Pred, _Alloc>::getRoot()
    {
        if (m_pRoot && m_pRoot[0].isValid()){
            return &m_pRoot[0];
        }
        return NULL;
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    typename CHeap<_KeyType, _ValType, _Pred, _Alloc>::const_pointer 
             CHeap<_KeyType, _ValType, _Pred, _Alloc>::getRoot() const
    {
        if (m_pRoot && m_pRoot[0].isValid()){
            return &m_pRoot[0];
        }
        return NULL;
    }

    // Return reference to data corresponding to key.
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    bool CHeap<_KeyType, _ValType, _Pred, _Alloc>::getRoot(pointer _data) const
    {
        if (m_pRoot && m_pRoot[0].isValid())
        {
            // Deep Copy the object.
            *_data = m_pRoot[0].m_data.second;
            return true;
        }
        return false;
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::removeRoot()
    {
        // We can call destructor of the node itself so that destructor for the
        // contained object is called. Remember we aren't freeing the memory.
        m_nodeAllocator.destroy(&m_pRoot[0]);

        // There is a purpose for placing this statements here.
        --m_uSize;

        if (m_uSize > 0){
            // Notice we are only copying the data, index of the node stays the same.
            // We don't need to do deep copy as we will invalidate one of the nodes.
            // m_pRoot[0].m_data = m_pRoot[m_uSize-1].m_data;  // don't do this!

            GLARE_MEMCPY(&m_pRoot[0].m_data, // We only need the key/value pair not the index info.
                         &m_pRoot[m_uSize].m_data, 
                         sizeof(m_pRoot[m_uSize].m_data));
            m_pRoot[m_uSize].invalidate(); // m_uSize was decremented before.

            // Restore the order property of the heap.
            reheapDown(0);
        }
        else{
            m_pRoot[0].invalidate();
        }
    }

    // Insert
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::insert(const key_type& _key, const_reference _data)
    {
        insert(pair_type(_key, _data));
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::insert(const pair_type& _pair)
    {
        // Is there enough space to insert?
        if ((m_uSize+1) > m_uCapacity)
        {// Grow in size by the below formula. Formula simply adds a new level to the binary tree.
            resize((2*m_uCapacity) + 1);
        }

        // Insert at next open slot so that the heap stays in the shape property.
        m_pRoot[m_uSize].m_index = m_uSize;
        m_pRoot[m_uSize].m_data = _pair; // Make deep-copy.

        ++m_uSize;

        reheapUp(m_uSize-1); // Index of the last node.
    }

    // All the resources are freed after this call.
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::clear()
    {
        if (m_pRoot)
        {
            for (int i=m_uCapacity-1; i >= 0; --i) {
                m_nodeAllocator.destroy(m_pRoot[i]);
            }

            m_nodeAllocator.deallocate(m_pRoot, m_uCapacity);
            m_pRoot = NULL;
            m_uCapacity = 0;
            m_uSize = 0;
        }
    }

    // Helpers go here.

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::swap(node_type& _first, node_type& _second)
    {
        if(&_first != &_second)
        {
            // Notice:
            // We use memcpy 'cos its faster than memmove plus we are safe here irrespective
            // of the underlying types and its guaranteed that buffers don't overlapped.
            pair_type temp;
            GLARE_MEMCPY(&temp, &_first.m_data , sizeof(pair_type));
            GLARE_MEMCPY(&_first.m_data, &_second.m_data, sizeof(pair_type));
            GLARE_MEMCPY(&_second.m_data, &temp, sizeof(pair_type));
        }
    }
    
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::resize(size_type _nSize)
    {
        if(node_pointer nodePtr = m_nodeAllocator.allocate(_nSize))
        {
            // Index to start calling the contructor of the newly created node-types.
            unsigned beginIdx = 0; // Assume to call for all the objects.

            if(m_pRoot)
            {// free existing memory after copying the data, avoid destructor.
                GLARE_MEMCPY(nodePtr, m_pRoot, m_uCapacity*sizeof(node_type));
                m_nodeAllocator.deallocate(m_pRoot, m_uCapacity);

                // Its error to call c'tor for the objects already initialized, so skip them.
                beginIdx = m_uCapacity;
            }

            m_pRoot = nodePtr;
            m_uCapacity = _nSize;

            node_type node_default; // Used for default initialization.

            for (int i=beginIdx; i < _nSize; ++i) {
                // Use copy constructor to create nodes with the default setting.
                m_nodeAllocator.construct(&nodePtr[i], node_default);
            }
        }
        else{
            GLARE_ASSERT(false, "[CHeap<>][resize()] Allocation Failed!");
        }
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    typename CHeap<_KeyType, _ValType, _Pred, _Alloc>::index_type
             CHeap<_KeyType, _ValType, _Pred, _Alloc>::
                                        compareNodes(index_type _leftNodeIdx, index_type _rightNodeIdx)
    {
        bool bIsLeftValid = isNodeValid(_leftNodeIdx);
        bool bIsRightValid = isNodeValid(_rightNodeIdx);
        index_type ret_index = node_type::node_index_invalid;

        if (bIsLeftValid && bIsRightValid)
        {
            ret_index = ((m_binPredicate(getNode(_leftNodeIdx).m_data.first, getNode(_rightNodeIdx).m_data.first) ) ? _leftNodeIdx : _rightNodeIdx);
        }
        else if (bIsLeftValid)
        {
            ret_index = _leftNodeIdx;
        }
        else if (bIsRightValid)
        {
            ret_index = _rightNodeIdx;
        }

        // So we return the index which for which either the operation returned true or
        // is valid among the two nodes.
        return ret_index;
    }

    // Reheap Up/Down; 
    // Function: Restores the order property of the heap.
    // Postcondition: The order property is restored.

    // ------------------------------------------------------------------------------------------------
    // Precondition: The order property of the heaps may be violated only by the root node of the tree.
    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::reheapDown(index_type _nodeIndex)
    // ------------------------------------------------------------------------------------------------
    {
        // Compare the 2 nodes and assign 1 of them.
        index_type swapIndex = compareNodes(getLeftChildIndex(_nodeIndex), 
                                            getRightChildIndex(_nodeIndex));

        if (swapIndex != node_type::node_index_invalid)
        {// We have index to one of the valid nodes.
            swapIndex = compareNodes(_nodeIndex, swapIndex);
            if (swapIndex != _nodeIndex);
            {// Put the node at _nodeIndex at its right place.dZ
                swap(getNode(swapIndex), getNode(_nodeIndex));
                reheapDown(swapIndex);
            }
        }
        // else; // Means that _nodeIndex is at the right spot.
    }

    template<typename _KeyType, typename _ValType, typename _Pred,typename _Alloc>
    void CHeap<_KeyType, _ValType, _Pred, _Alloc>::reheapUp(index_type _nodeIndex)
    {
        if(_nodeIndex > 0)
        {
            index_type parentNodeIndex = getParentIndex(_nodeIndex);
            if (compareNodes(_nodeIndex, parentNodeIndex) == _nodeIndex)
            {
                swap(getNode(_nodeIndex), getNode(parentNodeIndex));
                reheapUp(parentNodeIndex);
            }
            // else; // _nodeIndex is at it's right position.
        }
    }

} // End of namespace.

#endif
