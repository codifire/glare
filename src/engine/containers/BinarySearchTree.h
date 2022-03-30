#ifndef BINARY_SEARCH_TREE_IMPL_H
#define BINARY_SEARCH_TREE_IMPL_H

#include "GlareCoreUtility.h"
#include "engine_common.h"
#include "containers.h"
#include "BSTNode.h"

/*
[TODO] Big fucking possible flaw in the container type. If type T is a pointer.
[TODO] Big fucking possible flaw in the container type. On removing a node with two children check if the destructor of correct node is called.
*/


namespace glare
{
    template<typename T, typename Alloc = default_allocator<T> >
    class CBinarySearchTree
    {
        typedef typename BasicNode<T>::node_type            node_type;
        typedef typename BasicNode<T>::node_pointer         node_pointer;
        typedef typename BasicNode<T>::const_node_pointer   const_node_pointer;

    public:
        typedef T                                           value_type;
        typedef value_type*                                 pointer;
        typedef const value_type*                           const_pointer;
        typedef value_type&                                 reference;
        typedef const value_type&                           const_reference;
        typedef std::size_t                                 size_type;
        typedef std::ptrdiff_t                              difference_type;

        typedef Alloc                                       allocator_type;

        typedef typename 
        Alloc::template rebind<node_type>::other            node_allocator_type;

        typedef void (*process_data_cb)(const_reference vt);

    public:
        CBinarySearchTree();
        ~CBinarySearchTree();

        CBinarySearchTree(const CBinarySearchTree& _originalTree);
        CBinarySearchTree& operator = (const CBinarySearchTree& _originalTree);

        bool empty() const     { return (size() == 0); }
        size_type size() const { return m_uSize; }

        void clear();
        void insert(const_reference);
        void remove(const_reference);
        bool get(reference) const;

        void setPreOrderTraversal() { m_traversalFunc = preorder; }
        void setPostOrderTraversal() { m_traversalFunc = postorder; }
        void setInOrderTraversal() { m_traversalFunc = inorder; }

        void traverse(process_data_cb pFunc) const { m_traversalFunc(m_pRoot, pFunc); }

    protected:
        // Memory allocation then initialzation and  destruction then deletion.
        node_pointer createNode(const_reference);
        void         destroyNode(node_pointer);

        void         cleanUp(node_pointer);
        size_type    count(node_pointer) const;

        /* DELETION.
         * Logical Predecessor of a node is the node whose key is closest in value to, but less than, the value of the key of the node to be deleted.
         * Logical Successor of a node is the node whose key is closest in value to, but greater than, the value of the key of the node to be deleted.
         * We use a method in which we don't delete the node but rather replace its data member with the data member from another node in the tree that 
           maintains the search property. we then delete this other node. 
         * We replace the data member of the node we wish to delete with the data member of its logical predecessor and then delete the node containing 
           the predecessor.
         * The predecessor is always found in a node with either zero or one child.
         */

        // This is what we use for now (just because I want to!).
        void findLogicalPredecessor(node_pointer, node_pointer&, node_pointer&) const;

        // This exists for completeness, as an options.
        void findLogicalSuccessor(node_pointer, node_pointer&, node_pointer&) const;

        void         remove(node_pointer& _tree, const_reference _obj);
        void         deleteNode(node_pointer& _tree);

        static void preorder(node_pointer pNode, process_data_cb pFunc);
        static void postorder(node_pointer pNode, process_data_cb pFunc);
        static void inorder(node_pointer pNode, process_data_cb pFunc);

        void copy(node_pointer& _copyRoot, const_node_pointer _originalRoot);
        void copyTree(const CBinarySearchTree& _originalRoot);

        // Fast Functions; Use these faster methods for performance improvements.
        void find(node_pointer _tree, const_reference _item,        // In
                  node_pointer& _nodePtr, node_pointer& _parentPtr  // Out
                  ) const;

        void insertNode(const_reference _item);

    private:

        node_pointer    m_pRoot;
        size_type       m_uSize;
        void (*m_traversalFunc)(node_pointer pNode, process_data_cb pFunc);

        // Alloc           m_allocator;
        node_allocator_type m_nodeAllocator;
    };

    template<typename T, typename Alloc>
    inline CBinarySearchTree<T, Alloc>::CBinarySearchTree() : m_pRoot(NULL)
                                                             ,m_uSize(0)
                                                             ,m_traversalFunc(preorder)
    {
    }

