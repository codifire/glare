#include "test_containers.h"
#include "containers/BTree.h"
#include <string>
#include "gtest/gtest.h"


namespace glare { namespace glare_test { namespace test_btree
{ 
    // --------------------------------------------------------------------------------------------------
    using namespace std;
    
    enum fake_btree_type { fake_btree_val0, fake_btree_val1, fake_btree_val2, fake_btree_val3, fake_btree_val4 };
    const btree_order_t G_ORDER = 6;

    typedef fake_btree_type                                                 FakeType;
    typedef TestObject<FakeType, TestObjectInfoType>                        TestObjectType;

    typedef int                                                             test_key_t;
    typedef TestObjectType                                                  test_val_t;
    typedef default_allocator<test_val_t>                                   test_allocator_t;
    typedef default_allocator<test_key_t>                                   test_key_allocator_t;
    
    typedef BTreeNode<test_key_t, test_val_t, G_ORDER, test_allocator_t>    test_node_t;
    typedef BTree<test_key_t, test_val_t, G_ORDER, test_allocator_t>        test_btree_t;

    typedef default_allocator<test_node_t>                                  test_node_allocator_t;
    typedef test_node_t::node_pointer                                       node_pointer;

    test_allocator_t g_valueAllocator;
    test_key_allocator_t g_keyAllocator;
    test_node_allocator_t g_nodeAllocator;
    node_pointer g_rootNode = nullptr;
    node_pointer g_leftChild = nullptr;
    node_pointer g_rightChild = nullptr;

    const size_t G_ARR_SIZE = test_node_t::MAXKEYS-1; // because 1 was added initially!
    test_key_t g_arrKeys[G_ARR_SIZE] = {40, 30, 20, 0};
    test_key_t g_sortedArrKeys[test_node_t::MAXKEYS] = {0, 10, 20, 30, 40};

    TestObjectType::state_type::state_object_type& refState = TestObjectType::getState().getStateInfo();

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

    bool isLeafNode(node_pointer _ptr)
    {
        for (btree_order_t i=0; i<_ptr->ORDER; ++i)
        {
            if(_ptr->branch(i) != nullptr)
                return false;
        }

        return true;
    }

    TEST(Btree_Node_Test, test_1_check_default_construction)
    {
        using namespace glare;
        test_node_t node;

        EXPECT_EQ(0, node.nbKeys()) << "Keys must be 0";
        EXPECT_TRUE( refState.isZero() ) << "REF STATE NOT ZERO";
        EXPECT_TRUE( isLeafNode(&node) ) << "This node was meant to be the root alone with no children";
    };
    
