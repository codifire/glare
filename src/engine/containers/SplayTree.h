#ifndef GLARE_SPLAY_TREE_H
#define GLARE_SPLAY_TREE_H

#include "GlareCoreUtility.h"
#include "memory\allocators.h"
#include "BSTNode.h"

// TODO Support for iterator and then return pair<bool, iterator> from the splay()

/*
    Splay trees are binary search trees that achieve our goals by being self-adjusting self-adjusting trees
    in a quite remarkable way: Every time we access a node of the tree, whether for insertion or retrieval, 
    we perform radical surgery on the tree, lifting the newly accessed node all the way up, so that it 
    becomes the root of the modified tree. Other nodes are pushed out of the way as necessary to make room 
    for this new root. Nodes that are frequently accessed will frequently be lifted up to become the root, 
    and they will never drift too far from the top position. Inactive nodes, on the other hand, will slowly 
    be pushed farther and farther from the root.

    We perform the radical surgery on splay trees by using rotations of a similar form to those used for AVL trees, 
    but now with many rotations done for every insertion or retrieval in the tree. In fact, rotations are done all 
    along the path from the root to the target node that is being accessed.

    The key idea of splaying is to move the target node two levels up the tree at each step. Consider the path going 
    from the root down to the accessed node. Each time we move left going down this path, we say that we zig, and each 
    time we move right we say that we zag. A move of zig and zag two steps left (going down) is then called zig-zig, 
    two steps right zag-zag, left then right zig-zag, and right then left zag-zig. These four cases are the only 
    possibilities in moving two steps down the path. If the length of the path is odd, however, there will be one more 
    step needed at its end, either a zig(left) move, or a zag(right) move.

    The zig-zag case is identical to that of an AVL double rotation, and the zig case is identical to a single rotation.
    The zig-zig case, however, is not the same as would be obtained by lifting the target node twice with single rotations.
    Always think of lifting the target two levels at a time (except when only a single zig or zag step remains at the end).
    Also, it is only the nodes on the path from the target to the root whose relative positions are changed.
*/

namespace glare
{
    template<typename _keyType, typename _ValueType, typename _Pred = less<_keyType>, typename _AllocatorType = default_allocator<_ValueType> >
    class SplayTree
    {
    private:
        typedef BSTNode<_keyType, _ValueType>                   node_type;
        typedef node_type*                                      node_pointer;
        typedef const node_type*                                const_node_pointer;

        typedef typename 
        _Alloc::template rebind<node_type>::other               node_allocator_type;
        
    public:
        typedef _keyType                                        key_type;
        typedef _Pred                                           key_compare; // binary predicate.

        typedef _ValueType                                      value_type;
        typedef value_type*                                     pointer;
        typedef const value_type*                               const_pointer;
        typedef value_type&                                     reference;
        typedef const value_type&                               const_reference;
        typedef std::size_t                                     size_type;
        typedef std::ptrdiff_t                                  difference_type;
        typedef typename node_type::value_type                  pair_type;

        typedef _Alloc                                          allocator_type;

    public:
        SplayTree();
        ~SplayTree();

        SplayTree(const SplayTree& _ref);
        SplayTree& operator = (const SplayTree&);

        bool insert(const pair_type& _pair);
        void insert(const key_type&, const_reference);

        void erase(const key_type&);
        void remove(const key_type&);
        
        bool find(const key_type& _key, value_type& _pVal) const;
        pointer find(const key_type& _key);
        const_pointer find(const key_type& _key) const;

        void clear();
        void size();

    private: // Helpers
        void rotate_right(node_pointer& _root);
        void rotate_left(node_pointer& _root);
        void join_larger_key_subtree(node_pointer& _root, node_pointer& _firstLarge);
        void join_smaller_key_subtree(node_pointer& _root, node_pointer& _firstLarge);

        bool splay(const key_type& _key);

    private:
        node_pointer m_root;
        size_type    m_size;
        key_compare  m_binPredicate;
        void (*m_traversalFunc)(node_pointer _node, process_data_cb _funcCb);
        node_allocator_type m_nodeAllocator;
    };


