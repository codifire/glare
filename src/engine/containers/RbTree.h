#ifndef GLARE_RED_BLAC0K_TREE_H
#define GLARE_RED_BLACK_TREE_H

#include "BSTNode.h"
#include "memory\allocators.h"
#include <iterator>

//-------------------------------------------------------------------------------------------------------------------
// RB Tree Properties:
//-------------------------------------------------------------------------------------------------------------------
// #1 Every node is either red or black.
// #2 The root is black.
// #3 Every leaf (NIL or Sentinel) is black.
// #4 If a node is red, then both its children are black.
// #5 For each node, all simple paths from the node to descendant leaves contain the same number of black nodes.
//-------------------------------------------------------------------------------------------------------------------

namespace glare
{
    template<typename KeyType, typename ValueType>
    class RbTreeNode
    {
    public:
        typedef RbTreeNode<KeyType, ValueType>      selftype;
        typedef selftype*                           node_pointer;
        typedef const selftype*                     const_node_pointer;
        typedef GLARE_PAIR<KeyType, ValueType>      value_type;

        typedef ValueType                           ValueType;
        typedef KeyType                             KeyType;

        enum Color { Black, Red };

        RbTreeNode();
        explicit RbTreeNode(const value_type& _pair);
        RbTreeNode(const RbTreeNode& _other);
        
        // Basic Node Functionality
        const KeyType&      key() const { return m_pair.first; }
        const ValueType&    value() const { return m_pair.second; }
        ValueType&          value() { return m_pair.second; }

        node_pointer        left() { return m_left; }
        node_pointer        right() { return m_right; }
        node_pointer        parent() { return m_parent; }

        const_node_pointer  left() const { return m_left; }
        const_node_pointer  right() const { return m_right; }
        const_node_pointer  parent() const { return m_parent; }

        value_type&         getData() { return m_pair; }
        const value_type&   getData() const { return m_pair; }

        // Methods regarding color of the node.
        Color color() const     { return m_color; }
        void  color(Color _col) { m_color = _col; }
        bool  isBlack() const   { return m_color == Black; }
        bool  isRed() const     { return m_color == Red; }

    public:
        node_pointer m_left;
        node_pointer m_right;
        node_pointer m_parent;

    private:
        Color m_color;
        value_type m_pair;

        // Not assigning nodes yet.
        RbTreeNode& operator= (const RbTreeNode& _other);
    };

    // Default Constructor
    template<typename KeyType, typename ValueType>
    RbTreeNode<KeyType, ValueType>::RbTreeNode(): m_left(nullptr)
                                                , m_right(nullptr)
                                                , m_parent(nullptr)
                                                , m_color(Red)
                                                , m_pair()
    {
    }

    // 1 Argument Constructor
    template<typename KeyType, typename ValueType>
    RbTreeNode<KeyType, ValueType>::RbTreeNode(const value_type& _pair): m_left(nullptr)
                                                                       , m_right(nullptr)
                                                                       , m_parent(nullptr)
                                                                       , m_color(Red)
                                                                       , m_pair(_pair)
    {
    }

    // Copy Constructor
    template<typename KeyType, typename ValueType>
    RbTreeNode<KeyType, ValueType>::RbTreeNode(const RbTreeNode& _other): m_left(nullptr)
                                                                        , m_right(nullptr)
                                                                        , m_parent(nullptr)
                                                                        , m_color(_other.m_color)
                                                                        , m_pair(_other.m_pair)
    {
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Follows the tree implementation:
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    template<typename _KeyType, typename _ValType, typename _Pred = less<_KeyType>, typename _Alloc = default_allocator<_ValType> >
    class RedBlackTree
    {
        typedef RedBlackTree<_KeyType, _ValType, _Pred, _Alloc> selftype;
        typedef RbTreeNode<_KeyType, _ValType>                  node_type;
        typedef typename node_type::node_pointer                node_pointer;
        typedef typename node_type::const_node_pointer          const_node_pointer;

        typedef typename 
        _Alloc::template rebind<node_type>::other               node_allocator_type;

        template<typename _KeyType, typename _ValType, typename node_type, typename selftype>
        friend class rb_tree_iterator;

    public:
        typedef _ValType                                        val_type;
        typedef _KeyType                                        key_type;
        typedef _Pred                                           key_compare; // binary predicate.

        typedef typename node_type::value_type                  value_type;
        typedef value_type*                                     pointer;
        typedef const value_type*                               const_pointer;
        typedef value_type&                                     reference;
        typedef const value_type&                               const_reference;
        typedef std::size_t                                     size_type;
        typedef std::ptrdiff_t                                  difference_type;
        typedef _Alloc                                          allocator_type;

        // ---------------------------------------------------------------------------------------------------------
        // Iterators!
        // ---------------------------------------------------------------------------------------------------------
        class const_iterator: public std::iterator<std::bidirectional_iterator_tag, value_type>
        {
            template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
            friend class RedBlackTree;

        public:
            const reference operator*() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return m_nodePtr->getData();
            }
            const pointer operator->() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return &m_nodePtr->getData();
            }
            const_iterator& operator++() 
            {
                increment(); 
                return *this; 
            }
            const_iterator operator++(int)
            {
                const_iterator temp = *this;
                increment(); 
                return temp;
            }
            const_iterator& operator--()
            {
                decrement(); 
                return *this; 
            }
            const_iterator operator--(int)
            {
                const_iterator temp = *this;
                decrement(); 
                return temp;
            }
            bool operator==(const const_iterator& _right) const
            {
                return (m_nodePtr == _right.m_nodePtr);
            }
            bool operator!=(const const_iterator& _right) const
            {
                return (m_nodePtr != _right.m_nodePtr);
            }

            const_iterator(): m_nodePtr(nullptr) {}
            explicit const_iterator(node_pointer _ptr): m_nodePtr(_ptr) {}

        protected:
            void increment()
            {
                if (m_nodePtr != nullptr) 
                    m_nodePtr = RedBlackTree::successor(m_nodePtr);
            }
            void decrement()
            {
                if (m_nodePtr != nullptr) 
                    m_nodePtr = RedBlackTree::predecessor(m_nodePtr);
            }

            node_pointer m_nodePtr;
        };

