#ifndef GLARE_AVL_TREE_H
#define GLARE_AVL_TREE_H

#include "GlareCoreUtility.h"
#include "memory\allocators.h"


// TODO An optimization that will choose predecessor or successor based on the balance of the node to be deleted.
// For eg: if the node to be deleted is right higher then choose a successor so that the tree needs less balancing and is more balanced.

// TODO Eliminate Copy Assignment during deletion when a predecessor is found.

namespace glare
{
    template<typename keyType, typename ValueType>
    class AvlTreeNode
    {
    public:
        typedef AvlTreeNode<keyType, ValueType>                node_type;
        typedef node_type*                                     node_pointer;
        typedef const node_type*                               const_node_pointer;
        typedef GLARE_PAIR<keyType, ValueType>                 pair_type;
        
        enum BalanceFactor
        {
            RightHigher = -1,
            EqualHeight = 0,
            LeftHigher  = 1
        };

        struct SerializableType
        {
            pair_type     m_pair;
            BalanceFactor m_balanceFactor;

            SerializableType(const node_type& _node): m_pair(_node.m_pair)
                                                    , m_balanceFactor(_node.m_balanceFactor)
            {}
        };

        typedef SerializableType                              serializable_type;
        typedef GLARE_VECTOR<serializable_type>               serializable_list;

        explicit AvlTreeNode(const pair_type& _pairRef): m_left(nullptr)
                                              , m_right(nullptr)
                                              , m_pair(_pairRef)
                                              , m_balanceFactor(EqualHeight)
        
        {}

        AvlTreeNode(const AvlTreeNode& _nodeRef): m_left(nullptr)
                                                , m_right(nullptr)
                                                , m_pair(_nodeRef.m_pair)
                                                , m_balanceFactor(_nodeRef.m_balanceFactor)
        {}

        explicit AvlTreeNode(const serializable_type& _obj): m_left(nullptr)
                                                  , m_right(nullptr)
                                                  , m_pair(_obj.m_pair)
                                                  , m_balanceFactor(_obj.m_balanceFactor)
        {}

        node_type& operator = (const node_type& _other)
        {
            if(&_other != this)
            {
                m_left =  nullptr;
                m_right = nullptr;
                m_pair = _other.m_pair;
                m_balanceFactor = _other.m_balanceFactor;
            }
            return *this;
        }

        const keyType& key() const { return m_pair.first; }
        const ValueType& value() const { return m_pair.second; }
        ValueType& value() { return m_pair.second; }
        BalanceFactor balanceFactor() const { return m_balanceFactor; }
        void setBalanceFactor(BalanceFactor _f) { m_balanceFactor = _f; }
        void copyDataOnly(const node_type& _other) { m_pair = _other.m_pair; }

    public:
        node_pointer  m_left;
        node_pointer  m_right;

    private:
        pair_type     m_pair;
        BalanceFactor m_balanceFactor;
    };
    
    //-------------------------------------------------------------------------------------------------------------------------------

    template<typename _KeyType, typename _ValType, typename _Pred = less<_KeyType>, typename _Alloc = default_allocator<_ValType> >
    class AvlTree
    {
        typedef typename AvlTreeNode<_KeyType, _ValType>        node_type;
        typedef typename node_type::node_pointer                node_pointer;
        typedef typename node_type::const_node_pointer          const_node_pointer;

        typedef typename 
        _Alloc::template rebind<node_type>::other               node_allocator_type;

    public:
        typedef _KeyType                                        key_type;
        typedef _Pred                                           key_compare; // binary predicate.

        typedef _ValType                                        value_type;
        typedef value_type*                                     pointer;
        typedef const value_type*                               const_pointer;
        typedef value_type&                                     reference;
        typedef const value_type&                               const_reference;
        typedef std::size_t                                     size_type;
        typedef std::ptrdiff_t                                  difference_type;

        typedef typename node_type::pair_type                   pair_type;
        typedef _Alloc                                          allocator_type;

        typedef typename node_type::serializable_list           serializable_list;
        typedef typename node_type::serializable_type           serializable_type;

        typedef void (*process_data_cb)(const key_type& _key, const_reference _data);

        //---------------------------------------------------------------------------
        
        AvlTree();
        ~AvlTree();