    TEST(Btree_Node_Test, test_2_check_key_insertion)
    {
        using namespace glare;
        test_node_t node;
        test_val_t temp;
        
        size_t expect_cc_count = refState.m_copyConstructor_count; // cc: copy constructor
        size_t expect_ao_count = refState.m_copyAssignment_count; // ao: assignment operator
        size_t expect_c_count  = refState.m_constructor_count;  // c: constructor
        size_t expect_d_count  = refState.m_destructor_count;  // d: destructor

        ++expect_cc_count; // This operation must involve 1 copy constructor call only.

        node_pointer nodePtr = createObject(g_nodeAllocator);
        nodePtr->branch(0) = nullptr;            // Left Branch
        nodePtr->insertAt(0, 10, temp, nullptr); // Set a Key-Value with its right branch, so this is a new root.
        g_rootNode = nodePtr;

        EXPECT_TRUE( isLeafNode(g_rootNode) ) << "This node was meant to be the root alone with no children";
        EXPECT_FALSE( refState.isZero() ) << "We just inserted a value, it should have been copy constructed.";

        EXPECT_TRUE( refState.m_copyConstructor_count == expect_cc_count ) << "copy constructor call count doesn't meet the criterion, fix it";
        EXPECT_TRUE( refState.m_constructor_count     == expect_c_count  ) << "constructor call count doesn't meet the criterion, fix it";
        EXPECT_TRUE( refState.m_destructor_count      == expect_d_count  ) << "destructor call count doesn't meet the criterion, fix it";
        EXPECT_TRUE( refState.m_copyAssignment_count  == expect_ao_count ) << "assignment call count doesn't meet the criterion, fix it";
        EXPECT_EQ(1, g_rootNode->nbKeys()) << "Keys must be 1";

        for (btree_order_t i = 0; i < G_ARR_SIZE; ++i)
        {
            const test_key_t keyToInsert = g_arrKeys[i];
            
            btree_order_t pos;
            EXPECT_FALSE( g_rootNode->findKeyPosition(keyToInsert, pos) ) << "Key mustn't be found!";
            
            // This operation must involve 1 copy constructor call and 1 copy assignment call.
            ++expect_cc_count; // Copy construction always happen!
            expect_ao_count += (g_rootNode->nbKeys() - pos); // Its a fact, verified by simulating with pen and paper!

            g_rootNode->insertAt(pos, keyToInsert, temp, nullptr);

            EXPECT_TRUE( refState.m_copyConstructor_count == expect_cc_count ) << "copy constructor call count doesn't meet the criterion, fix it";
            EXPECT_TRUE( refState.m_constructor_count     == expect_c_count  ) << "constructor should not have been called yet, fix it";
            EXPECT_TRUE( refState.m_destructor_count      == expect_d_count  ) << "destructor should not have been called yet, fix it";
            EXPECT_TRUE( refState.m_copyAssignment_count  == expect_ao_count ) << "assignment call count doesn't meet the criterion, fix it";
            EXPECT_EQ( keyToInsert, g_rootNode->key(pos) ) << "Key is not in its desired position"; // Check if the inserted key is in it's right position
        }

        for (int i = 0; i < g_rootNode->MAXKEYS; i++)
        {
            EXPECT_EQ( g_sortedArrKeys[i], g_rootNode->key(i) ) << "Key aren't in the right place";
        }

        EXPECT_TRUE( g_rootNode->isFull() );
        EXPECT_TRUE( isLeafNode(g_rootNode) ) << "This node was meant to be the root alone with no children";
    }

    TEST(Btree_Node_Test, test_3_check_node_split_insertion)
    {
        test_key_t* keyPtr = g_keyAllocator.allocate(1); // Key to be inserted in the current node
        test_val_t* valuePtr = g_valueAllocator.allocate(1); // along with the value.
        node_pointer rightBranchPtr = g_nodeAllocator.allocate(1); ; // Right branch of Key-Value or Data.
        g_nodeAllocator.construct(rightBranchPtr);

        // At this time we assume the keys in the root node to be laid like: {0, 10, 20, 30, 40};
        // Inserting 35 seems like a good idea, we can verify insertion by looking rightNode->key(0)
        test_key_t key(35);
        test_val_t value;
        btree_order_t position;

        EXPECT_FALSE( g_rootNode->findKeyPosition(key, position) ) << "This value was never inserted!";
        EXPECT_TRUE( g_rootNode->isFull() ) << "Should have been full for this test to work properly!";

        g_rootNode->splitInsertAt(position, key, value, nullptr, keyPtr, valuePtr, rightBranchPtr);

        g_leftChild = g_rootNode;
        g_rightChild = rightBranchPtr;

        node_pointer nodePtr = createObject(g_nodeAllocator);
        nodePtr->branch(0) = g_leftChild; // Left Branch
        nodePtr->insertAt(0, *keyPtr, *valuePtr, rightBranchPtr); // Set a Key-Value with its right branch, so this is a new root.
        g_rootNode = nodePtr;

        destroyObject(g_keyAllocator, keyPtr);
        destroyObject(g_valueAllocator, valuePtr);

        EXPECT_EQ(30, g_rootNode->key(0));
        EXPECT_EQ(1, g_rootNode->nbKeys());
        EXPECT_EQ(3, g_leftChild->nbKeys());
        EXPECT_EQ(2, g_rightChild->nbKeys());
        EXPECT_EQ(35, g_rightChild->key(0));
        EXPECT_FALSE( isLeafNode(g_rootNode) );
        EXPECT_TRUE( isLeafNode(g_leftChild) );
        EXPECT_TRUE( isLeafNode(g_rightChild) );
    }
    
