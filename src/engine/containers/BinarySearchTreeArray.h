#ifndef _GLARE_BINARY_SEARCH_TREE_ARRAY_H_
#define _GLARE_BINARY_SEARCH_TREE_ARRAY_H_

#include "BTreeArrayNode.h"

// [TODO] Array Resizing: Resize to 2*capacity+1, something like creating a new level in the tree.

namespace glare
{
    template<typename _ValTy, typename _KeyTy, typename _Alloc = default_allocator<T> >
    class CBinarySearchTreeArray
    {
    public:
        typedef _KeyTy                  key_type;
        typedef _ValTy                  value_type;
        typedef value_type*             pointer;
        typedef const value_type*       const_pointer;
        typedef value_type&             reference;
        typedef const value_type&       const_reference;
        typedef std::size_t             size_type;
        typedef std::ptrdiff_t          difference_type;

        typedef GLARE_PAIR<key_type, value_type>            pair_type;

        typedef typename BasicNode<pair_type>::node_type    node_type;
        typedef typename node_type::index_type              index_type;
        typedef typename node_type::node_pointer            node_pointer;
        typedef typename node_type::const_node_pointer      const_node_pointer;

        typedef _Alloc                                      allocator_type;
        typedef typename _Alloc::template rebind<node_type> node_allocator_type;

        typedef void (*process_data_cb)(const_reference vt);

        CBinarySearchTreeArray();
        CBinarySearchTreeArray(size_type _uCapacity);
        ~CBinarySearchTreeArray();

        CBinarySearchTreeArray(const CBinarySearchTreeArray& _originalTree);
        CBinarySearchTreeArray& operator = (const CBinarySearchTreeArray& _originalTree);

        bool empty() const     { return (size() == 0); }
        size_type size() const { return m_uSize; }

        void clear();
        void insert(const key_type& _key, const_reference _data);
        void insert(const pair_type&);

        void remove(const key_type& _key);

        pointer get(const key_type& _key) const;
        bool get(const key_type& _key, reference _data);

        void setPreOrderTraversal() { m_traversalFunc = GTreeNode::preorder; }
        void setPostOrderTraversal() { m_traversalFunc = GTreeNode::postorder; }
        void setInOrderTraversal() { m_traversalFunc = GTreeNode::inorder; }

        void traverse(process_data_cb pFunc) const { m_traversalFunc(m_pRoot, pFunc); }

    protected:

        static void preorder(index_type _index, process_data_cb _pFunc);
        static void postorder(index_type _index, process_data_cb _pFunc);
        static void inorder(index_type _index, process_data_cb _pFunc);

        node_type& getNode(index_type _index) { return m_pRoot[_index]; }
        bool isIndexValid(index_type _index) { return _index < m_uCapacity; }
        bool isNodeValid(index_type _index) { return (isIndexValid(_index) && getNode(_index).isValid()); }

        index_type getLeftChildIndex(index_type _index) const { return(_index*2 + 1); }
        index_type getRightChildIndex(index_type _index) const { return(_index*2 + 2); }
        index_type getParentIndex(index_type _index) const { return static_cast<index_type>((_index-1)/2); }

        bool hasLeftChild(index_type _index) const { return isNodeValid( getLeftChildIndex(_index) ); }
        bool hasRightChild(index_type _index) const { return isNodeValid( getRightChildIndex(_index) ); }
        bool isLeafNode(index_type _index) const { return (!hasLeftChild(_index) && !hasRightChild(_index)); }

        index_type getPredecessor(index_type _index);

        void remove(index_type _treeIndex, const key_type& _key);
        void deleteNode(index_type _index);

        void copy(const CBinarySearchTreeArray& _originalTree);

        // If a valid node isn't found at the specified index then the nodes ideal position is returned in _nodePtr.
        bool find(index_type _treeIndex, const key_type& _key, index_type& _nodeIndex) const;

        node_pointer m_pRoot; // Also the base address of the array.
        size_type    m_uSize; // Allocated space.
        size_type    m_uCapacity;

        void (*m_traversalFunc)(node_pointer pNode, process_data_cb pFunc);