    template<typename T, typename Alloc>
    inline CBinarySearchTree<T, Alloc>::~CBinarySearchTree()
    {
        clear();
    }
    
    template<typename T, typename Alloc>
    inline CBinarySearchTree<T, Alloc>::CBinarySearchTree(const CBinarySearchTree& _originalTree): m_pRoot(NULL) 
                                                                                                 , m_uSize(0)
                                                                                                 , m_traversalFunc(_originalTree.m_traversalFunc)
    {
        copyTree(_originalTree);
    }
    
    template<typename T, typename Alloc>
    inline CBinarySearchTree<T, Alloc>& CBinarySearchTree<T, Alloc>::operator = (const CBinarySearchTree& _originalTree)
    {
        if (this != &_originalTree){
            clear(m_pRoot)
            copyTree(_originalTree);
        }
        return *this;
    }
    
    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::clear()
    {
        if(m_pRoot) 
        { 
            cleanUp(m_pRoot);
            m_uSize = 0;
            m_pRoot = NULL;
        }
    }

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::cleanUp(node_pointer _ptr)
    {
        if (_ptr->m_pLeft){
            cleanUp(_ptr->m_pLeft);
        }
        if (_ptr->m_pRight){
            cleanUp(_ptr->m_pRight);
        }
        destroyNode(_ptr);
    }

    template<typename T, typename Alloc>
    typename CBinarySearchTree<T, Alloc>::size_type CBinarySearchTree<T, Alloc>::count(node_pointer _ptr) const
    {
        size_type uCnt = 1;

        if (_ptr->m_pLeft){
            uCnt += count(_ptr->m_pLeft);
        }
        if (_ptr->m_pRight){
            uCnt += count(_ptr->m_pRight);
        }
        return uCnt;
    }

    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::insert(const_reference _obj)
    {
        insertNode(_obj);
    }

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::insertNode(const_reference _item)
    {
        node_pointer nodePtr, parentPtr;

        find(m_pRoot, _item, nodePtr, parentPtr);

        if (nodePtr == NULL) // If node was not found then insert it.
        {
            node_pointer newNode = createNode(_item);
            newNode->m_pLeft = NULL;
            newNode->m_pRight = NULL;

            if (parentPtr == NULL){
                m_pRoot = newNode;
            }
            else if (_item < parentPtr->m_data){
                parentPtr->m_pLeft = newNode;
            }
            else{
                parentPtr->m_pRight = newNode;
            }

            ++m_uSize;
        }
    }

    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::remove(const_reference _obj)
    {
        remove(m_pRoot, _obj);
    }

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::remove(node_pointer& _refRootPtr, const_reference _item)
    {
        node_pointer nodePtr, parentPtr;
        find(_refRootPtr, _item, nodePtr, parentPtr);

        if (nodePtr)
        {
            // Node is found!
            if (nodePtr == _refRootPtr){ // Equivalent of (parentPtr == NULL) but more general, in case _refRootPtr != m_pRoot.
                deleteNode(_refRootPtr);
            }
            else if(parentPtr->m_pLeft == nodePtr){  // Doing this to forward the real tree node pointer not the copy of the pointer variable.
                deleteNode(parentPtr->m_pLeft);
            }
            else{
                deleteNode(parentPtr->m_pRight);
            }
        }
    }

//------------------------------------------------------------------------------------
    /*
        Three cases:
        1: Delete node with zero child. Set the parent to point to NULL and delete the node.
        2: Delete node with one child. Set the parent to point to child of deleted node.
        3: Delete node with two children. Find a predecessor, copy it in the node and then delete the predecessor node.

        [Note][Advantage] We take advantage of the fact that the predecessor/successor, whatever, is found as the 
                          replacement for the node to be deleted has either 'zero or one child'.
    */

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::deleteNode(node_pointer& _refNodePtr)
//------------------------------------------------------------------------------------
    {
        if (_refNodePtr->m_pLeft == NULL){
            node_pointer temp = _refNodePtr;
            _refNodePtr = _refNodePtr->m_pRight;
            destroyNode(temp);
            --m_uSize;
        }
        else if(_refNodePtr->m_pRight == NULL){
            node_pointer temp = _refNodePtr;
            _refNodePtr = _refNodePtr->m_pLeft;
            destroyNode(temp);
            --m_uSize;
        }
        else{
            node_pointer pPredecessor, pParentPred;
            findLogicalPredecessor(_refNodePtr, pPredecessor, pParentPred);

            _refNodePtr->m_data = pPredecessor->m_data;

            // Delete the predecessor. This is where we take advantage through recursion of the above mentioned point.
            if (pParentPred->m_pLeft == pPredecessor){
                deleteNode(pParentPred->m_pLeft);
            }
            else{
                deleteNode(pParentPred->m_pRight);
            }
            
        }
    }

    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::findLogicalPredecessor(node_pointer _pNode, node_pointer& _outPredPtr, node_pointer& _outParentPtr) const
    {
        _outPredPtr = _pNode->m_pLeft,   // Subtree.
        _outParentPtr = _pNode;          // Parent of subtree.

        // Right most node in the subtree, with the largest value.
        while(_outPredPtr->m_pRight != NULL)
        {
            _outParentPtr = _outPredPtr;
            _outPredPtr = _outPredPtr->m_pRight;
        }
    }

    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::findLogicalSuccessor(node_pointer _pTree, node_pointer& _outPtr, node_pointer& _outParentPtr) const
    {
        _outPtr = _pNode->m_pRight,   // Subtree.
        _outParentPtr = _pNode;          // Parent of subtree.

        // Left most node of the subtree, with the smallest value.
        while(_outPtr->m_pLeft != NULL)
        {
            _outParentPtr = _outPtr;
            _outPtr = _outPtr->m_pLeft;
        }
    }