    TEST(Btree_Node_Test, test_4_check_remove_data)
    {
        // At this point, after test_3_check_node_split_insertion test g_leftChild has 3 keys, we remove 1 here.
        const btree_order_t nbRemainingKeys = g_leftChild->nbKeys() - 2;
        g_leftChild->removeLeafData(0);
        g_leftChild->removeLeafData(1);
        EXPECT_EQ( nbRemainingKeys, g_leftChild->nbKeys() );
    }

    TEST(Btree_Node_Test, test_5_check_move_left)
    {
        test_key_t key(50);
        test_val_t value;
        btree_order_t position;
        g_rightChild->findKeyPosition(key, position);

        btree_order_t oldNbKeysRightChild = g_rightChild->nbKeys();
        btree_order_t oldNbKeysLeftChild  = g_leftChild->nbKeys();

        g_rightChild->insertAt(position, key, value, nullptr);

        EXPECT_TRUE( g_leftChild->nbKeys() < test_node_t::MINKEYS );
        EXPECT_EQ( oldNbKeysRightChild+1, g_rightChild->nbKeys() );
        EXPECT_TRUE( g_rightChild->nbKeys() > test_node_t::MINKEYS );

        oldNbKeysRightChild = g_rightChild->nbKeys();
        
        btree_order_t oldRootVal = g_rootNode->key(0);
        btree_order_t oldRightChildVal0 = g_rightChild->key(0);
        btree_order_t oldLeftChildValLast = g_leftChild->key(oldNbKeysLeftChild - 1);

        g_rootNode->moveLeft(1);

        EXPECT_EQ( oldNbKeysRightChild - 1, g_rightChild->nbKeys() );
        EXPECT_EQ( oldNbKeysLeftChild + 1, g_leftChild->nbKeys() );
        EXPECT_EQ( oldRootVal, g_leftChild->key(g_leftChild->nbKeys()-1) );
        EXPECT_EQ( oldLeftChildValLast, g_leftChild->key(g_leftChild->nbKeys()-2) );
        EXPECT_EQ( oldRightChildVal0, g_rootNode->key(0) );
        EXPECT_EQ( g_leftChild->nbKeys(), test_node_t::MINKEYS );
        EXPECT_EQ( g_rightChild->nbKeys(), test_node_t::MINKEYS );

        EXPECT_LE( 1, g_leftChild->key(0) );

        g_leftChild->findKeyPosition(1, position);
        g_leftChild->insertAt(position, key, value, nullptr);

        EXPECT_TRUE( isLeafNode(g_leftChild) );
        EXPECT_TRUE( isLeafNode(g_rightChild) );
    }

    TEST(Btree_Node_Test, test_6_check_move_right)
    {
        // Should be done after move left, because at this point we have laid our foundation.
        g_rightChild->removeLeafData(0);

        EXPECT_GT( g_leftChild->nbKeys(), test_node_t::MINKEYS );
        EXPECT_LT( g_rightChild->nbKeys(), test_node_t::MINKEYS );

        btree_order_t oldValRoot = g_rootNode->key(0);
        btree_order_t oldValLCLast = g_leftChild->key(g_leftChild->nbKeys() - 1);
        btree_order_t oldValRC0 = g_rightChild->key(0);

        btree_order_t nbOldKeysLC = g_leftChild->nbKeys();
        btree_order_t nbOldKeysRC = g_rightChild->nbKeys();

        g_rootNode->moveRight(0);

        EXPECT_EQ( oldValRoot, g_rightChild->key(0) );
        EXPECT_EQ( oldValLCLast, g_rootNode->key(0) );
        EXPECT_EQ( oldValRC0, g_rightChild->key(1) );
        EXPECT_GT( nbOldKeysLC, g_leftChild->nbKeys() );
        EXPECT_LT( nbOldKeysRC, g_rightChild->nbKeys() );

        EXPECT_TRUE( isLeafNode(g_leftChild) );
        EXPECT_TRUE( isLeafNode(g_rightChild) );
    }

