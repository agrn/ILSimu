#ifndef __RASSEIVER_TEST_CIRCULAR_BUFFER_HPP
# define __RASSEIVER_TEST_CIRCULAR_BUFFER_HPP

# include <cppunit/extensions/HelperMacros.h>
# include <array>

class CircularBufferTestCase: public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE(CircularBufferTestCase);
        CPPUNIT_TEST(test_switch);
        CPPUNIT_TEST(test_direct_access);
        CPPUNIT_TEST(test_size);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();

protected:
        void test_switch();
        void test_direct_access();
        void test_size();

private:
        std::array<int16_t, 5> data1, data2, data3;
};

#endif  /* __RASSEIVER_TEST_CIRCULAR_BUFFER_HPP */
