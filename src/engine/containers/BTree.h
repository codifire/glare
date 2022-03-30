#ifndef GLARE_B_TREE_H
#define GLARE_B_TREE_H

#include "GlareCoreUtility.h"
#include "memory\allocators.h"

// TODO Isolate copy constructor calls, assignment operator calls and temporaries.
// TODO Provide copy constructor, assignment operator to the BTreeNode.
// TODO Modify BTreeNode::insertAt to copy construct values rather than assign.
// TODO Remove the memory leaks, branches and stuff.

// BTree or a multi-way tree is a tree with order greater than 2, O(m) > 2. This means that all the nodes can have at max m children, where m > 2.
// Binary Search Tree is of order 2, it has at most 2 children per node, 1 key to partition two children.

namespace glare
{
    typedef unsigned int  btree_order_t;

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    class BTreeNode
    {
    public:
        typedef BTreeNode                                                  node_type;
        typedef node_type*                                                 node_pointer;
        typedef const node_type*                                           const_node_pointer;
        typedef _ValueType                                                 value_type;
        typedef value_type*                                                pointer;
        typedef const value_type*                                          const_pointer;
        typedef value_type&                                                reference;
        typedef const value_type&                                          const_reference;
        typedef _keyType                                                   key_type;
        typedef GLARE_PAIR<key_type, value_type>                           pair_type;
        
        btree_order_t nbKeys() const { return m_keyCount; }
        key_type& key(btree_order_t _idx)
        { 
            key_type* m_keyArray = reinterpret_cast<key_type*>(m_keyBuffer);
            return m_keyArray[_idx];
        }
        const key_type& key(btree_order_t _idx) const 
        { 
            const key_type* m_keyArray = reinterpret_cast<const key_type*>(m_keyBuffer);
            return m_keyArray[_idx];
        }
        value_type& value(btree_order_t _idx) { return m_value[_idx]; }
        const value_type& value(btree_order_t _idx) const { return m_value[_idx]; }
        node_pointer& branch(btree_order_t _idx) { return m_branch[_idx]; }
        const node_pointer& branch(btree_order_t _idx) const { return m_branch[_idx]; }
        bool isFull() const { return m_keyCount == MAXKEYS; }

        bool findKeyPosition(const key_type& _key, btree_order_t& _pos) const;
        void insertAt(btree_order_t _pos, const key_type& _key, const_reference _val, node_pointer _rightBranch);

        void splitInsertAt(btree_order_t _pos, const key_type& _keyIn, const_reference _valueIn, node_pointer _rightBranchIn, 
                                       key_type* _medianKeyOut, pointer _medianValueOut, node_pointer _rightBranchOut);

        void moveLeft(btree_order_t rightBranchPosition);
        void moveRight(btree_order_t leftBranchPosition);
        node_pointer combine(btree_order_t leftBranchPosition);

        void removeLeafData(btree_order_t _idx);
        void copyInPredecessor(btree_order_t _idx);
        
        BTreeNode();
        ~BTreeNode();

        static const btree_order_t ORDER = _Order;      // Max nb branches.
        static const btree_order_t MAXKEYS = _Order-1;  // Max nb keys.
        static const btree_order_t MINKEYS = MAXKEYS/2; // Min nb keys.

        BTreeNode(const BTreeNode&);
        BTreeNode& operator= (const BTreeNode&);

    private:
        void release();
        void destroy_key_value(btree_order_t _pos)
        {
            key(_pos).~key_type();
            m_allocator.destroy(m_value + _pos);
        }
        void copy_construct_key_value(btree_order_t _pos, const key_type& _key, const value_type& _val)
        {
            key_type* m_keyArray = reinterpret_cast<key_type*>(m_keyBuffer);
            new (m_keyArray + _pos) key_type(_key);         // copy construct key.
            m_allocator.construct(m_value + _pos, _val);    // copy construct value.
        }
        void copy_assign_key_value(btree_order_t _pos, const key_type& _key, const value_type& _val)
        {
            key(_pos) = _key;
            value(_pos) = _val;
        }

        void shift_left(btree_order_t _begPos); // from _begPos till end, array shifts left, i.e. remove _begPos-1, array shrinks.
        void shift_right(btree_order_t _begPos, const _keyType& _key, const_reference _val, node_pointer _lefttBranch); // from _begPos till end, array shifts right, i.e. array grows.
        
        // A node consists of key-value pairs and branches.
        btree_order_t   m_keyCount;       // Nb of keys (or key-value pairs)
        unsigned char   m_keyBuffer[sizeof(key_type) * MAXKEYS]; // An optimization, so the keys are local to the object.
        node_pointer    m_branch[ORDER];  // Branches, 1 more than nb keys. 
        value_type*     m_value;          // Same nb of values as keys. Optimization not needed as the object is rarely fetched in comparison to keys.
        
        _AllocatorType  m_allocator;
    };
    
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::BTreeNode(): m_keyCount(0)
    {
        m_value = m_allocator.allocate(MAXKEYS); // We allocate space for MAXKEYS, or we reserve space but we don't construct unless needed.
        GLARE_MEMSET(m_value, 0, sizeof(value_type) * MAXKEYS);
        GLARE_MEMSET(m_branch, 0, sizeof(m_branch));
        GLARE_MEMSET(m_keyBuffer, 0, sizeof(m_keyBuffer));
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::~BTreeNode()
    {
        release();
        m_allocator.deallocate(m_value, MAXKEYS);
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::BTreeNode(const BTreeNode& _other): m_keyCount(_other.m_keyCount)
    {
        m_value = m_allocator.allocate(MAXKEYS); // We allocate space for MAXKEYS, or we reserve space but we don't construct unless needed.
        GLARE_MEMSET(m_value, 0, sizeof(value_type) * MAXKEYS);
        GLARE_MEMSET(m_branch, 0, sizeof(m_branch));
        GLARE_MEMSET(m_keyBuffer, 0, sizeof(m_keyBuffer));

        for (btree_order_t i=0; i<_other.m_keyCount; ++i) {
            copy_construct_key_value(i, _other.key(i), _other.value(i));
        }
    }
    
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::release()
    {
        for (btree_order_t i = m_keyCount; i>0; --i)
        {
            destroy_key_value(i-1);
        }
        m_keyCount = 0;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>& 
        BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::operator= (const BTreeNode& _other)
    {
        if (this != &_other)
        {
            // MinCount is the number of elements that can be copy assigned straightaway!
            const btree_order_t MinCount = _other.m_keyCount < m_keyCount ? _other.m_keyCount : m_keyCount;
            for (btree_order_t i = 0; i < MinCount; ++i) {
                copy_assign_key_value(i, _other.key(i), _other.value(i));
            }

            if(_other.m_keyCount < m_keyCount)
            { // then we already copied everything.
                for (btree_order_t i = MinCount; i < m_keyCount; ++i) {
                    destroy_key_value(i);
                }
            }
            else if(m_keyCount < _other.m_keyCount)
            {
                for (btree_order_t i = MinCount; i < _other.m_keyCount; ++i) {
                    copy_construct_key_value(i, _other.key(i), _other.value(i));
                }
            }

            m_keyCount = _other.m_keyCount;
        }
        return *this;
    }

    // Pre:  A key to search for and a _pos to be returned.
    // Post: If the key was found then true is returned and _pos is its position, otherwise, false
    //       is returned and _pos is returned as the next branch to take along the search path.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::findKeyPosition(const key_type& _key, btree_order_t& _pos) const
    {
        _pos = 0;
        while (_pos < m_keyCount && _key > key(_pos)) {
            ++_pos;
        }

        if (_pos < m_keyCount && _key == key(_pos)) {
            return true;
        }
        return false;
    }
    
    // Pre: Insertion always happens on the way up the tree at position _pos, calling this functions means a value is sent up by the lower level.
    //      We always send median up, which will then replace the existing value at _pos, hence cause it to shift to next place along with its right node.
    //Post: Key/value then occupies the _pos and fit the newly split right node to the right of Key/value at _pos, maintaining the Btree, Search Tree property.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::insertAt(btree_order_t _pos, const _keyType& _key, const _ValueType& _val, node_pointer _rightBranch)
    {
        GLARE_ASSERT(m_keyCount < MAXKEYS, "Fatal Error: Can't Insert in a full node.");

        if(_pos != m_keyCount)
        {
            // Since we are about to expand, copy construct the last element as they all move right, beginning from position '_pos'.
            const btree_order_t lastKeyPos = m_keyCount - 1;
            copy_construct_key_value(m_keyCount, key(lastKeyPos), value(lastKeyPos));
            m_branch[m_keyCount + 1] = m_branch[m_keyCount];

            for (btree_order_t i = lastKeyPos; i > _pos; --i)
            {
                copy_assign_key_value(i, key(i-1), value(i-1));
                m_branch[i+1] = m_branch[i];
            }

            copy_assign_key_value(_pos, _key, _val);
            m_branch[_pos+1] = _rightBranch;
        }
        else
        {
            copy_construct_key_value(m_keyCount, _key, _val);
            m_branch[_pos+1] = _rightBranch;
        }

        ++m_keyCount;
    }

    // Pre: Current node will be full and the new entry to be inserted belongs at position _pos; 0 <= _pos < ORDER.
    // Post: Current node is divided into two nodes, current becomes the left child of median and right child would be _rightBranchOut.
    // We shall divide the node so that the median is the largest entry in the left half.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::splitInsertAt(btree_order_t _pos, const _keyType& _keyIn, const _ValueType& _valueIn, node_pointer _rightBranchIn, 
                                                                                _keyType* _medianKeyOut, _ValueType* _medianValueOut, node_pointer _rightBranchOut)
    {
        GLARE_ASSERT(isFull(), "Fatal Error: Node must be full.");
        GLARE_ASSERT(_rightBranchOut->m_keyCount == 0, "Fatal Error: Right branch must be empty for division of keys.");

        btree_order_t mid = static_cast<btree_order_t>(ORDER/2); // The entries from mid and so on goes in the right half.

        if (_pos <= mid) // New entry belongs in left half.
        {
            for (btree_order_t i=mid; i<MAXKEYS; ++i)
            {
                const btree_order_t idx = i-mid;
                _rightBranchOut->copy_construct_key_value(idx, key(i), value(i));
                _rightBranchOut->m_branch[idx+1] = branch(i+1);
                destroy_key_value(i);
            }

            m_keyCount = mid;
            _rightBranchOut->m_keyCount = MAXKEYS - mid; // or MAXKEYS - m_keyCount;

            insertAt(_pos, _keyIn, _valueIn, _rightBranchIn);
        }
        else // New entry belongs in right half.
        {
            ++mid;

            for (btree_order_t i=mid; i<MAXKEYS; ++i)
            {
                const btree_order_t idx = i-mid;
                _rightBranchOut->copy_construct_key_value(idx, key(i), value(i));
                _rightBranchOut->m_branch[idx+1] = branch(i+1);
                destroy_key_value(i);
            }

            m_keyCount = mid;
            _rightBranchOut->m_keyCount = MAXKEYS - mid;

            _rightBranchOut->insertAt(_pos - mid, _keyIn, _valueIn, _rightBranchIn);
        }

        // copy the largest entry in the left half and send it as the median, as we decided.
        const btree_order_t medianIdx = m_keyCount-1;

        // Optimized by copy constructing the objects rather than default constructing and assigning.
        new (_medianKeyOut) key_type(key(medianIdx));                // copy construct key.
        m_allocator.construct(_medianValueOut, value(medianIdx));    // copy construct value.

        _rightBranchOut->m_branch[0] = branch(m_keyCount); // or medianIdx + 1; 'cos its right branch!

        destroy_key_value(medianIdx);  // Destroy the largest entry in the left half.

        --m_keyCount;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::removeLeafData(btree_order_t _idx)
    {
        GLARE_ASSERT(branch(_idx) == nullptr, "removeLeafData should only be invoked on the leaf");

        if (_idx == nbKeys()-1)
        {
            destroy_key_value(_idx); // destroy last key as a result of shifting left.
            // m_branch[nbKeys()] = nullptr; its a leaf.
            --m_keyCount;
        }
        else
        {
            shift_left(_idx + 1);
        }
    }
    
    // Pre: Current has n keys and n+1 branches, where n > 1.
    // Post: Shifts the key/value and branches left, overriding (_begPos-1) causing the array to shrink in size.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::shift_left(btree_order_t _begPos)
    {
        GLARE_ASSERT(_begPos > 0 && _begPos < nbKeys(), "Fatal Error, _begPos is out of bounds.");

        for (btree_order_t i = _begPos; i < nbKeys(); ++i)
        {
            copy_assign_key_value(i-1, key(i), value(i));
            m_branch[i-1] = m_branch[i];
        }

        m_branch[nbKeys()-1] = m_branch[nbKeys()];
        m_branch[nbKeys()] = nullptr;

        destroy_key_value(nbKeys()-1); // destroy last key as a result of shifting left.
        
        --m_keyCount;
    }
    
    // Pre: Current has n keys and n+1 branches, where n < MAXKEYS.
    // Post: Shifts the key/value and branches right, overriding (_begPos+1) with _begPos causing the array to grow in size.
    //       Note: key/value at _begPos co-exists with same key/value at _begPos+1; but branch[_begPos] is NULL. _begPos can then be copy assigned.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::shift_right(btree_order_t _begPos, const _keyType& _key, const _ValueType& _val, node_pointer _leftBranch)
    {
        GLARE_ASSERT(_begPos < nbKeys() && nbKeys() < MAXKEYS, "Fatal Error, _begPos is out of bounds.");

        // Since, the array is about to grow in size:
        const btree_order_t lastKeyPos = nbKeys()-1;
        copy_construct_key_value(nbKeys(), key(lastKeyPos), value(lastKeyPos));
        m_branch[nbKeys()+1] = m_branch[nbKeys()];

        for (btree_order_t i = lastKeyPos; i > _begPos; --i)
        {
            copy_assign_key_value(i, key(i-1), value(i-1));
            m_branch[i+1] = m_branch[i];
        }

        m_branch[_begPos + 1] = m_branch[_begPos];
        m_branch[_begPos] = _leftBranch;

        copy_assign_key_value(_begPos, _key, _val);

        ++m_keyCount;
    }

    // Pre: Current node has more than minimum number of entries in the right branch and one too few entries in the left branch.
    // Post: The left most entry from the right branch has moved into the current node, which has sent an entry into the left branch.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::moveLeft(btree_order_t rightBranchPosition)
    {
        const btree_order_t currKeyPosition = rightBranchPosition - 1;
        node_pointer ptrLeftBranch = branch(currKeyPosition);      // Left Branch of key(currKeyPosition).
        node_pointer ptrRightBranch = branch(rightBranchPosition); // Right Branch of key(currKeyPosition).
        GLARE_ASSERT(ptrRightBranch->nbKeys() > MINKEYS, "Fatal Error, Too few keys in the right branch, algorithm at fault.");

        // Step 1: move the current[pos-1] key/value into leftmost of the left branch.
        // Step 2: rightBranch->branch[0] becomes right branch of key/value inserted in step 1.
        //ptrLeftBranch->insertAt(ptrLeftBranch->nbKeys(), key(currKeyPosition), value(currKeyPosition), &(ptrRightBranch->branch(0)));

        ptrLeftBranch->copy_construct_key_value(ptrLeftBranch->nbKeys(), key(currKeyPosition), value(currKeyPosition));
        ptrLeftBranch->m_branch[++ptrLeftBranch->m_keyCount] = ptrRightBranch->branch(0);

        // Step 3: move the rightBranch[0] into current[pos-1].
        // Step 4: shift the rightBranch to fill the gap.
        copy_assign_key_value(currKeyPosition, ptrRightBranch->key(0), ptrRightBranch->value(0));
        ptrRightBranch->shift_left(1); // Shift all key/value and branches to 1 position left, beginning at position 1.
    }

    // Pre: Current node has more than minimum number of entries in the left branch and one too few entries in the right branch.
    // Post: The right most entry from the left branch has moved in to the current node, which has sent an entry into the right branch.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::moveRight(btree_order_t leftBranchPosition)
    {
        node_pointer ptrLeftBranch = branch(leftBranchPosition);      // Left Branch of key(leftBranchPosition).
        node_pointer ptrRightBranch = branch(leftBranchPosition + 1); // Right Branch of key(leftBranchPosition).
        GLARE_ASSERT(ptrLeftBranch->nbKeys() > MINKEYS, "Fatal Error, Too few keys in the left branch, algorithm at fault.");

        // Step 1: Shift all the entries in the rightBranch 1 place to the right to make space for insertion.
        // Step 2: Copy assign branch(leftBranchPosition) to rightBranch->keyValue(0).
        // Step 3: Copy ptrLeftBranch->branch(ptrLeftBranch->nbKeys()) to rightBranch(0).
        ptrRightBranch->shift_right(0, key(leftBranchPosition), value(leftBranchPosition), ptrLeftBranch->branch(ptrLeftBranch->nbKeys()));

        // Step 4: Copy ptrLeftBranch->KeyValue(ptrLeftBranch->nbKeys()) to branch(leftBranchPosition).
        // Step 5: Shrink leftBranch.
        const btree_order_t rightmostPos = ptrLeftBranch->nbKeys()-1;
        copy_assign_key_value(leftBranchPosition, ptrLeftBranch->key(rightmostPos), ptrLeftBranch->value(rightmostPos));
        ptrLeftBranch->destroy_key_value(rightmostPos);
        --ptrLeftBranch->m_keyCount;
    }

    // Pre: Current node has key whose child has too few entries to be moved, so, need to combine leftBranch, Key, rightBranch.
    // Post: Current node has 1 less entry because it is combined with its left and the right children.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>*
    BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::combine(btree_order_t rightBranchPosition)
    {
        const btree_order_t currKeyPosition = rightBranchPosition - 1; // Position of the current key to be brought into the left branch from the current node.
        node_pointer ptrLeftBranch = branch(currKeyPosition);      // Left Branch of key(currKeyPosition).
        node_pointer ptrRightBranch = branch(rightBranchPosition); // Right Branch of key(currKeyPosition).
        GLARE_ASSERT((ptrLeftBranch->nbKeys() + ptrRightBranch->nbKeys()) < MAXKEYS, "Fatal Error, Not enough space to combine, algorithm at fault.");

        ptrLeftBranch->copy_construct_key_value(ptrLeftBranch->nbKeys(), key(currKeyPosition), value(currKeyPosition));
        ptrLeftBranch->m_branch[++ptrLeftBranch->m_keyCount] = ptrRightBranch->branch(0);

        for (btree_order_t i=0; i<ptrRightBranch->nbKeys(); ++i)
        {
            ptrLeftBranch->copy_construct_key_value(ptrLeftBranch->nbKeys(), ptrRightBranch->key(i), ptrRightBranch->value(i));
            ptrLeftBranch->m_branch[++ptrLeftBranch->m_keyCount] = ptrRightBranch->branch(i+1);
        }

        for (btree_order_t i = currKeyPosition; i < nbKeys()-1; ++i)
        {
            copy_assign_key_value(i, key(i+1), value(i+1));
            m_branch[i+1] = m_branch[i+2];
        }
        
        m_branch[m_keyCount] = nullptr;
        destroy_key_value(m_keyCount-1);
        --m_keyCount;

        // ptrRightBranch->release(); Leave this to the caller of the function, it will destroy and deallocate.
        return ptrRightBranch;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTreeNode<_keyType, _ValueType, _Order, _AllocatorType>::copyInPredecessor(btree_order_t _idx)
    {
        node_pointer subRoot = branch(_idx);

        while (subRoot->branch(subRoot->nbKeys()) != nullptr)
        {
            subRoot = subRoot->branch(subRoot->nbKeys());
        }

        copy_assign_key_value(_idx, subRoot->key(subRoot->nbKeys()-1), subRoot->value(subRoot->nbKeys()-1));
    }

    // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // BTree follows:
    // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    template<typename _KeyType, typename _ValType, btree_order_t  _Order, typename _Alloc = default_allocator<_ValType> >
    class BTree
    {
        typedef typename BTreeNode<_KeyType, _ValType, _Order, _Alloc>::node_type   node_type;
        typedef typename node_type::node_pointer                                    node_pointer;
        typedef typename node_type::const_node_pointer                              const_node_pointer;
        typedef typename _Alloc::template rebind<node_type>::other                  node_allocator_type;
        typedef typename _Alloc::template rebind<_KeyType>::other                   key_allocator_type;

    public:
        typedef _KeyType                                        key_type;
        typedef _ValType                                        value_type;
        typedef value_type*                                     pointer;
        typedef const value_type*                               const_pointer;
        typedef value_type&                                     reference;
        typedef const value_type&                               const_reference;
        typedef std::size_t                                     size_type;
        typedef std::ptrdiff_t                                  difference_type;
        typedef _Alloc                                          allocator_type;
        typedef GLARE_PAIR<key_type, value_type>                pair_type;


        BTree();
        ~BTree();
        BTree(const BTree&);
        BTree& operator= (const BTree&);

        bool find(const key_type& _key, value_type& _pVal) const;
        pointer find(const key_type& _key);
        const_pointer find(const key_type& _key) const;
        bool insert(const pair_type& _pair);
        bool insert(const key_type& _key, const_reference _value);
        void remove(const key_type& _key);

        void clear();

    private:
        void cleanUp(node_pointer _subRoot);
        void copy(node_pointer& _copyRoot, const_node_pointer _originalRoot);

        enum ERCode_Insert
        {
            ERCode_Insert_Error_Invalid,
            ERCode_Insert_Success,
            ERCode_Insert_Overflow, // Means that, on our way up the Btree, the current node will try to insert the median.
            ERCode_Insert_Error_Duplicate,
        };

        bool internal_find(const_node_pointer _current, const key_type& _key, node_pointer& _retNode, btree_order_t& _position) const;
        bool internal_insert(const key_type& _key, const const_reference _val);
        ERCode_Insert internal_push_down_insert(node_pointer _current, const key_type& _key, const_reference _val,
                                                key_type* &_medianKeyOut, pointer& _medianValueOut, node_pointer& _rightBranchOut);

        bool internal_remove(const key_type& _key);
        bool internal_recursive_remove(node_pointer _current, const key_type& _key);
        void internal_restore(node_pointer _current, btree_order_t _pos);

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
            _alloc.construct(ptr, _value); // Copy Construct.
            return ptr;
        }

        template<typename T>
        void destroyObject(T& _alloc, typename T::pointer _ptr)
        {
            _alloc.destroy(_ptr);
            _alloc.deallocate(_ptr, 1);
        }

        node_pointer        m_root;
        node_allocator_type m_nodeAllocator;
        allocator_type      m_allocator;
        key_allocator_type  m_keyAllocator;
    }; // ----------- End of Class -----------


    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTree<_keyType, _ValueType, _Order, _AllocatorType>::BTree() : m_root(nullptr) {
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTree<_keyType, _ValueType, _Order, _AllocatorType>::~BTree() 
    {
        clear();
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTree<_keyType, _ValueType, _Order, _AllocatorType>::BTree(const BTree& _other) : m_root(nullptr)
    {
        copy(m_root, _other.m_root);
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    BTree<_keyType, _ValueType, _Order, _AllocatorType>& 
    BTree<_keyType, _ValueType, _Order, _AllocatorType>::operator = (const BTree& _other)
    {
        if (this != &_other)
        {
            clear();
            copy(m_root, _other.m_root);
        }
        return *this;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::internal_find(const_node_pointer _current, const key_type& _key, 
                                                                            node_pointer& _retNode, btree_order_t& _position) const
    {
        // _position = 0; // No need, _current->findKeyPosition() does it internally.
        while (_current != nullptr)
        {
            if(_current->findKeyPosition(_key, _position)) // Search for the key in the current node.
            { // Found.
                _retNode = const_cast<node_pointer>(_current);
                return true; // position is set correctly by the _current->findKeyPosition() method itself.
            }
            else
            { // Search Failed.
                _current = _current->branch(_position);
            }
        }

        return false;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::internal_insert(const key_type& _key, const const_reference _val)
    {
        key_type* medianKeyOut = nullptr; // Median Key-
        pointer medianValueOut = nullptr; // Value Pair
        node_pointer rightBranchOut = nullptr; // Right branch of Key-Value or Data.

        ERCode_Insert result = internal_push_down_insert(m_root, _key, _val, medianKeyOut, medianValueOut, rightBranchOut);
        if(result == ERCode_Insert_Overflow)
        {
            node_pointer nodePtr = createObject(m_nodeAllocator);
            nodePtr->branch(0) = m_root; // Left Branch
            nodePtr->insertAt(0, *medianKeyOut, *medianValueOut, rightBranchOut); // Set Median Key-Value with right branch, so this is new root.
            m_root = nodePtr;
            result = ERCode_Insert_Success;

            // Free the pointers.
            destroyObject(m_keyAllocator, medianKeyOut); medianKeyOut = nullptr;
            destroyObject(m_allocator, medianValueOut); medianValueOut = nullptr;
        }

        GLARE_ASSERT(medianKeyOut == nullptr && medianValueOut == nullptr, "Fatal Error: Memory Leak, why are the pointers not free");
        GLARE_ASSERT(result != ERCode_Insert_Error_Invalid, "How could the result be invalid!");

        return result == ERCode_Insert_Success;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType> 
    typename BTree<_keyType, _ValueType, _Order, _AllocatorType>::ERCode_Insert 
    BTree<_keyType, _ValueType, _Order, _AllocatorType>::internal_push_down_insert(node_pointer _current, const key_type& _key, const_reference _val,
                                                                                   key_type* &_medianKeyOut, pointer& _medianValueOut, 
                                                                                   node_pointer& _rightBranchOut)
    {
        ERCode_Insert result = ERCode_Insert_Error_Invalid;

        if (_current == nullptr)
        {
            GLARE_ASSERT(_medianKeyOut == nullptr && _medianValueOut == nullptr && _rightBranchOut == nullptr, "Fatal Error: How come pointer's not NULL?");
            _medianKeyOut = createObject(m_keyAllocator, _key);
            _medianValueOut = createObject(m_allocator, _val);
            _rightBranchOut = nullptr;
            result = ERCode_Insert_Overflow;
        }
        else
        {
            btree_order_t position; // No need to initialize.
            if(_current->findKeyPosition(_key, position)) // Search the current node for it.
            {
                result = ERCode_Insert_Error_Duplicate;
            }
            else
            {
                // Key was not found so advance the search.
                key_type* keyPtr = nullptr; // Key to be inserted in the current node
                pointer valuePtr = nullptr; // along with the value.
                node_pointer rightBranchPtr = nullptr; // Right branch of Key-Value or Data.

                // position means the branch to take to advance in search.
                result = internal_push_down_insert(_current->branch(position), _key, _val, keyPtr, valuePtr, rightBranchPtr);
                
                if (result == ERCode_Insert_Overflow)
                {
                    // We have something, try inserting it in the current node.
                    if (!_current->isFull())
                    {
                        result = ERCode_Insert_Success;
                        _current->insertAt(position, *keyPtr, *valuePtr, rightBranchPtr); // Will copy-assign from keyPtr, valuePtr, dispose them later.
                    }
                    else
                    {
                        //_medianKeyOut = createObject(m_keyAllocator);
                        //_medianValueOut = createObject(m_allocator);
                        // Optimization, don't construct them here, copy construct them in splitInsertAt()
                        _medianKeyOut = m_keyAllocator.allocate(1);
                        _medianValueOut = m_allocator.allocate(1);

                        _rightBranchOut = createObject(m_nodeAllocator);

                        // Use *keyPtr, *valuePtr objects to copy-assign and later dispose of them.
                        _current->splitInsertAt(position, *keyPtr, *valuePtr, rightBranchPtr, _medianKeyOut, _medianValueOut, _rightBranchOut);
                        // Let the result be ERCode_Insert_Overflow.
                    }

                    if (keyPtr) { destroyObject(m_keyAllocator, keyPtr); keyPtr = nullptr; }
                    if (valuePtr) { destroyObject(m_allocator, valuePtr); valuePtr = nullptr; }
                    // Note: rightBranchPtr is not destroyed, because it is actually added, not copy-assigned to the internals of the node like Key-Value.
                }

                GLARE_ASSERT(keyPtr == nullptr && valuePtr == nullptr, "Fatal Error: Memory Leak, why are the pointers not free");
            }
        }

        return result;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::internal_remove(const key_type& _key)
    {
        bool result = internal_recursive_remove(m_root, _key);
        if (result && m_root && m_root->nbKeys() == 0)
        {
            node_pointer ptrOldRoot = m_root;
            m_root = m_root->branch(0);
            destroyObject(m_nodeAllocator, ptrOldRoot);
        }
        return result;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::internal_recursive_remove(node_pointer _current, const key_type& _key)
    {
        bool result = false;
        
        if (_current == nullptr)
        {
            result = false; // Search failed, no such key present.
        }
        else
        {
            btree_order_t position; // No need for initialization.

            if (_current->findKeyPosition(_key, position))
            {
                // Key found, current node has it!
                if (_current->branch(position))
                {
                    // Not a leaf!
                    _current->copyInPredecessor(position);
                    internal_recursive_remove(_current->branch(position), _current->key(position));
                }
                else
                {
                    // Its a leaf!
                    _current->removeLeafData(position);
                }

                result = true;
            }
            else
            {
                result = internal_recursive_remove(_current->branch(position), _key);
            }

            if (_current->branch(position) && _current->branch(position)->nbKeys() < node_type::MINKEYS)
            {
                internal_restore(_current, position);
            }
        }

        return result;
    }

    // Pre: _current points to a non leaf node; _current->branch(position) is a node with one too few entries.
    // Post: An entry is taken from elsewhere to restore the minimum number of entries in the node to which _current->branch(position) points.
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTree<_keyType, _ValueType, _Order, _AllocatorType>::internal_restore(node_pointer _current, btree_order_t _position)
    {
        if (_position == _current->nbKeys())
        {
            // _current->branch(position) points to Right most node.
            if (_current->branch(_position - 1)->nbKeys() > node_type::MINKEYS)
            {
                _current->moveRight(_position - 1);
            }
            else
            {
                node_pointer ptrRightBranch = _current->combine(_position);
                destroyObject(m_nodeAllocator, ptrRightBranch);
            }
        }
        else if(_position == 0)
        {
            // _current->branch(position) points to Left most node.
            if (_current->branch(_position + 1)->nbKeys() > node_type::MINKEYS)
            {
                _current->moveLeft(1);
            }
            else
            {
                node_pointer ptrRightBranch = _current->combine(1);
                destroyObject(m_nodeAllocator, ptrRightBranch);
            }
        }
        else
        {
            // _current->branch(position) points to intermediate node.
            if (_current->branch(_position - 1)->nbKeys() > node_type::MINKEYS)
            {
                _current->moveRight(_position - 1);
            }
            if (_current->branch(_position + 1)->nbKeys() > node_type::MINKEYS)
            {
                _current->moveLeft(_position + 1);
            }
            else
            {
                node_pointer ptrRightBranch = _current->combine(_position);
                destroyObject(m_nodeAllocator, ptrRightBranch);
            }

        }
    }

    
    // Public Interface
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::find(const key_type& _key, value_type& _pVal) const
    {
        node_pointer nodePtr = nullptr;
        size_t position = node_type::MAXKEYS;

        if(internal_find(m_root, _key, nodePtr, position)) {
            _pVal = nodePtr->value(position);
            return true;
        }
        return false;
    }
    
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    _ValueType* BTree<_keyType, _ValueType, _Order, _AllocatorType>::find(const key_type& _key)
    {
        node_pointer nodePtr = nullptr;
        size_t position = node_type::MAXKEYS;

        if(internal_find(m_root, _key, nodePtr, position)) {
            return &nodePtr->value(position);
        }
        return nullptr;
    }
    
    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    const _ValueType* BTree<_keyType, _ValueType, _Order, _AllocatorType>::find(const key_type& _key) const
    {
        node_pointer nodePtr = nullptr;
        size_t position = node_type::MAXKEYS;

        if(internal_find(m_root, _key, nodePtr, position)) {
            return static_cast<const_pointer>(&nodePtr->value(position));
        }
        return nullptr;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::insert(const pair_type& _pair)
    {
        return internal_insert(_pair.first, _pair.second);
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    bool BTree<_keyType, _ValueType, _Order, _AllocatorType>::insert(const key_type& _key, const_reference _value)
    {
        return internal_insert(_key, _value);
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTree<_keyType, _ValueType, _Order, _AllocatorType>::remove(const key_type& _key)
    {
        internal_remove(_key);
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTree<_keyType, _ValueType, _Order, _AllocatorType>::clear()
    {
        cleanUp(m_root);
        m_root = nullptr;
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTree<_keyType, _ValueType, _Order, _AllocatorType>::cleanUp(node_pointer _subRoot)
    {
        if (_subRoot != nullptr)
        {
            for (btree_order_t i = 0; i <= _subRoot->nbKeys(); ++i) {
                cleanUp(_subRoot->branch(i));
            }
            destroyObject(m_nodeAllocator, _subRoot);
        }
    }

    template<typename _keyType, typename _ValueType, btree_order_t _Order, typename _AllocatorType>
    void BTree<_keyType, _ValueType, _Order, _AllocatorType>::copy(node_pointer& _copyRoot, const_node_pointer _originalRoot)
    {
        if (_originalRoot == nullptr) {
            _copyRoot = nullptr;
        }
        else
        {
            _copyRoot = createObject(m_nodeAllocator, *_originalRoot);
            for (btree_order_t i = 0; i <= _originalRoot->nbKeys(); ++i) {
                copy(_copyRoot->branch(i), _originalRoot->branch(i));
            }
        }
    }
    
} // namespace

#endif