        AvlTree(const AvlTree&);
        AvlTree& operator = (const AvlTree&);
    
        bool insert(const pair_type& _pair);
        bool insert(const key_type& _key, const_reference _value);

        void remove(const key_type& _key);

        bool find(const key_type& _key, value_type& _val) const;
        pointer find(const key_type& _key);
        const_pointer find(const key_type& _key) const;

        void clear();

        size_t size() { return m_size; }

        void traverse(process_data_cb _cb);
        void setPreOrderTraversal()  { m_traversalFunc = preorder; }
        void setPostOrderTraversal() { m_traversalFunc = postorder; }
        void setInOrderTraversal()   { m_traversalFunc = inorder; }

        void serializeList(serializable_list& _list, bool _read);

    private:
        void rotate_left(node_pointer& _subRoot);
        void rotate_right(node_pointer& _subRoot);
        void balance_left(node_pointer& _subRoot);
        void balance_right(node_pointer& _subRoot);

        const node_pointer avl_find(const key_type& _key, const node_pointer& _subRoot) const;
        void avl_insert(const pair_type& _pair, node_pointer& _subRoot, node_pointer& _newNode, bool& _taller);
        void avl_delete(const key_type& _key, node_pointer& _subRoot, bool& _shorter);
        bool delete_node(node_pointer& _refNodePtr);
        
        static void preorder (node_pointer _node, process_data_cb _funcCb);
        static void postorder(node_pointer _node, process_data_cb _funcCb);
        static void inorder  (node_pointer _node, process_data_cb _funcCb);

        void internal_clean(node_pointer _subRoot);

        void copy(node_pointer& _copyRoot, const_node_pointer _originalRoot);
        void copyTree(const AvlTree& _originalRoot);

        // Tree Serialization
        void avl_serialize_insert(node_pointer& _node, serializable_type& _proxy);
        void avl_serialize_to_list(node_pointer _node, serializable_list& _list);
        void avl_serialize_from_list(node_pointer& _node, serializable_list& _list);

        node_pointer find_logical_predecessor(node_pointer _node) const;
        node_pointer find_logical_successor(node_pointer _node) const;