        node_allocator_type m_nodeAllocator;
    };
    
    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::CBinarySearchTreeArray() : m_pRoot(NULL)
                                                                             , m_uSize(0)
                                                                             , m_uCapacity(0)
                                                                             , m_traversalFunc(preorder)
    {

    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::CBinarySearchTreeArray(size_type _uCaps) : m_pRoot(NULL)
                                                                                             , m_uSize(0)
                                                                                             , m_uCapacity(0)
                                                                                             , m_traversalFunc(preorder)
    {
        if((m_pRoot = m_nodeAllocator.allocate(_uCaps)))
        {
            m_uCapacity = _uCaps;

            for (int i=0; i<_uCaps; ++i)
            {
                m_nodeAllocator.construct(&m_pRoot[i], node_index_invalid);
            }
        }
    }
    
    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::~CBinarySearchTreeArray()
    {
        if (m_pRoot)
        {
            clear();

            for (int i=0; i<m_uCapacity; ++i)
            {
                m_nodeAllocator.destroy(&m_pRoot[i]);
            }
            m_nodeAllocator.deallocate(m_pRoot, m_uCapacity);
        }
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::CBinarySearchTreeArray(const CBinarySearchTreeArray& _originalTree): m_pRoot(NULL)
                                                                                                                       , m_uSize(0)
                                                                                                                       , m_uCapacity(0)
                                                                                                                       , m_traversalFunc(preorder)
    {
        copy(_originalTree);
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>& 
        CBinarySearchTreeArray<_ValTy, _KeyTy, Alloc>::operator = (const CBinarySearchTreeArray& _originalTree)
    {
        if (this != &_originalTree){
            clear(m_pRoot)
            copy(_originalTree);
        }
        return *this;
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    bool CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::find(index_type _treeIndex, const key_type& _key, index_type& _nodeIndex) const
    {
        _nodeIndex = _treeIndex;

        while(isNodeValid(_nodeIndex))
        {
            if (_key < getNode(_nodeIndex).m_data.first) // data is a pair<key, val>.
            {
                _nodeIndex = getLeftChildIndex(_nodeIndex);
            }
            else if (_key > getNode(_nodeIndex).m_data.first)
            {
                _nodeIndex = getRightChildIndex(_nodeIndex);
            }
            else{ // Found it!
                return true; 
            }
        }
        return false;
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::insert(const key_type& _key, const_reference _data)
    {
        index_type nodeIdx = node_type::node_index_invalid;

        // 0 is the root node index.
        if(!find(0, _key, nodeIdx))
        { // Not found then insert.
            getNode(nodeIdx).m_index = nodeIdx;
            getNode(nodeIdx).m_data.first = _key;
            getNode(nodeIdx).m_data.second = _data;
            ++m_uSize;
        }
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::insert(const pair_type& pairRef)
    {
        insert(pairRef.first, pairRef.second);
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    pointer CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::get(const key_type& _key) const
    {
        // zero is root node index
        index_type nodeIdx = node_type::node_index_invalid;
        pointer nodePtr = NULL;

        if(find(0, _key, nodeIdx)){
            nodePtr = &(getNode(nodeIdx).m_data.second);
        }
        return nodePtr;
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    bool CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::get(const key_type& _key, reference _data) const
    {
        // zero is root node index
        index_type nodeIdx = node_type::node_index_invalid;
        if(find(0, _key, nodeIdx)){
            _data = getNode(nodeIdx).m_data.second;
            return true;
        }
        return false;
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::remove(const key_type& _key)
    {
        remove(0, _key);
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    inline void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::remove(index_type _treeIndex, const key_type& _key)
    {
        index_type nodeIndex;
        if (find(_treeIndex, _key, nodeIndex /*[out]*/))
        {
            deleteNode(nodeIndex);
        }
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::deleteNode(index_type _index)
    {
        //[NOTE] We just call destructor of the node and doesn't deallocate the memory 'cos it was allocated as an array.
        m_nodeAllocator.destroy(&getNode(_index)); // So that the contained object's destructor is called.

        if (isLeafNode(_index))
        {// Terminate recursion at leaf, coz then we don't have anything to replace the node with.
            getNode(_index).invalidate();

            // This seems to be an odd place to reduce the size but this code is executed once per deletion process and
            // we just want to decrement once per sequence of recursive calls for deletion of an element.
            --m_uSize;
        }
        else
        {// Has either one or two children!
            // We always replace the node to be deleted with the predecessor (found in the left subtree, obviously).
            // In case we don't have a left subtree then we definitely have a right subtree as this node is not a leaf node.
            node_type predecessorIndex = getRightChildIndex(_index);

            if (hasLeftChild(_index))
            {// Current has left child!
                predecessorIndex = getPredecessor(getLeftChildIndex(_index));
            }

            // Copy the predecessor then delete it.
            getNode(_index) = getNode(predecessorIndex);
            deleteNode(predecessorIndex); // Recurse.
        }
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    inline index_type CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::getPredecessor(index_type _index)
    {// Predecessor is the right most node of the tree rooted at _index.
        while(hasRightChild(_index)) {
            _index = getRightChildIndex(_index);
        }
        return _index;
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    inline void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::copy(const CBinarySearchTreeArray& _originalTree)
    {
        if (_originalTree.m_pRoot)
        {
            m_pRoot = m_nodeAllocator.allocate(_originalTree.m_uCapacity);
            m_uSize = _originalTree.m_uSize;
            m_uCapacity = _originalTree.m_uCapacity;

            // We run till capacity 'cos we didn't run constructor for any of the newly allocated node elements.
            for (int i=0; i<m_uCapacity; ++i)
            {
                m_pRoot[i] = _originalTree.m_pRoot[i];
            }
        }
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    inline void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::clear()
    {
        // Invoke destructor of each of the contained element then free memory.
        for (int i=0; i<m_uCapacity; ++i)
        {
            m_nodeAllocator.destroy(&m_pRoot[i]);
        }
        m_nodeAllocator.deallocate(m_pRoot, m_uCapacity);
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::preorder(index_type _index, process_data_cb _pFunc)
    {
        pFunc(getNode(_index).m_data.second);
        if (hasLeftChild(_index)){
            preorder(getLeftChildIndex(_index), pFunc);
        }
        if (hasRightChild(_index)){
            preorder(getRightChildIndex(_index), pFunc);
        }
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::postorder(index_type _index, process_data_cb _pFunc)
    {
        if (hasLeftChild(_index)){
            postorder(getLeftChildIndex(_index), pFunc);
        }
        if (hasRightChild(_index)){
            postorder(getRightChildIndex(_index), pFunc);
        }
        pFunc(getNode(_index).m_data.second);
    }

    template<typename _ValTy, typename _KeyTy, typename _Alloc>
    void CBinarySearchTreeArray<_ValTy, _KeyTy, _Alloc>::inorder(index_type _index, process_data_cb _pFunc)
    {
        if (hasLeftChild(_index)){
            inorder(getLeftChildIndex(_index), pFunc);
        }

        pFunc(getNode(_index).m_data.second);

        if (hasRightChild(_index)){
            inorder(getRightChildIndex(_index), pFunc);
        }
    }


} // End of namespace
#endif
