#include "test_containers.h"
#include "containers/AvlTree.h"
#include "containers/GlareCoreUtility.h"

#include <vector>
#include <map>

namespace glare
{
    namespace
    {
        //typedef TestObject<TestObjectLogType>       TestLogObject;
        //typedef TestObject<TestObjectInfoType>      TestInfoObject;

        enum fake_type_avl { fake_type_val0, fake_type_val1, fake_type_val2, fake_type_val3, fake_type_val4 };

        typedef AvlTree<fake_type_avl, fake_type_avl>       FakeType;
        typedef TestObject<FakeType, TestObjectLogType>     TestLogObject;
        typedef TestObject<FakeType, TestObjectInfoType>    TestInfoObject;
        
        typedef AvlTree<int, TestInfoObject>        TestAvlTreeInfoType;
        typedef AvlTree<int, TestLogObject>         TestAvlTreeLogType;
        
        typedef TestAvlTreeInfoType                 TestAvlTreeType;
        typedef TestAvlTreeType::pair_type          PairType;
        typedef TestAvlTreeType::value_type         ValueType;
        typedef TestAvlTreeType::serializable_list  SearializableList;
        typedef TestAvlTreeType::serializable_type  SearializableType;

        //------------------------------------------------------------------------
        TestAvlTreeType testTreeA;
        TestAvlTreeType testTreeB;
        ValueType       logObj;

        PairType insertList[] = {
            PairType(500, logObj), PairType(400, logObj), PairType(600, logObj),
            PairType(300, logObj), PairType(450, logObj),
            PairType(550, logObj), PairType(700, logObj), PairType(525, logObj), PairType(575, logObj), PairType(650, logObj), PairType(800, logObj),
            PairType(200, logObj), PairType(350, logObj), PairType(425, logObj), PairType(475, logObj), PairType(100, logObj), PairType(250, logObj), PairType(470, logObj),
            PairType(510, logObj), PairType(530, logObj), PairType(625, logObj), PairType(675, logObj), PairType(750, logObj), PairType(900, logObj)
        };
    }

    void printAvl(TestAvlTreeType& _tree)
    {
        SearializableList readList;
        _tree.serializeList(readList, true);

        for (size_t i=0; i<_tree.size(); ++i)
        {
            SearializableType& element = readList[i];
            printf("Key: %d; BalanceFactor: %d\n", element.m_pair.first, element.m_balanceFactor);
        }
    }

    void edgeCaseDoubleLeft(const TestAvlTreeType& _tree)
    {
        TEST_LOGCLR("\n\n---------------------------------------------------");
        TEST_LOGCLR("Edge Case Double Left Higher");
        TEST_LOGCLR("\n---------------------------------------------------");
        TestAvlTreeType copyTree(_tree);
        copyTree.remove(510);
        copyTree.remove(575);

        printAvl(copyTree);
        TEST_LOGCLR("\n---------------------------------------------------");
    }

    void edgeCaseDoubleRight(const TestAvlTreeType& _tree)
    {
        TEST_LOGCLR("\n\n---------------------------------------------------");
        // This code used to crash once but its corrected now. The problem was a copy paste of extra balance_left statement.
        // Effected Function: balance_right: case node_type::LeftHigher->case node_type::EqualHeight->(balance_left: removed).
        TEST_LOGCLR("Edge Case Double Right Higher");
        TEST_LOGCLR("\n---------------------------------------------------");
        TestAvlTreeType copyTree(_tree);
        copyTree.remove(900);
        copyTree.remove(650);
        copyTree.remove(625);
        copyTree.remove(675);

        printAvl(copyTree);
        TEST_LOGCLR("\n---------------------------------------------------");
    }
    
    void edgeCaseDoubleLeft2(const TestAvlTreeType& _tree)
    {
        TEST_LOGCLR("\n\n---------------------------------------------------");
        TEST_LOGCLR("Edge Case Double Left Higher 2");
        TEST_LOGCLR("\n---------------------------------------------------");
        TestAvlTreeType copyTree(_tree);
        
        copyTree.insert(580, logObj);

        copyTree.remove(510);
        copyTree.remove(530);

        copyTree.remove(900);
        copyTree.remove(750);

        copyTree.remove(625);
        copyTree.remove(675);

        copyTree.remove(800);
        copyTree.remove(650);

        // Now lets see if this makes the root equal higher.
        copyTree.remove(100);
        copyTree.remove(250);
        copyTree.remove(470);

        // Lets make the whole tree a perfect one.
        copyTree.insert(560, logObj);

        printAvl(copyTree);

        GLARE_LOG("Size of the tree is: %u", copyTree.size());
        TEST_LOGCLR("\n---------------------------------------------------");
    }

    int testAvl()
    {
        TestObjectLogType::verbose();
        logObj.resetState();
        std::map<int, ValueType> mLogs; 
        std::map<int, ValueType> mLogs2;

        TEST_LOGCLR("\n\n---------------------------------------------------");
        TEST_LOGCLR("Begin Testing AVL Tree");

        size_t size_max = (sizeof(insertList) / sizeof(PairType));

        for (size_t i=0; i<size_max; ++i)
        {
            testTreeA.insert(insertList[i]);
        }

        testTreeB = testTreeA;
        testTreeA.clear();
        TestObjectInfoType state1 = logObj.getState();
        logObj.resetState();

        for (size_t i=0; i<size_max; ++i)
        {
            mLogs.insert(insertList[i]);
        }

        mLogs2 = mLogs;
        mLogs.clear();
        TestObjectInfoType state2 = logObj.getState();

        if (state1 == state2)
        {
            TEST_CLRLOG("States are equal to the standard types");
        }

        TEST_LOGCLR("\n\n---------------------------------------------------");
        printAvl(testTreeB);
        TEST_LOGCLR("---------------------------------------------------\n");

        edgeCaseDoubleLeft(testTreeB);
        edgeCaseDoubleRight(testTreeB);
        edgeCaseDoubleLeft2(testTreeB);
        
        TEST_LOGCLR("\n\n---------------------------------------------------");
        TEST_LOGCLR("Serialization read/write");
        TEST_LOGCLR("---------------------------------------------------\n");
        SearializableList readList;
        testTreeB.serializeList(readList, true);  // Read.
        
        TestAvlTreeType testTreeC;
        testTreeC.serializeList(readList, false); // Write.

        printAvl(testTreeC);

        TEST_LOGCLR("End Testing AVL Tree");
        TEST_LOGCLR("---------------------------------------------------\n");
        TestObjectLogType::verbose(false);
        return  TestResult_Success;
    }
}