    private:
        size_type    m_size;
        node_pointer m_root;
        key_compare  m_binPredicate;
        void (*m_traversalFunc)(node_pointer _node, process_data_cb _funcCb);
        node_allocator_type m_nodeAllocator;
    }; // avlTree

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::AvlTree(): m_size(0)
                                                         , m_root(nullptr) 
                                                         , m_traversalFunc(preorder)
    {}

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::~AvlTree()
    {
        clear();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::AvlTree(const AvlTree& _avl): m_size(0)
                                                                            , m_root(nullptr)
                                                                            , m_binPredicate(_avl.m_binPredicate)
                                                                            , m_traversalFunc(_avl.m_traversalFunc)
    {
        copyTree(_avl);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>& 
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::operator = (const AvlTree& _avl)
    {
        if (this != &_avl)
        {
           clear();
           copyTree(_avl);
        }
        return *this;
    }
    //--------------------------------------------------------------------------------------------------------------

    
    //--------------------------------------------------------------------------------------------------------------
    // ROTATIONS AND BALANCING
    //--------------------------------------------------------------------------------------------------------------
    /*
                y                               x
               / \     Right Rotation          /  \
              x   T3   – – – – – – – >        T1   y
             / \       < - - - - - - -            / \
            T1  T2     Left Rotation            T2  T3
    */
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::rotate_left(node_pointer& _subRoot)
    //--------------------------------------------------------------------------------------------------------------
    {
        GLARE_ASSERT(_subRoot && _subRoot->m_right, "[AVL][Logic Fail] This impossible situation shouldn't have arised.");

        node_pointer rightTree = _subRoot->m_right;
        _subRoot->m_right = rightTree->m_left;
        rightTree->m_left = _subRoot;
        _subRoot = rightTree;
    }

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::rotate_right(node_pointer& _subRoot)
    //--------------------------------------------------------------------------------------------------------------
    {
        GLARE_ASSERT(_subRoot && _subRoot->m_left, "[AVL][Logic Fail] This impossible situation shouldn't have arised.");

        node_pointer leftTree = _subRoot->m_left;
        _subRoot->m_left = leftTree->m_right;
        leftTree->m_right = _subRoot;
        _subRoot = leftTree;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Pre: _subRoot points to a subtree of an AVL tree that is doubly unbalanced on the right.
    // Post: The AVL properties have been restored to the subtree.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::balance_right(node_pointer& _subRoot)
    //--------------------------------------------------------------------------------------------------------------
    {
        node_pointer& rightSubRoot(_subRoot->m_right);
        switch(rightSubRoot->balanceFactor())
        {
        case node_type::EqualHeight: // Possible only in deletion. It's impossible to have met this case while insertion.
            {
                _subRoot->setBalanceFactor(node_type::RightHigher);
                rightSubRoot->setBalanceFactor(node_type::LeftHigher);
                rotate_left(_subRoot); // After rotation, _subRoot becomes left child of rightSubRoot.
                break;   
            }
        case node_type::RightHigher: // We need single rotation to balance _subRoot.
            {
                _subRoot->setBalanceFactor(node_type::EqualHeight);
                rightSubRoot->setBalanceFactor(node_type::EqualHeight);
                rotate_left(_subRoot);
                break;
            }
        case node_type::LeftHigher: // We need RL double rotation to balance _subRoot.
            {
                node_pointer leftSubTree = rightSubRoot->m_left;
                switch(leftSubTree->balanceFactor())
                {
                case node_type::EqualHeight: // Possible only in deletion. It's impossible to have met this case while insertion.
                    {
                        _subRoot->setBalanceFactor(node_type::EqualHeight);
                        rightSubRoot->setBalanceFactor(node_type::EqualHeight);
                        break;
                    }
                case node_type::RightHigher:
                    {
                        _subRoot->setBalanceFactor(node_type::LeftHigher);
                        rightSubRoot->setBalanceFactor(node_type::EqualHeight);
                        break;
                    }
                case node_type::LeftHigher:
                    {
                        _subRoot->setBalanceFactor(node_type::EqualHeight);
                        rightSubRoot->setBalanceFactor(node_type::RightHigher);
                        break;
                    }
                }
                leftSubTree->setBalanceFactor(node_type::EqualHeight);
                rotate_right(rightSubRoot); // A right rotation reduces this case to a simple single rotation problem now.
                rotate_left(_subRoot); // A single left rotation balances _subRoot now.
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------------------
    // Its kind of similar to what we did in the balance_right().
    // Pre: _subRoot points to a subtree of an AVL tree that is doubly unbalanced on the left.
    // Post: The AVL properties have been restored to the subtree.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::balance_left(node_pointer& _subRoot)
    //--------------------------------------------------------------------------------------------------------------
    {
        node_pointer& leftSubRoot(_subRoot->m_left);
        switch (leftSubRoot->balanceFactor())
        {
        case node_type::EqualHeight: // Possible only in deletion. It's impossible to have met this case while insertion.
            {
                _subRoot->setBalanceFactor(node_type::LeftHigher);
                leftSubRoot->setBalanceFactor(node_type::RightHigher);
                rotate_right(_subRoot); // Perform Rotation on the node that became unbalanced during deletion.
                break;
            }
        case node_type::LeftHigher: // This is the simplest case, here we only need a single right rotation.
            {
                // The only nodes that are affected by rotation are _subroot (being balanced) and its left child (due to right rotation).
                // Rest remains as-is 'cos of the relative heights between the nodes don't change.
                _subRoot->setBalanceFactor(node_type::EqualHeight);
                leftSubRoot->setBalanceFactor(node_type::EqualHeight);
                rotate_right(_subRoot); // Perform Rotation on the unbalanced node.
                break;
            }
        case node_type::RightHigher: // Much complicated case, here we need a double LR rotations.
            {
                // The only nodes that are affected by rotation are _subroot (being balanced) and leftSubRoot because it is the left child
                // whose height increased after all. The balance factors of _subRoot & leftSubRoot depends upon the balance factor of the 
                // rightSubTree of leftSubRoot (i.e. leftSubRoot->m_right). One can verify that diagramatically. 
                // rightSubTree is always EqualHeight after the LR rotations as the matter of fact.
                node_pointer rightSubTree(leftSubRoot->m_right);
                switch (rightSubTree->balanceFactor())
                {
                case node_type::LeftHigher:
                    {
                        _subRoot->setBalanceFactor(node_type::RightHigher);
                        leftSubRoot->setBalanceFactor(node_type::EqualHeight);
                        break;
                    }
                case node_type::RightHigher:
                    {
                        _subRoot->setBalanceFactor(node_type::EqualHeight);
                        leftSubRoot->setBalanceFactor(node_type::LeftHigher);
                        break;
                    }
                case node_type::EqualHeight: // Possible only in deletion. It's impossible to have met this case while insertion.
                    {
                        // This case is only possible when a node had been deleted from the right subtree of _subRoot causing 
                        // _subRoot to be "double left higher".
                        _subRoot->setBalanceFactor(node_type::EqualHeight);
                        leftSubRoot->setBalanceFactor(node_type::EqualHeight);
                    }
                }
                rightSubTree->setBalanceFactor(node_type::EqualHeight); // Always be equal height, fact!
                rotate_left(leftSubRoot); // A left rotation reduces this big problem to a simple single rotation problem now.
                rotate_right(_subRoot); // A single right rotation thus restores the height, effectively, the AVL property. 
                break;
            }
        }
    }
    //--------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline bool AvlTree<_KeyType, _ValType, _Pred, _Alloc>::insert(const key_type& _key, const_reference _value)
    //--------------------------------------------------------------------------------------------------------------
    {
        const pair_type _pair(_key, _value);
        return insert(_pair);
    }

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline bool AvlTree<_KeyType, _ValType, _Pred, _Alloc>::insert(const pair_type& _pair)
    //--------------------------------------------------------------------------------------------------------------
    {
        bool taller = false;
        node_pointer newNode = nullptr;
        avl_insert(_pair, m_root, newNode, taller);
        return newNode ? true : false;
    }

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::avl_insert(const pair_type& _pair, 
        node_pointer& _subRoot, 
        node_pointer& _newNode, bool& _taller)
    //--------------------------------------------------------------------------------------------------------------
    {
        if (_subRoot == nullptr)
        {
            if (node_pointer nodePtr = m_nodeAllocator.allocate(1))
            {
                m_nodeAllocator.construct(nodePtr, _pair);
                _subRoot = nodePtr;
                _newNode = nodePtr;
                _taller = true;
                ++m_size;
            }
            else
            {
                _newNode = nullptr;
                _taller = false;
            }
        }
        else if(_subRoot->key() == _pair.first)
        {
            _newNode = nullptr; // Duplicate key not allowed.
            _taller = false;
        }
        else if(m_binPredicate(_pair.first, _subRoot->key()))
        {
            // Go Left.
            avl_insert(_pair, _subRoot->m_left, _newNode, _taller);
            if (_taller)
            {
                // A node is inserted in the left subtree of _subRoot, it is possible that the height of _subRoot has changed.
                switch(_subRoot->balanceFactor())
                {
                case node_type::LeftHigher: // _subRoot is already leftHigher new insertion causes it to be doubly left higher.
                    // An AVL tree needs at most 1 balance operation to preserve the shape property during insertion.
                    balance_left(_subRoot);
                    _taller = false;
                    break;
                case node_type::EqualHeight: // Height of _subRoot has increased, this node becomes left heavy and its still balanced.
                    // Let the balance checking continue till root or till _subRoot's parent or ancestor balance changes to EqualHeight.
                    _subRoot->setBalanceFactor(node_type::LeftHigher);
                    break;
                case node_type::RightHigher: 
                    // Height of the node has not increased, no need for balancing.
                    _subRoot->setBalanceFactor(node_type::EqualHeight);
                    _taller = false;
                    break;
                }
            }
        }
        else
        {
            // Go Right.
            avl_insert(_pair, _subRoot->m_right, _newNode, _taller);
            if (_taller)
            {
                // A node is inserted in the right subtree of _subRoot, it is possible that the height of _subRoot has changed.
                switch(_subRoot->balanceFactor())
                {
                case node_type::LeftHigher:
                    _subRoot->setBalanceFactor(node_type::EqualHeight);
                    _taller = false;
                    break;
                case node_type::EqualHeight: // Height of _subRoot has increased, this node becomes right heavy and its still balanced.
                    // Let the balance checking continue till root or till _subRoot's parent or ancestor balance changes to EqualHeight.
                    _subRoot->setBalanceFactor(node_type::RightHigher);
                    break;
                case node_type::RightHigher: // _subRoot is already rightHigher new insertion causes it to be doubly right higher.
                    // An AVL tree needs at most 1 balance operation to preserve the shape property during insertion.
                    balance_right(_subRoot);
                    _taller = false;
                    break;
                }
            }
        }
    }
    //--------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::remove(const key_type& _key)
    //--------------------------------------------------------------------------------------------------------------
    {
        bool shorter = false;
        avl_delete(_key, m_root, shorter);
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline typename AvlTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::find_logical_predecessor(node_pointer _node) const
    //--------------------------------------------------------------------------------------------------------------
    {
        // Key with the highest value, or the right most node, in the left subtree.
        node_pointer predecessor = _node->m_left;

        // Right most node of the subtree, with the largest value.
        while(predecessor->m_right != nullptr)
        {
            predecessor = predecessor->m_right;
        }

        return predecessor;
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline typename AvlTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::find_logical_successor(node_pointer _node) const
    //--------------------------------------------------------------------------------------------------------------
    {
        // Key with the lowest value, or the left most node, in the right subtree.
        node_pointer successor = _node->m_right;

        // Left most node of the subtree, with the smallest value.
        while(successor->m_left != nullptr)
        {
            successor = successor->m_left;
        }

        return successor;
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    bool AvlTree<_KeyType, _ValType, _Pred, _Alloc>::delete_node(node_pointer& _refNodePtr)
    //------------------------------------------------------------------------------------
    {
        // We only delete if _refNodePtr is a node with one or zero child.
        if (_refNodePtr->m_left == nullptr || _refNodePtr->m_right == nullptr)
        {
            node_pointer temp = _refNodePtr;
            _refNodePtr = (temp->m_left == nullptr) ? temp->m_right : temp->m_left;

            m_nodeAllocator.destroy(temp);
            m_nodeAllocator.deallocate(temp, 1);

            --m_size;
            return true;
        }

        // This means we want the caller to find a predecessor/successor and delete that.
        return false;
    }
    //--------------------------------------------------------------------------------------------------------------
    /*
        Three cases:
        (1) Delete node with zero child. Set the parent to point to NULL and delete the node.
        (2) Delete node with one child. Set the parent to point to child of deleted node.
        (3) Delete node with two children. Find a predecessor, copy it in the node and then delete the predecessor node.
        (4) When actually deleting a node, notice, we don't set the balance of the child which will then be linked to 
            the parent of the node to be deleted. Because, predecessor or successor always have one or no child at all.
            
        [Note][Advantage] We take advantage of the fact that the predecessor/successor, whatever, is found as the 
                          replacement for the node to be deleted has either 'zero or one child'. In case of 1 child, 
                          that child will have a balance factor "EqualHeight" and it must remain same after deletion.


       [CASE 1] When shorter subtree was shortened and 
       [Case 1.1]: root of taller subtree is Equal_Higher
                        
           y (<-2x left higher)            x  (<- Right Higher)
          /       Right Rotation          /  \
         x        – – – – – – – >        T1   y (<- Left Higher)
        / \       < - - - - - - -            /
       T1  T2     Left Rotation            T2
    */
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::avl_delete(const key_type& _key, node_pointer& _subRoot, bool& _shorter)
    //--------------------------------------------------------------------------------------------------------------
    {
        enum Direction { EDir_LeftBranch, EDir_RightBranch, EDir_None };
        Direction direction = EDir_None;

        if (_subRoot == nullptr)
        {
            _shorter = false; // Not Found!
        }
        else if (_key == _subRoot->key())
        {   
            if (delete_node(_subRoot))
            {
                // Because, predecessor or successor always have one or no child at all. 
                // That child will have a balance factor "EqualHeight" and it must remain same after deletion.
                _shorter = true; // We actually deleted a node, height of the tree might change.
            }
            else
            {   // _subRoot has two or more children.
                // We need to reduce the problem to a case where _subRoot has one or zero child.
                node_pointer nodePtr = find_logical_predecessor(_subRoot);
                _subRoot->copyDataOnly(*nodePtr); // We copy only key/value data.
                avl_delete(nodePtr->key(), _subRoot->m_left, _shorter); // Will find and delete the predecessor.
                if (_shorter)
                {
                    // Tree might have been properly balanced by now, if not then _shorter == true and 
                    // then only we shall attempt to do it. Because _shorter == true means height has changed.
                    direction = EDir_LeftBranch;
                }
            }
        }
        else if (m_binPredicate(_key, _subRoot->key()))
        {
            avl_delete(_key, _subRoot->m_left, _shorter);
            if (_shorter)
            {
                direction = EDir_LeftBranch;
            }
        }
        else
        {
            avl_delete(_key, _subRoot->m_right, _shorter);
            if (_shorter)
            {
                direction = EDir_RightBranch;
            }
        }

        // Process the result commonly for actual deletion and recursive calls.
        switch (direction)
        {
        case EDir_LeftBranch: // Deletion happened in the left subtree of _subRoot.
            {
                switch (_subRoot->balanceFactor())
                {
                case node_type::LeftHigher:
                    _subRoot->setBalanceFactor(node_type::EqualHeight); // Height of _subRoot decreased!
                    break;
                case node_type::EqualHeight:
                    _subRoot->setBalanceFactor(node_type::RightHigher);
                    _shorter = false; // Height of _subRoot didn't change at all.
                    break;
                case node_type::RightHigher: // This node violated AVL property, double right higher.
                    if (_subRoot->m_right->balanceFactor() == node_type::EqualHeight)
                    {
                        // Could've passed _shorter to balance_right but it is used for both insertion and deletion.
                        _shorter = false; // The height of _subRoot will never change even after rotation.
                    }
                    balance_right(_subRoot);
                    break;
                }
            }
            break;
        case EDir_RightBranch: // Deletion happened in the right subtree of _subRoot.
            {
                switch (_subRoot->balanceFactor())
                {
                case node_type::LeftHigher: // This node violated AVL property, double left higher.
                    if (_subRoot->m_left->balanceFactor() == node_type::EqualHeight)
                    {
                        _shorter = false; // The height of _subRoot will never change even after rotation.
                    }
                    balance_left(_subRoot);
                    break;
                case node_type::EqualHeight:
                    _subRoot->setBalanceFactor(node_type::LeftHigher);
                    _shorter = false; // Height of _subRoot didn't change at all.
                    break;
                case node_type::RightHigher:
                    _subRoot->setBalanceFactor(node_type::EqualHeight); // Height of _subRoot decreased!
                    break;
                }
            }
            break;
        }
    }
    //--------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    bool AvlTree<_KeyType, _ValType, _Pred, _Alloc>::find(const key_type& _key, value_type& _pVal) const
    {
        if (pointer ptr = find(_key))
        {
            _pVal = *ptr;
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename AvlTree<_KeyType, _ValType, _Pred, _Alloc>::pointer 
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::find(const key_type& _key)
    {
        return &(avl_find(_key, m_root)->value());
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename AvlTree<_KeyType, _ValType, _Pred, _Alloc>::const_pointer 
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::find(const key_type& _key) const
    {
        return &(avl_find(_key, m_root)->value());
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    const typename AvlTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
    AvlTree<_KeyType, _ValType, _Pred, _Alloc>::avl_find(const key_type& _key, const node_pointer& _subRoot) const
    //--------------------------------------------------------------------------------------------------------------
    {
        const node_pointer nodePtr = _subRoot;
        while(nodePtr)
        {
            if (_key == nodePtr->key())
            {
                break;
            }
            else if (_key < nodePtr->key())
            {
                nodePtr = nodePtr->m_left;
            }
            else // (_key > nodePtr->key())
            {
                nodePtr = nodePtr->m_right;
            }
        }
        return nodePtr;
    }
    //--------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::clear()
    //--------------------------------------------------------------------------------------------------------------
    {
        if(m_root) 
        { 
            internal_clean(m_root);
            m_root = nullptr;
            m_size = 0;
        }
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::internal_clean(node_pointer _subroot)
    {
        if (_subroot->m_left) {
            internal_clean(_subroot->m_left);
        }
        if (_subroot->m_right) {
            internal_clean(_subroot->m_right);
        }

        // Notice: PostOrder style destruction.
        m_nodeAllocator.destroy(_subroot);
        m_nodeAllocator.deallocate(_subroot, 1);
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::copyTree(const AvlTree& _originalRoot)
    //--------------------------------------------------------------------------------------------------------------
    {
        m_binPredicate = _originalRoot.m_binPredicate;
        m_traversalFunc = _originalRoot.m_traversalFunc;
        copy(m_root, _originalRoot.m_root);
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::copy(node_pointer& _copyRoot, const_node_pointer _originalRoot)
    //--------------------------------------------------------------------------------------------------------------
    {
        if (_originalRoot == nullptr)
        {
            _copyRoot = nullptr;
        }
        else
        {
            // Notice: Preorder fashion, create and copy current.
            _copyRoot = m_nodeAllocator.allocate(1);
            m_nodeAllocator.construct(_copyRoot, *_originalRoot);
            ++m_size;
            
            copy(_copyRoot->m_left, _originalRoot->m_left);
            copy(_copyRoot->m_right, _originalRoot->m_right);
        }
    }
    //--------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::preorder(node_pointer _node, process_data_cb _funcCb)
    //--------------------------------------------------------------------------------------------------------------
    {
        _funcCb(_node->key(), _node->value());
        if (_node->m_left) {
            preorder(_node->m_left, _funcCb);
        }
        if (_node->m_right) {
            preorder(_node->m_right, _funcCb);
        }
    }        
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::postorder(node_pointer _node, process_data_cb _funcCb)
    //--------------------------------------------------------------------------------------------------------------
    {
        if (_node->m_left) {
            postorder(_node->m_left, _funcCb);
        }
        if (_node->m_right) {
            postorder(_node->m_right, _funcCb);
        }
        _funcCb(_node->key(), _node->value());
    }    
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::inorder(node_pointer _node, process_data_cb _funcCb)
    //--------------------------------------------------------------------------------------------------------------
    {
        if (_node->m_left) {
            inorder(_node->m_left, _funcCb);
        }

        _funcCb(_node->key(), _node->value());

        if (_node->m_right) {
            inorder(_node->m_right, _funcCb);
        }
    }

    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::traverse(process_data_cb _cb)
    //--------------------------------------------------------------------------------------------------------------
    {
        m_traversalFunc(m_root, _cb);
    }

    //--------------------------------------------------------------------------------------------------------------
    // TREE SERIALIZATION
    //--------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::serializeList(serializable_list& _list, bool _read)
    //--------------------------------------------------------------------------------------------------------------
    {
        if (_read)
        {
            avl_serialize_to_list(m_root, _list);
        }
        else
        {
            avl_serialize_from_list(m_root, _list);
        }
    }    
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::avl_serialize_to_list(node_pointer _node, serializable_list& _list)
    //--------------------------------------------------------------------------------------------------------------
    {
        _list.push_back(serializable_type(*_node));

        if (_node->m_left) {
            avl_serialize_to_list(_node->m_left, _list);
        }
        if (_node->m_right) {
            avl_serialize_to_list(_node->m_right, _list);
        }
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::avl_serialize_insert(node_pointer& _subRoot, serializable_type& _obj)
    //--------------------------------------------------------------------------------------------------------------
    {
        if (_subRoot == nullptr)
        {
            if (node_pointer nodePtr = m_nodeAllocator.allocate(1))
            {
                m_nodeAllocator.construct(nodePtr, _obj);
                _subRoot = nodePtr;
                ++m_size;
            }
        }
        else if(m_binPredicate(_obj.m_pair.first, _subRoot->key()))
        {
            avl_serialize_insert(_subRoot->m_left, _obj);
        }
        else
        {
            avl_serialize_insert(_subRoot->m_right, _obj);
        }
    }
    //--------------------------------------------------------------------------------------------------------------
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void AvlTree<_KeyType, _ValType, _Pred, _Alloc>::avl_serialize_from_list(node_pointer& _root, serializable_list& _list)
    //--------------------------------------------------------------------------------------------------------------
    {
        serializable_list::iterator begItr = _list.begin();
        serializable_list::iterator endItr = _list.end();
        while (begItr != endItr)
        {
            avl_serialize_insert(_root, *begItr);
            ++begItr;
        }
    }

} // End of namespace.

#endif
