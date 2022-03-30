#include "containers/containers.h"
#include "test_containers.h"



namespace glare
{
    TestLogger* TestLogger::m_instance = nullptr;
    bool TestObjectLogType::m_verbose = false;
    bool TestObjectInfoType::m_enable = false;

    int testContainer()
    {
        TestLogger::create();
        TEST_LOGCLR("Begin Test Containers");

        testAvl();

        return TestResult_Success;
    }

} // namespace glare