    TEST(Btree_Node_Test, test_7_check_node_combine)
    {
        EXPECT_EQ( g_leftChild->nbKeys(), test_node_t::MINKEYS );
        EXPECT_EQ( g_rightChild->nbKeys(), test_node_t::MINKEYS );

        g_leftChild->removeLeafData(0);
        EXPECT_LT( g_leftChild->nbKeys(), test_node_t::MINKEYS );

        node_pointer ptrRightBranch = g_rootNode->combine(1);
        EXPECT_EQ( ptrRightBranch, g_rightChild );
        EXPECT_TRUE( isLeafNode(g_rightChild) );
        destroyObject(g_nodeAllocator, ptrRightBranch);

        node_pointer ptrOldRoot = g_rootNode;
        EXPECT_FALSE( isLeafNode(ptrOldRoot) );
        g_rootNode = g_rootNode->branch(0);
        destroyObject(g_nodeAllocator, ptrOldRoot);
        EXPECT_EQ( g_rootNode, g_leftChild );

        g_leftChild = nullptr;
        g_rightChild = nullptr;
    }

    TEST(Btree_Node_Test, test_8_check_copy_constructor_assignment)
    {
        test_node_t node(*g_rootNode);
        test_node_t node2;
        node2 = *g_rootNode;

        EXPECT_EQ(g_rootNode->nbKeys(), node.nbKeys());
        EXPECT_EQ(g_rootNode->nbKeys(), node2.nbKeys());
    }

    TEST(Btree_Node_Test, test_7_check_node_destroy)
    {
        // If all went fine, then we only had one node left after combining g_leftChild, g_rootNode and g_rightChild.
        EXPECT_TRUE( isLeafNode(g_rootNode) );
        destroyObject(g_nodeAllocator, g_rootNode); // All the memory allocated so far should be deallocated.
        // We only need to delete the root node, assuming rest is taken care of in the previous test cases till test_7_check_node_combine.
        // destroyObject(g_nodeAllocator, g_leftChild);  // This call should not be required after the test_7_check_node_combine test case.
        // destroyObject(g_nodeAllocator, g_rightChild); // This call should not be required after the test_7_check_node_combine test case.
        EXPECT_TRUE( (refState.m_constructor_count + refState.m_copyConstructor_count) == refState.m_destructor_count ) << "Fatal Error, possible memory leak";
    }

    // Btree tests follows:
    TEST(Btree_Test, test_1_basic)
    {
        TestObjectType::resetState();

        test_btree_t btree;
        test_val_t value;

        EXPECT_EQ(btree.find(5), nullptr);

        EXPECT_TRUE(btree.insert(5, value));
        EXPECT_TRUE(btree.find(5) != nullptr);
        btree.remove(5);
        EXPECT_EQ(btree.find(5), nullptr);
    }

    TEST(Btree_Test, test_2_detailed)
    {
        test_btree_t btree;
        test_btree_t btreecopy;
        test_val_t value;

        test_key_t keys[] = {
            100, 200, 300, 400, 500,
            150, 250, 450, 550, 600,
            625, 675, 700, 125, 145,
            235, 365, 375, 190, 715,
        };

        const size_t Size = sizeof(keys) / sizeof(test_key_t);

        for(size_t i = 0; i < Size; ++i)
            EXPECT_TRUE(btree.insert(keys[i], value));

        for(size_t i = 0; i < Size; ++i)
            EXPECT_NE(btree.find(keys[i]), nullptr);

        for(size_t i = 0; i < Size; ++i)
            EXPECT_EQ(btree.find(keys[i]+3), nullptr);
        
        // Test Assignment.
        btreecopy = btree;

        for(size_t i = 0; i < Size; ++i)
            btree.remove(keys[i]);

        EXPECT_EQ(btree.find(550), nullptr); // try find?
        
        // Test copy construction.
        test_btree_t btreecopyctor(btreecopy);

        for(size_t i = 0; i < Size; ++i)
        {
            EXPECT_NE(btreecopy.find(keys[i]), nullptr);
            EXPECT_NE(btreecopyctor.find(keys[i]), nullptr);
        }
    }

    TEST(Btree_Test, test_3_check_memory_leaks)
    {
        EXPECT_TRUE((refState.m_constructor_count + refState.m_copyConstructor_count) == refState.m_destructor_count) << "Fatal Error, possible memory leak";
    }

    // --------------------------------------------------------------------------------------------------
}   // namespace test_btree
}   // namespace glare_test
}   // namespace glare