        class iterator: public const_iterator
        {
            template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
            friend class RedBlackTree;

        public:
            reference operator*() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return m_nodePtr->getData();
            }
            pointer operator->() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return &m_nodePtr->getData();
            }
            iterator& operator++() 
            {
                increment(); 
                return *this; 
            }
            iterator operator++(int)
            {
                const_iterator temp = *this;
                increment(); 
                return temp;
            }
            iterator& operator--()
            {
                decrement(); 
                return *this; 
            }
            iterator operator--(int)
            {
                const_iterator temp = *this;
                decrement(); 
                return temp;
            }
            bool operator==(const iterator& _right) const
            {
                return (m_nodePtr == _right.m_nodePtr);
            }
            bool operator!=(const iterator& _right) const
            {
                return (m_nodePtr != _right.m_nodePtr);
            }

            iterator(): const_iterator(nullptr) {}
            explicit iterator(node_pointer _ptr): const_iterator(_ptr) {}
        };

        class const_reverse_iterator: public std::iterator<std::bidirectional_iterator_tag, value_type>
        {
            template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
            friend class RedBlackTree;

        public:
            const_reverse_iterator(): m_nodePtr(nullptr){}
            explicit const_reverse_iterator(node_pointer _ptr): m_nodePtr(_ptr){}

            const reference operator*() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return m_nodePtr->getData();
            }
            const pointer operator->() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return &m_nodePtr->getData();
            }
            const_reverse_iterator& operator++()
            {
                decrement(); 
                return *this; 
            }
            const_reverse_iterator operator++(int)
            {
                const_reverse_iterator temp = *this;
                decrement(); 
                return temp;
            }
            const_reverse_iterator& operator--() 
            {
                increment(); 
                return *this; 
            }
            const_reverse_iterator operator--(int)
            {
                const_reverse_iterator temp = *this;
                increment(); 
                return temp;
            }

            bool operator==(const const_reverse_iterator& _right) const
            {
                return (const_iterator::m_nodePtr == _right.const_iterator::m_nodePtr);
            }
            bool operator!=(const const_reverse_iterator& _right) const
            {
                return (const_iterator::m_nodePtr != _right.const_iterator::m_nodePtr);
            }

        protected:
            void increment()
            {
                if (m_nodePtr != nullptr) 
                    m_nodePtr = RedBlackTree::successor(m_nodePtr);
            }
            void decrement()
            {
                if (m_nodePtr != nullptr) 
                    m_nodePtr = RedBlackTree::predecessor(m_nodePtr);
            }

            node_pointer m_nodePtr;
        };

        class reverse_iterator: public const_reverse_iterator
        {
            template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
            friend class RedBlackTree;

        public:
            reverse_iterator(): const_reverse_iterator(nullptr){}
            explicit reverse_iterator(node_pointer _ptr): const_reverse_iterator(_ptr){}

            reference operator*() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return m_nodePtr->getData();
            }
            pointer operator->() const
            {
                GLARE_ASSERT(m_nodePtr != nullptr, "Can't be nullptr");
                return &m_nodePtr->getData();
            }

            reverse_iterator& operator++()
            {
                decrement(); 
                return *this; 
            }
            reverse_iterator operator++(int)
            {
                reverse_iterator temp = *this;
                decrement(); 
                return temp;
            }
            reverse_iterator& operator--() 
            {
                increment(); 
                return *this; 
            }
            reverse_iterator operator--(int)
            {
                reverse_iterator temp = *this;
                increment(); 
                return temp;
            }

            bool operator==(const reverse_iterator& _right) const
            {
                return (m_nodePtr == _right.m_nodePtr);
            }
            bool operator!=(const reverse_iterator& _right) const
            {
                return (m_nodePtr != _right.m_nodePtr);
            }
        };
        // ---------------------------------------------------------------------------------------------------------

        RedBlackTree();
        ~RedBlackTree();

        RedBlackTree(const RedBlackTree& _other);
        RedBlackTree& operator= (const RedBlackTree& _other);

        GLARE_PAIR<iterator, bool> insert(const value_type& _pair);
        GLARE_PAIR<iterator, bool> insert(const key_type& _key, const val_type& _value);

        void erase(const key_type& _key);
        void erase(iterator& _itr);
        void erase(reverse_iterator& _itr);

        bool exists(const key_type& _key) const;
        bool find(const key_type& _key, val_type& _val) const;
        iterator find(const key_type& _key) const;

        void clear();

        size_type size() { return m_size; }
        bool empty() { return (m_size == 0); }

        void swap(RedBlackTree& _tree);

        iterator begin()
        { 
            return iterator(m_leftmost); // minimum(m_root)
        }
        iterator end()
        { 
            return iterator(nullptr);
        }

        reverse_iterator rbegin()
        { 
            return reverse_iterator(m_rightmost); // maximum(m_root)
        }
        reverse_iterator rend()
        { 
            return reverse_iterator(nullptr);
        }

    private:
        // Core
        bool rb_insert(const value_type& _pair, node_pointer& _newNodePtr);
        void rb_insert_fixup(node_pointer _newNodePtr);
        
        void rb_remove(node_pointer _nodeToDelete);
        void rb_remove_fixup(node_pointer x, node_pointer xp);

        node_pointer bst_find(const key_type& _key) const;

        // Helpers
        void transplant(node_pointer _u, node_pointer _v);
        void rotate_right(node_pointer _subRootPtr);
        void rotate_left(node_pointer _subRootPtr);

        // Post-Order style clean up.
        void internal_clean(node_pointer _subRoot);

        // Pre-Order style copy.
        void internal_copy(node_pointer& _copySubroot, const_node_pointer _originalSubroot, node_pointer _parent);
        
        // Static Helpers
        static node_pointer minimum(node_pointer _u);
        static node_pointer maximum(node_pointer _u);
        static node_pointer successor(node_pointer _x);
        static node_pointer predecessor(node_pointer _x);

        template<typename T>
        typename T::pointer createObject(T& _alloc)
        {
            typename T::pointer ptr = _alloc.allocate(1);
            _alloc.construct(ptr);
            return ptr;
        }

        template<typename T, typename VAL>
        typename T::pointer createObject(T& _alloc, const VAL& _value)
        {
            typename T::pointer ptr = _alloc.allocate(1);
            _alloc.construct(ptr, _value);
            return ptr;
        }

        template<typename T>
        void destroyObject(T& _alloc, typename T::pointer _ptr)
        {
            _alloc.destroy(_ptr);
            _alloc.deallocate(_ptr, 1);
        }

        size_type           m_size;
        node_pointer        m_root;
        node_pointer        m_leftmost;   // Leftmost(the lowest key) node stays leftmost till it is the lowest key in the tree, when we insert a node with even smaller key then we update it.
        node_pointer        m_rightmost;  // Rightmost(the highest key) node stays rightmost till we insert an even higher key-value node than this is.
        key_compare         m_binPredicate;
        node_allocator_type m_nodeAllocator;
    };
    
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::RedBlackTree(): m_size(0)
                                                                   , m_root(nullptr)
                                                                   , m_leftmost(nullptr)
                                                                   , m_rightmost(nullptr)
    {
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::~RedBlackTree()
    {
        clear();
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::RedBlackTree(const RedBlackTree& _other): m_size(0)
                                                                                             , m_root(nullptr)
                                                                                             , m_leftmost(nullptr)
                                                                                             , m_rightmost(nullptr)
                                                                                             , m_binPredicate(_other.m_binPredicate)
    {
        internal_copy(m_root, _other.m_root, nullptr); // parent of m_root is nullptr.
        if (m_root == nullptr)
        {
            m_leftmost = nullptr;
            m_rightmost = nullptr;
        }
        else
        {
            m_leftmost = minimum(m_root);
            m_rightmost = maximum(m_root);
        }
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>& RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::operator= (const RedBlackTree& _right)
    {
        if (this != &_right)
        {
            clear();
            internal_copy(m_root, _right.m_root, nullptr); // parent of m_root is nullptr.
            
            if (m_root == nullptr)
            {
                m_leftmost = nullptr;
                m_rightmost = nullptr;
            }
            else
            {
                m_leftmost = minimum(m_root);
                m_rightmost = maximum(m_root);
            }

            m_binPredicate = _right.m_binPredicate;
        }
        return *this;
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    GLARE_PAIR<typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::iterator, bool> RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::insert(const value_type& _pair)
    {
        iterator nodeItr;
        bool result = rb_insert(_pair, nodeItr.m_nodePtr);
        return GLARE_PAIR<iterator, bool>(nodeItr, result);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    GLARE_PAIR<typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::iterator, bool> RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::insert(const key_type& _key, const val_type& _value)
    {
        return insert(value_type(_key, _value));
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    bool RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::rb_insert(const value_type& _pair, node_pointer& _retNodePtr)
    {
        node_pointer parentPtr = nullptr, currentPtr = m_root;
        
        while(currentPtr != nullptr)
        {
            parentPtr = currentPtr;
            if (m_binPredicate(_pair.first, currentPtr->key())) // less_than(givenKey, currentPtr->key())
                currentPtr = currentPtr->m_left;
            else if (_pair.first == currentPtr->key()) 
            {
                _retNodePtr = currentPtr;
                return false; // Duplicate!
            }
            else
                currentPtr = currentPtr->m_right;
        }

        node_pointer newNodePtr = createObject(m_nodeAllocator, _pair);
        newNodePtr->m_parent = parentPtr;
        // newNodePtr->color(node_type::Red); // Red by default, no need to mention!
        _retNodePtr = newNodePtr;
        
        if(parentPtr) 
        {
            if (m_binPredicate(_pair.first, parentPtr->key())) // less_than(givenKey, currentPtr->key())
            {
                parentPtr->m_left = newNodePtr;
                if (parentPtr == m_leftmost)
                    m_leftmost = newNodePtr;
            }
            else
            {
                parentPtr->m_right = newNodePtr;
                if (parentPtr == m_rightmost)
                    m_rightmost = newNodePtr;
            }

            rb_insert_fixup(newNodePtr); // This is the only case when we need to fix the insertion.
        }
        else
        {
            m_root = newNodePtr;
            m_leftmost = newNodePtr;
            m_rightmost = newNodePtr;
            m_root->color(node_type::Black);
            // Insertion shouldn't be a problem as it is the first node to be inserted in the tree, need not be fixed.
        }
        
        ++m_size; // We now have a completely balanced RB Tree with 1 more node.
        return true;
    }
    
    // Pre: The insertion might have violated the RBTree properties by having a red node with red parent.
    // Post: The RB properties hold true and the tree becomes a legit RBtree.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::rb_insert_fixup(node_pointer _newNodePtr)
    {
        // Following loop invariant is true prior to the first iteration of the loop, 
        // and each iteration maintains the loop invariant:
        //
        // a. Node Z is red, where Z is the newly inserted node prior to call.
        // b. If Z.p is the root, then Z.p is black and did not change prior to the call.
        // c. If the tree violates any of the red-black properties, then it violates at most
        //    one of them, and the violation is of either property 2 or property 4. If the
        //    tree violates property 2, it is because Z is the root and is red. If the tree
        //    violates property 4, it is because both Z and Z.p are red.
        
        GLARE_ASSERT(_newNodePtr && _newNodePtr->m_parent, "Both should exist for the violation to happen");

        while (_newNodePtr != m_root && _newNodePtr->m_parent->color() == node_type::Red)
        {
            node_pointer grandParentPtr = _newNodePtr->m_parent->m_parent;
            if (_newNodePtr->m_parent == grandParentPtr->m_left)
            {
                node_pointer auntPtr = grandParentPtr->m_right;
                if (auntPtr && auntPtr->color() == node_type::Red)
                {
                    // Case 1: _newNodePtr, _newNodePtr->m_parent, auntPtr all red; flip color.
                    _newNodePtr->m_parent->color(node_type::Black);                  
                    auntPtr->color(node_type::Black);
                    grandParentPtr->color(node_type::Red);
                    _newNodePtr = grandParentPtr; // Problem is passed 2 levels up the tree.
                }
                else
                { 
                    // Aunt is black, this means at least 1 rotation will happen.
                    if (_newNodePtr == _newNodePtr->m_parent->m_right) // Do we need a double rotation?
                    {
                        // Case 2: Violation is made by having a red node on a zig-zag path.
                        _newNodePtr = _newNodePtr->m_parent; // Parent becomes left child of its current child after rotation, basically transforming to case 3.
                        rotate_left(_newNodePtr); // grandParentPtr is still the same and Case 3 is converted to Case 2.
                    }
                    // Case 3: Simple Zig-Zig case, 1 rotation will suffice.
                    _newNodePtr->m_parent->color(node_type::Black); // If its a Case 3 then this is the updated parent after left rotation.
                    grandParentPtr->color(node_type::Red);
                    rotate_right(grandParentPtr);
                }
            }
            else
            {
                // New node' parent is the right child of its grand parent!
                node_pointer auntPtr = grandParentPtr->m_left;
                if (auntPtr && auntPtr->color() == node_type::Red)
                {
                    // Case 1: _newNodePtr, _newNodePtr->m_parent, auntPtr all red; flip color.
                    _newNodePtr->m_parent->color(node_type::Black);
                    auntPtr->color(node_type::Black);
                    grandParentPtr->color(node_type::Red);
                    _newNodePtr = grandParentPtr; // Problem is passed 2 levels up the tree.
                }
                else
                {
                    // Aunt is black, this means at least 1 rotation will happen.
                    if (_newNodePtr == _newNodePtr->m_parent->m_left) // Do we need a double rotation?
                    {
                        // Case 2: Violation is made by adding a red node on a zag-zig path.
                        _newNodePtr = _newNodePtr->m_parent; // Transforming to case 3, _newNodePtr will be the violating Red Condition node after rotation.
                        rotate_right(_newNodePtr); // Transformed, now _newNodePtr points to the child that really violates the red condition!
                    }
                    // Case 3: Simple Zig-Zig case, 1 rotation will suffice.
                    _newNodePtr->m_parent->color(node_type::Black);
                    grandParentPtr->color(node_type::Red);
                    rotate_left(grandParentPtr);
                }
            }
        }
        m_root->color(node_type::Black);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::rotate_right(node_pointer _subRootPtr)
    {
        GLARE_ASSERT(_subRootPtr && _subRootPtr->m_left, "[RBTree][Logic Fail] This impossible situation shouldn't have arised.");

        node_pointer leftSubtree = _subRootPtr->m_left;
        _subRootPtr->m_left = leftSubtree->m_right;

        if (leftSubtree->m_right)
            leftSubtree->m_right->m_parent = _subRootPtr;

        leftSubtree->m_parent = _subRootPtr->m_parent;

        if (_subRootPtr->m_parent)
        {
            if (_subRootPtr == _subRootPtr->m_parent->m_left)
                _subRootPtr->m_parent->m_left = leftSubtree;
            else
                _subRootPtr->m_parent->m_right = leftSubtree;
        }
        else
        {
            m_root = leftSubtree;
        }

        leftSubtree->m_right = _subRootPtr;
        _subRootPtr->m_parent = leftSubtree;
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::rotate_left(node_pointer _subRootPtr)
    {
        GLARE_ASSERT(_subRootPtr && _subRootPtr->m_right, "[RBTree][Logic Fail] This impossible situation shouldn't have arised.");

        node_pointer rightSubtree = _subRootPtr->m_right;
        _subRootPtr->m_right = rightSubtree->m_left;

        if(rightSubtree->m_left)
            rightSubtree->m_left->m_parent = _subRootPtr;

        rightSubtree->m_parent = _subRootPtr->m_parent;

        if(_subRootPtr->m_parent)
        {
            if(_subRootPtr == _subRootPtr->m_parent->m_left)
                _subRootPtr->m_parent->m_left = rightSubtree;
            else
                _subRootPtr->m_parent->m_right = rightSubtree;
        }
        else
        {
            m_root = rightSubtree;
        }

        rightSubtree->m_left = _subRootPtr;
        _subRootPtr->m_parent = rightSubtree;
    }

    // Pre: We wand to replace the subtree rooted at node _u with the one rooted at _v.
    // Post: node _u’s parent becomes node _v’s parent, and _u’s parent ends up having _v as its appropriate child.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::transplant(node_pointer _u, node_pointer _v)
    {
        if (_u->parent() == nullptr)
            m_root = _v; // This means that _u is the root.
        else if (_u == _u->parent()->left())
            _u->parent()->m_left = _v;
        else
        {
            _u->parent()->m_right = _v;
        }

        if (_v)
            _v->m_parent = _u->m_parent;
    }
    
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
        RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::minimum(node_pointer _nodePtr)
    {
        while (_nodePtr->m_left != nullptr)
            _nodePtr = _nodePtr->m_left;
        return _nodePtr;
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
        RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::maximum(node_pointer _nodePtr)
    {
        while (_nodePtr->m_right != nullptr)
            _nodePtr = _nodePtr->m_right;
        return _nodePtr;
    }
    
    // We break the code for "inorder" successor into two cases:
    // * If the right subtree of node x is nonempty, then the successor of x is just the leftmost node in x’s right subtree.
    // * If the right subtree of node x is empty and x has a successor y, then y is the lowest ancestor of x whose left child
    //   is also an ancestor of x. To find y, we simply go up the tree from x until we encounter a node that is the left child of its parent.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
        RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::successor(node_pointer _x)
    {
        if(_x->right() != nullptr)
            return minimum(_x->right());

        node_pointer y = _x->parent();
        while (y != nullptr && _x == y->right())
        {
            _x = y;
            y = y->parent();
        }
        return y;
    }

    // Code for "inorder" predecessor is symmetric to the successor's code. We break the code for predecessor into two cases:
    // * If the left subtree of node x is nonempty, then the predecessor of x is just the rightmost node in x’s left subtree.
    // * If the left subtree of node x is empty and x has a predecessor y, then y is the highest ancestor of x whose right child
    //   is also an ancestor of x. To find y, we simply go up the tree from x until we encounter a node that is the right child of its parent.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
        RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::predecessor(node_pointer _x)
    {
        if (_x->left() != nullptr)
            return maximum(_x->left());

        node_pointer y = _x->parent();
        while (y != nullptr && _x == y->left())
        {
            _x = y;
            y = y->parent();
        }
        return y;
    }


    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::erase(const key_type& _key)
    {
        if (node_pointer nodeToDelete = bst_find(_key))
            rb_remove(nodeToDelete);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::erase(iterator& _itr)
    {
        if (_itr.m_nodePtr)
            rb_remove(_itr.m_nodePtr);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::erase(reverse_iterator& _itr)
    {
        if (_itr.m_nodePtr)
            rb_remove(_itr.m_nodePtr);
    }

    // Removal
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::rb_remove(node_pointer _nodeToDelete)
    {
        node_pointer y = _nodeToDelete; // 'y' is a node that either gets removed from then tree or moved within the tree (in this case its a successor).
        node_type::Color originalColorY = y->color();
            
        node_pointer x = nullptr;  // we keep track of the node 'x' that moves into node y' original position, thus replacing it. x could also be a nullptr.
        node_pointer xp = nullptr; // New parent of x. This used to be the parent of 'y' before 'x' replaced it.
        // So 'x' being the node replacing 'y' and xp is x's new parent after replacing y.

        // We found the node to be deleted!
        if(y->m_left == nullptr)
        {
            x = y->m_right; // x could be a nullptr.
            xp = y->parent();
            transplant(y, x);
            GLARE_ASSERT((x == nullptr || x->isRed()), "If a node has only one child, that child has to be Red otherwise RB Properties are violated");
        }
        else if(y->m_right == nullptr)
        {
            x = y->m_left; // here 'x' could not be a nullptr, otherwise it wouldn't have entered this else-if part.
            xp = y->parent();
            transplant(y, x);
            GLARE_ASSERT(x->isRed(), "If a node has only one child, that child has to be Red otherwise RB Properties are violated");
        }
        else
        {
            y = minimum(_nodeToDelete->m_right); // 'y' now points to the successor indication that its being removed, 'cos successor replaces '_nodeToDelete' takes it's color which keeps the equation same at the _nodeToDelete level.
                                                // The node that actually gets missing is the successor node itself, ignore its value because what matters now is its color. If it was black then we have a violation.
            originalColorY = y->color();
            x = y->right(); // x could be a nullptr. x will replace 'y' because y is being re-placed in the tree.
            xp = y->parent(); // xp will be x' new parent after the transplant(y, x).

            if (xp == _nodeToDelete)
                xp = y; // We don't want xp to point to _nodeToDelete as it is being deleted and also the real parent of x in this particular case will itself be y.
            else
            {
                transplant(y, x); // After this transplant x takes y' position and y' parent becomes x' parent, i.e. 'xp'.
                y->m_right = _nodeToDelete->m_right;
                _nodeToDelete->m_right->m_parent = y;
            }
            transplant(_nodeToDelete, y); // replace _nodeToDelete with its successor.
            y->m_left = _nodeToDelete->m_left;
            y->m_left->m_parent = y;
            y->color(_nodeToDelete->color());
        }

        // One important point, if m_leftmost or m_rightmost is the _nodeToDelete then 'y' can't be a successor(_nodeToDelete). 
        // As m_leftmost or m_rightmost can have at the most one child right or left respectively.
        if (m_leftmost == _nodeToDelete) // Leftmost can have a right child, which could be nullptr.
        {
            m_leftmost = (x == nullptr) ? xp : minimum(x); // x can only be right child of leftmost node.
        }
        // Not using else, because if leftmost == rightmost == _nodeToDelete == m_root, then both should be modified.
        if(m_rightmost == _nodeToDelete)
        {
            m_rightmost = (x == nullptr) ? xp : maximum(x);  // x can only be left child of rightmost node.
        }

        // If y is black then moving y within the tree causes any simple path that previously contained y to have one fewer black node. Thus, it violates property #5.
        // We can correct the violation of property #5 by saying that node x, now occupying y' original position, has an "extra" black. That is, if we add 1 to the 
        // count of black nodes on any simple path that contains x, then under this interpretation, property 5 holds. When we remove or move the black node y, we "push" 
        // its blackness onto node x. The problem is that now node x is neither red nor black, thereby violating property 1. Instead, node x is either "doubly black" or
        // "red-and-black," and it contributes either 2 or 1, respectively, to the count of black nodes on simple paths containing x. The color attribute of x will still 
        // be either RED (if x is red-and-black) or BLACK (if x is doubly black). In other words, the extra black on a node is reflected in x's pointing to the node rather 
        // than in the color attribute.
        if (originalColorY == node_type::Black)
            rb_remove_fixup(x, xp); // x can be nullptr that is why we want to send x's parent separately.

        destroyObject(m_nodeAllocator, _nodeToDelete); // Delete the actual node now!
        --m_size; // We have 1 less number of nodes now.
    }
    
    // Pre: rb_remove may violate property #2, #4, #5
    //  * #2, if the node to delete was root and a red root replaced it.
    //  * #4, if the deleted node was black with a red child (right), which replaces it.
    //  * #5, if the deleted node was black with both child nils then we are short of 1 black count on this path. 
    //    It has supposedly pushed its blackness to its child which replaces it, and this child is now doubly black. 
    //    1 count of black because its a nil node & 1 because it has the blackness pushed upon it by its former black parent.

    // Post: Violations of property #2, #4, #5 are removed and the tree is back to being a legit RB Tree.
    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::rb_remove_fixup(node_pointer _x, node_pointer _xp)
    {
        // Some Conventions.
        //       (p)
        //      //  \
        //     x     s
        //
        // x is the double black node which replaced its former black parent.
        // s is the sibling.
        // p is the parent.

        // All possible cases for the doubly black node 'x':
        // Case #1: sibling of x is black and has 1 red child. This is a terminal case.
        //       a: When sibling is right and its right child is red, color of left doesn't matter. Same for when sibling is left and its left child.
        //       b: When sibling is right and its left child is red, right is black, we transform it to case #1(a). Same for when the sibling is left
        //          and its right child is red, and left child is black. we make rotation to transform it to case #1(a).
        //
        // Case #2: sibling of x is black and both of s' children are black. This is the recoloring case only.
        //       a: If parent p is red then recoloring is the terminal case.
        //       b: If parent p is black, then x will point to p now, indicating that it is p which is doubly black now. and propagates the problem upwards.
        //
        // Case #3: sibling 's' of x is red, in this case we make a rotation on 'p' and transform into one of the above mentioned cases.

        node_pointer x = _x;   // Double black node.
        node_pointer xp = _xp; // Parent of the double black node. Why take xp separately when we can do x->parent(), the answer is that x can also be nullptr and xp can't be nullptr.

        while ( x != m_root && (x == nullptr || x->isBlack()) )
        {
            // Loop Invariant:
            // x is always the double black node in question at the beginning of the iteration.
            // x is never a root, because when that happens we just simply discard the extra black.
            // The idea is to find a red node on the path upward and change the pair (RED, DOUBLE-BLACK) to (BLACK, BLACK).
            // Restructuring solves the problem locally while recoloring may propagate it upwards.

            if (x == xp->left()) // x is the left child of its parent.
            {
                node_pointer s = xp->right();    

                // Check if it is case #3 then transform it into case #1 or #2.
                if (s->isRed()) // Case #3
                { 
                    // s being red means its children and its parent must be black.
                    s->color(node_type::Black); // s replaces p so it takes its color to not change the overall picture.
                    xp->color(node_type::Red);   // we color parent red because we already have s (its 'now' parent) as black.
                    rotate_left(xp); // 'xp' will still remain the parent of 'x' after the left rotation.
                    s = xp->right(); // left child of older 's' now becomes new sibling after rotation. It must be black by property #4.
                }

                // At this point we know that 's' is black whatsoever, so we don't need to test for that. Either Case #1 or #2 applies.
                if ( (s->right() == nullptr || s->right()->isBlack()) && (s->left() == nullptr || s->left()->isBlack()) )
                {
                    s->color(node_type::Red); // Take 1 black from both the double black node and 's' and pass it on to its parent.
                    x = xp; // the notion of being double black or having an extra black count is reflected by 'x' pointing to a node.
                    // So, if xp was black then it becomes double black now. if it was red then it becomes red-black node. x' pointing to it adds an extra black.
                    // If we came from Case#3 then the loop will terminate after this as xp must be red. After the while loop x is turned to black which solves the problem.
                    xp = xp->parent();
                }
                else // its case #1 then.
                { 
                    // Test for case #1(b), if it is then transform it to case #1(a).
                    if (s->right() == nullptr || s->right()->isBlack()) // If s->right is black then s->left must be red, 'cos we are in the else part & we know 1 of the 2 children is RED for sure.
                    {
                        s->left()->color(node_type::Black);
                        s->color(node_type::Red);
                        rotate_right(s); // Transformed!
                        s = xp->right();  // The new sibling is black with a red right child, case #1(a), woo!
                    }

                    // Case #1(a) applies
                    s->color(xp->color()); // Sibling takes its parent's color to keep the bigger picture similar after rotation.
                    xp->color(node_type::Black); // Parent becomes black to consume the extra black of x.
                    s->right()->color(node_type::Black); // s' right child becomes black to compensate for 1 black count that came from s before rotation.
                    rotate_left(xp);
                    x = m_root; // a way to break.
                }
            }
            else // x is the right child of its parent, all of the above code, therefore, will be mirrored.
            {
                node_pointer s = xp->left();

                if (s->isRed()) // Case #3
                {
                    s->color(node_type::Black); // We can also swap the colors but direct setting the colors is much better.
                    xp->color(node_type::Red);
                    rotate_right(xp);
                    s = xp->left(); // The new sibling is black, congratulations!
                }

                // At this point we know that 's' is black so one of the cases #1 or #2 apply.
                // Test for case #2
                if ( (s->left() == nullptr || s->left()->isBlack()) && (s->right() == nullptr || s->right()->isBlack()) )
                {
                    s->color(node_type::Red); // Take 1 black count from both the "supposedly" double-black node and 's' and pass it on to its parent 'p'.
                    x = xp; // Now parent's color either become red-black or double-black. If red-black then its a terminal case.
                    // Notion of a node being red-black or double-black is reflected by x' pointing to it. The actual color value stays red or black respectively.
                    xp = xp->parent();
                }
                else
                {
                    // Test for case #1b, if it is #1(b), indeed, transform it to #1(a).
                    if (s->left() == nullptr || s->left()->isBlack())
                    {
                        s->right()->color(node_type::Black);
                        s->color(node_type::Red);
                        rotate_left(s);
                        s = xp->left();
                    }

                    // Case #1(a) applies
                    s->color(xp->color());
                    xp->color(node_type::Black);
                    s->left()->color(node_type::Black); // Left can never be nullptr, impossible.
                    rotate_right(xp);
                    x = m_root; // a way to break.
                }
            }
        }

        if (x)
            x->color(node_type::Black); // fixes property #2, #4
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::node_pointer 
    RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::bst_find(const key_type& _key) const
    {
        node_pointer currentPtr = m_root;

        while(currentPtr != nullptr)
        {
            if (m_binPredicate(_key, currentPtr->key())) // less_than(givenKey, currentPtr->key())
                currentPtr = currentPtr->m_left;
            else if (_key == currentPtr->key())
                break; // Found!
            else
                currentPtr = currentPtr->m_right;
        }

        return currentPtr; 
    }


    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    bool RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::exists(const key_type& _key) const
    {
        return (bst_find(_key) != nullptr);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    bool RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::find(const key_type& _key, val_type& _val) const
    {
        if(node_pointer nodePtr = bst_find(_key))
        {
            _val = nodePtr->value();
            return true;

        }
        return false;
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    typename RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::iterator 
        RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::find(const key_type& _key) const
    {
        return iterator(bst_find(_key));
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::swap(RedBlackTree& _tree)
    {
        if (this != &_tree)
        {
            // Swap size
            size_type tmpSize = m_size;
            m_size = _tree.m_size;
            _tree.m_size = tmpSize;

            // Swap root
            node_pointer tmpRoot = m_root;
            m_root = _tree.m_root;
            _tree.m_root = tmpRoot;

            // Swap Leftmost
            tmpRoot = m_leftmost;
            m_leftmost = _tree.m_leftmost;
            _tree.m_leftmost = tmpRoot;

            // Swap Rightmost
            tmpRoot = m_rightmost;
            m_rightmost = _tree.m_rightmost;
            _tree.m_rightmost = tmpRoot;

            // Swap comparator
            key_compare tmpBinPredicate = m_binPredicate;
            m_binPredicate = _tree.m_binPredicate;
            _tree.m_binPredicate = tmpBinPredicate;

            // Swap allocator
            node_allocator_type tmpNodeAllocator = m_nodeAllocator;
            m_nodeAllocator = _tree.m_nodeAllocator;
            _tree.m_nodeAllocator = tmpNodeAllocator;
        }
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    inline void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::clear()
    {
        if (m_root)
        {
            internal_clean(m_root);
            m_root = nullptr;
            m_leftmost = nullptr;
            m_rightmost = nullptr;
            m_size = 0;
        }
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::internal_clean(node_pointer _subroot)
    {
        if (_subroot->m_left)
            internal_clean(_subroot->m_left);
        
        if (_subroot->m_right)
            internal_clean(_subroot->m_right);

        // Notice: PostOrder style destruction.
        m_nodeAllocator.destroy(_subroot);
        m_nodeAllocator.deallocate(_subroot, 1);
    }

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>::internal_copy(node_pointer& _refSubroot, const_node_pointer _originalSubroot, node_pointer _parent)
    {
        if (_originalSubroot == nullptr)
        {
            _refSubroot = nullptr;
        }
        else
        {
            // Notice: Pre-Order style create and copy.
            _refSubroot = createObject(m_nodeAllocator, *_originalSubroot);
            _refSubroot->m_parent = _parent;
            ++m_size;

            internal_copy(_refSubroot->m_left, _originalSubroot->m_left, _refSubroot);
            internal_copy(_refSubroot->m_right, _originalSubroot->m_right, _refSubroot);
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------------------------------
    // Efficient swap for the RedBlackTree class template. Thanks to ADL that we'll find this swap first not the std::swap().
    // ---------------------------------------------------------------------------------------------------------------------------------------------------------

    // Offer a public swap member function that efficiently swaps the value of two objects of your type.
    // Offer a non-member swap in the same namespace as your class or template.
    // If you're writing a class (not a class template), specialize std::swap for your class. So we can't fully specialize std::swap.
    // When calling swap, employ a using declaration for std::swap, then call swap without namespace qualification.

    template<typename _KeyType, typename _ValType, typename _Pred, typename _Alloc>
    void swap(RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>& _left, RedBlackTree<_KeyType, _ValType, _Pred, _Alloc>& _right)
    {
        _left.swap(_right);
    }
}

#endif