    // ------------------------------------------------------------------------------------------------------------------------------------
    template<typename _keyType, typename _ValueType, typename _Pred, typename _AllocatorType>
    void SplayTree<_keyType, _ValueType, _Pred, _AllocatorType>::rotate_right(node_pointer& _root)
    // ------------------------------------------------------------------------------------------------------------------------------------
    {
        node_pointer leftSubtree = _root->m_left;
        _root->m_left = leftSubtree->m_right;
        leftSubtree->m_right = _root;
        _root = leftSubtree;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------
    template<typename _keyType, typename _ValueType, typename _Pred, typename _AllocatorType>
    void SplayTree<_keyType, _ValueType, _Pred, _AllocatorType>::rotate_left(node_pointer& _root)
    // ------------------------------------------------------------------------------------------------------------------------------------
    {
        node_pointer rightSubtree = _root->m_right;
        _root->m_right = rightSubtree->m_left;
        rightSubtree->m_left = _root;
        _root = rightSubtree;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------
    /*
        Note the similarity of join_larger_key_subtree to a zig move: In both cases the left child node moves up to replace
        its parent node, which moves down into the right subtree. In fact, join_larger_key_subtree is exactly a zig move except 
        that the link from the former left child down to the former parent is deleted; instead, the parent (with its right subtree) 
        moves into the larger-key subtree.

        The three-way invariant tells us that every key in central subtree comes before every key in the larger-key subtree; 
        Hence root (with its right subtree) must be attached on the left of the leftmost node in the larger-key subtree.
    */
    template<typename _keyType, typename _ValueType, typename _Pred, typename _AllocatorType>
    void SplayTree<_keyType, _ValueType, _Pred, _AllocatorType>::join_larger_key_subtree(node_pointer& _root, node_pointer& _firstLarge)
    // ------------------------------------------------------------------------------------------------------------------------------------
    {
        _firstLarge->m_left = _root;
        _firstLarge = _root;
        _root = _root->m_left;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------
    template<typename _keyType, typename _ValueType, typename _Pred, typename _AllocatorType>
    void SplayTree<_keyType, _ValueType, _Pred, _AllocatorType>::join_smaller_key_subtree(node_pointer& _root, node_pointer& _lastSmall)
    // ------------------------------------------------------------------------------------------------------------------------------------
    {
        _lastSmall->m_right = _root;
        _lastSmall = _root;
        _root = _root->m_right;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------
    /*  << Uses Top-down splaying algorithm. >>

        While splaying proceeds, the tree temporarily falls apart into separate subtrees, which are reconnected 
        after the target is made the root.

        We shall use three subtrees, three-way tree split as follows:

        * The central subtree: contains nodes within which the target will lie if it is present.

        * The smaller-key subtree: contains nodes with keys strictly less than the target; in fact, every key in 
          the smaller-key subtree is less than every key in the central subtree.

        * The larger-key subtree: contains nodes with keys strictly greater than the target; in fact, every key in 
          the larger-key subtree is greater than every key in the central subtree.

        These conditions will remain true throughout the splaying process, so we shall call them the three-way invariant.

        Initially, the central subtree is the whole tree, and the smaller-key and larger key subtrees are empty, so the 
        three-way invariant is initially correct. As the search proceeds, nodes are stripped off the central subtree and 
        joined to one of the other two subtrees. When the search ends, the root of the central subtree will be the target 
        node if it is present, and the central subtree will be empty if the target was not found. In either case, all the 
        components will finally be joined together with the target as the root.
    */
    template<typename _keyType, typename _ValueType, typename _Pred, typename _AllocatorType>
    bool SplayTree<_keyType, _ValueType, _Pred, _AllocatorType>::splay(const key_type& _key)
    // ------------------------------------------------------------------------------------------------------------------------------------
    {
        node_type dummyNode; dummyNode.m_left = dummyNode.m_right = nullptr;
        
        node_pointer currentPtr     = m_root;       // Middle or Central Subtree.
        node_pointer lastSmallPtr   = &dummyNode;   // Node with largest key in Smaller-Key Subtree.
        node_pointer firstLargePtr  = &dummyNode;   // Node with smallest key in Larger-Key Subtree.
        node_pointer childPtr       = nullptr;

        while (currentPtr || _key != currentPtr->key()) // Quit if key is found or it doesn't exist.
        {
            if (_key < currentPtr->key())
            {
                // Go Left.
                childPtr = currentPtr->m_left;
                if (childPtr == nullptr || _key == childPtr->key()) // Zig.
                {
                    // We join, so, currentPtr, i.e. root of central subtree, is either what we are looking for or nullptr.
                    join_larger_key_subtree(currentPtr, firstLargePtr);
                }
                else if (_key < childPtr->key()) // Zig-Zig Case.
                {
                    rotate_right(currentPtr); // After this it becomes a simple zig case.
                    join_larger_key_subtree(currentPtr, firstLargePtr); // A node is lifted two levels up after this point, Zig.
                }
                else // Zig-Zag, key lies between (currentPtr, childPtr).
                {
                    join_larger_key_subtree(currentPtr, firstLargePtr);
                    join_smaller_key_subtree(currentPtr, lastSmallPtr);
                }
            } 
            else
            {
                // Go Right.
                childPtr = currentPtr->m_right;
                if (childPtr == nullptr || _key == childPtr->key())
                {
                    join_smaller_key_subtree(currentPtr, lastSmallPtr);
                }
                else if (_key > childPtr->key()) // Zag-Zag.
                {
                    rotate_left(currentPtr);
                    join_smaller_key_subtree(currentPtr, lastSmallPtr);
                } 
                else // Zag-Zig.
                {
                    join_smaller_key_subtree(currentPtr, lastSmallPtr);
                    join_larger_key_subtree(currentPtr, firstLargePtr);
                }
            }
        }

        bool resFound = false;

        if (currentPtr)
        {
            // Found Key in the existing tree.
            lastSmallPtr->m_right = currentPtr->m_left;
            firstLargePtr->m_left = currentPtr->m_right;
            resFound = true;
        }
        else
        {
            // Key not found in the existing tree, will add new node.
            currentPtr = m_nodeAllocator.allocate(1);
            m_nodeAllocator.construct(currentPtr); // Default construct.
            lastSmallPtr->m_right = firstLargePtr->m_left = nullptr;
            ++m_size;
        }

        m_root = currentPtr;-+
        m_root->m_left = dummyNode.m_right; // Root of smaller-key subtree, as we started by adding to the right most in the smaller-key subtree.
        m_root->m_right = dummyNode.m_left; // Root of larger-key subtree, as we started by adding to the left most in the larger-key subtree.

        return resFound;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------------------------------


    // ------------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------------------------------

} // namespace

#endif