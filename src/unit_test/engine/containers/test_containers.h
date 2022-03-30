#pragma once

#include <iostream>
#include <assert.h>

namespace glare
{
    enum { TestResult_Failure = -1, TestResult_Success = 0 };

    class TestLogger
    {
        TestLogger(){}

        TestLogger(const TestLogger&);
        TestLogger& operator =(const TestLogger&);

    public:
        template<typename T>
        const TestLogger& logClr(T& _obj) const
        {
            std::cout << _obj << "\n";
            return *this;
        }

        template<typename T>
        const TestLogger& clrLog(T& _obj) const
        {
            std::cout << "\n" << _obj;
            return *this;
        }

        template<typename T>
        const TestLogger& log(T& _obj) const
        {
            std::cout<<_obj;
            return *this;
        }

        template<typename T>
        const TestLogger& assertOn(T& _obj) const
        {
            ::assert(_obj);
            return *this;
        }

        template<typename T>
        const TestLogger& assertMsg(T& _obj, const char* const _str = nullptr) const
        {
            if(_str) logClr(_str);
            return assertOn(_obj);
        }

        static void create() { m_instance = new TestLogger(); }
        static TestLogger& get() { return *m_instance; }
        static TestLogger* getPtr() { return m_instance; }

    private:
        
        static TestLogger* m_instance;
    };
    
    #define TEST_LOG(_X)                    TestLogger::get().log(_X)
    #define TEST_CLRLOG(_X)                 TestLogger::get().clrLog(_X)
    #define TEST_LOGCLR(_X)                 TestLogger::get().logClr(_X)
    #define TEST_ASSERT(_X)                 TestLogger::get().assertOn(_X)
    #define TEST_ASSERT_MSG(_X, _MSG)       TestLogger::get().assertMsg(_X, _MSG)

    //----------------------------------------------------------------------------------------------------
    // Common object to use for all the tests.
    //----------------------------------------------------------------------------------------------------
    struct TestObjInfo
    {
        size_t m_id;
        
        size_t m_constructor_count;
        size_t m_destructor_count;
        size_t m_copyConstructor_count;
        size_t m_copyAssignment_count;

        explicit TestObjInfo(size_t _id=0): m_id(_id)
                                          , m_constructor_count(0)
                                          , m_destructor_count(0)
                                          , m_copyConstructor_count(0)
                                          , m_copyAssignment_count(0)
        {}

        void reset()
        {
            m_constructor_count     = 0;
            m_destructor_count      = 0;
            m_copyConstructor_count = 0;
            m_copyAssignment_count  = 0;
        }

        void dump()
        {
            TEST_LOGCLR("id: ").log(m_id);
            TEST_LOGCLR("constructor calls: ").log(m_constructor_count);
            TEST_LOGCLR("destructor calls: ").log(m_destructor_count);
            TEST_LOGCLR("copy constructor calls: ").log(m_copyConstructor_count);
            TEST_LOGCLR("assignment calls: ").log(m_copyAssignment_count);
        }

        bool isZero() const
        {
            return ( m_constructor_count == 0     && 
                     m_destructor_count == 0      && 
                     m_copyConstructor_count == 0 &&
                     m_copyAssignment_count == 0  );
        }

        bool operator ==(const TestObjInfo& _other) const
        {
            return m_constructor_count     == _other.m_constructor_count     &&
                   m_destructor_count      == _other.m_destructor_count      &&
                   m_copyConstructor_count == _other.m_copyConstructor_count &&
                   m_copyAssignment_count  == _other.m_copyAssignment_count;
        }
    };

    class TestObjectInfoType
    {
    public:
        TestObjectInfoType()
        {
            m_enable = true;
        }

        void registerCtorCall() const
        {
            if(m_enable) ++m_info.m_constructor_count;
        }

        void registerDtorCall() const
        {
            if(m_enable) ++m_info.m_destructor_count;
        }

        void registerCopyCtorCall() const
        {
            if(m_enable) ++m_info.m_copyConstructor_count;
        }

        void registerCopyAssignmentCall() const
        {
            if(m_enable) ++m_info.m_copyAssignment_count;
        }

        void reset() const { m_info.reset(); }
        void dump()  const { m_info.dump();  }
        
        bool operator ==(const TestObjectInfoType& _other) const { return m_info == _other.m_info; }

        typedef TestObjInfo state_object_type;
        state_object_type& getStateInfo() { return m_info; }

        static void enable(bool b = true) { m_enable = b; }
    private:
        mutable TestObjInfo m_info;
        static bool m_enable;
    };

    class TestObjectLogType
    {
    public:
        void registerCtorCall() const
        {
            if(m_verbose) { TEST_LOGCLR("constructor called"); }
        }
        void registerDtorCall() const
        {
            if(m_verbose) { TEST_LOGCLR("destructor called"); }
        }
        void registerCopyCtorCall() const
        {
            if(m_verbose) { TEST_LOGCLR("copy constructor called"); }
        }
        void registerCopyAssignmentCall() const
        {
            if(m_verbose) { TEST_LOGCLR("copy assignment called"); }
        }

        void reset() const {}
        void dump() const {}

        static void verbose(bool b=true) { m_verbose = b; }
        static void enable(bool b=true) { verbose(b); }

        typedef void state_object_type;
    private:
        static bool m_verbose;
    };

    //------------------------------------------------------------------------------
    template<typename ClassT, typename T = TestObjectLogType>
    class TestObject
    {
    public:
        TestObject()
        {
            m_state.registerCtorCall();
        }

        TestObject(const TestObject&)
        {
            m_state.registerCopyCtorCall();
        }

        TestObject& operator = (const TestObject&)
        {
            m_state.registerCopyAssignmentCall();
            return *this;
        }

        ~TestObject()
        {
            m_state.registerDtorCall();
        }

        static void enableState(bool b = true) { m_state.enable(b); }
        static void resetState() { m_state.reset(); }
        static T&   getState() { return m_state; }

        typedef ClassT  class_type;
        typedef T       state_type;

    private:
        
        static T m_state;
    };

    template<typename ClassTy, typename T>
    T TestObject<ClassTy, T>::m_state;

    //----------------------------------------------------------------------------------------------------
    // Exports
    //----------------------------------------------------------------------------------------------------


    int testAvl();
    int testContainer();

}

    