    template<typename T, typename Alloc>
    inline bool CBinarySearchTree<T, Alloc>::get(reference _obj) const
    {
        bool bResult = false;
        node_pointer nodePtr, parentPtr;

        find(m_pRoot, _obj, nodePtr, parentPtr);
        if(nodePtr)
        {
            bResult = true;
            _obj = ptr->m_data;
        }
        return bResult;
    }

    template<typename T, typename Alloc>
    inline typename CBinarySearchTree<T, Alloc>::node_pointer CBinarySearchTree<T, Alloc>::createNode(const_reference _obj)
    {
        node_pointer pNewNode = m_nodeAllocator.allocate(1);
        m_nodeAllocator.construct( pNewNode, _obj);
        return pNewNode;
    }

    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::destroyNode(node_pointer pNode)
    {
        m_nodeAllocator.destroy(pNode);
        m_nodeAllocator.deallocate(pNode, 1);
    }
    
    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::preorder(node_pointer pNode, process_data_cb pFunc)
    {
        pFunc(pNode->m_data);
        if (pNode->m_pLeft){
            preorder(pNode->m_pLeft, pFunc);
        }
        if (pNode->m_pRight){
            preorder(pNode->m_pRight, pFunc);
        }
    }

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::postorder(node_pointer pNode, process_data_cb pFunc)
    {
        if (pNode->m_pLeft){
            postorder(pNode->m_pLeft, pFunc);
        }
        if (pNode->m_pRight){
            postorder(pNode->m_pRight, pFunc);
        }
        pFunc(pNode->m_data);
    }

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::inorder(node_pointer pNode, process_data_cb pFunc)
    {
        if (pNode->m_pLeft){
            inorder(pNode->m_pLeft, pFunc);
        }

        pFunc(pNode->m_data);

        if (pNode->m_pRight){
            inorder(pNode->m_pRight, pFunc);
        }
    }

    template<typename T, typename Alloc>
    inline void CBinarySearchTree<T, Alloc>::copyTree(const CBinarySearchTree& _originalRoot)
    {
        copy(m_pRoot, _originalRoot->m_pRoot);
    }
    
    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::copy(node_pointer& _copy, const_node_pointer _originalTree)
    {
        if (_originalTree == NULL){
            _copy = NULL;
        }
        else
        {
            _copy = createNode(_originalTree->m_data);
            copy(_copy->m_pLeft, _originalTree->m_pLeft);
            copy(_copy->m_pRight, _originalTree->m_pRight);
        }
    }

    template<typename T, typename Alloc>
    void CBinarySearchTree<T, Alloc>::find(node_pointer _tree, const_reference _item, node_pointer& _nodePtr, node_pointer& _parentPtr) const
    {
        _nodePtr = _tree;
        _parentPtr = NULL;

        while(_nodePtr != NULL)
        {
            if (_item < _nodePtr->m_data)
            {
                _parentPtr = _nodePtr;
                _nodePtr = _nodePtr->m_pLeft;
            }
            else if (_item > _nodePtr->m_data)
            {
                _parentPtr = _nodePtr;
                _nodePtr = _nodePtr->m_pRight;
            }
            else{
                return; // Found it!
            }
        }
    }

} // End of namespace.

#endif
