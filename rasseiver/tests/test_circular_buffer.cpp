#include <cppunit/config/SourcePrefix.h>

#include "test_circular_buffer.hpp"
#include "../src/circular_buffer.hpp"

#include <numeric>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(CircularBufferTestCase);

void CircularBufferTestCase::setUp() {
	std::iota(data1.begin(), data1.end(), 0);
	std::iota(data2.begin(), data2.end(), 10);
	std::iota(data3.begin(), data3.end(), 20);
}

void CircularBufferTestCase::test_switch() {
	CircularBuffer<int16_t> buffer;

	buffer.switch_buffer(data1.data(), data1.size());

	for (int i {0}; i < (int) data1.size(); ++i) {
		CPPUNIT_ASSERT_EQUAL(buffer[i], data1[i]);
	}

	buffer.switch_buffer(data2.data(), data2.size());
	for (int i {0}; i < (int) data2.size(); ++i) {
		CPPUNIT_ASSERT_EQUAL(buffer[i], data2[i]);
		CPPUNIT_ASSERT_EQUAL(buffer[-i - 1],
                                     data1[data1.size() - i - 1]);
	}

	buffer.switch_buffer(data3.data(), data3.size());
	for (int i {0}; i < (int) data3.size(); ++i) {
		CPPUNIT_ASSERT_EQUAL(buffer[i], data3[i]);
		CPPUNIT_ASSERT_EQUAL(buffer[-i - 1],
                                     data2[data2.size() - i - 1]);
	}

        buffer.switch_buffer();
        for (int i {0}; i < (int) data3.size(); ++i) {
		CPPUNIT_ASSERT_EQUAL(buffer[-i - 1],
                                     data3[data3.size() - i - 1]);
	}
}

void CircularBufferTestCase::test_direct_access() {
        CircularBuffer<int16_t> buffer;

        buffer.switch_buffer(data1.data(), data1.size());
        {
                std::vector<int16_t> const &current {buffer.get_current()};

                for (int i {0}; i < (int) data1.size(); ++i) {
                        CPPUNIT_ASSERT_EQUAL(current[i], data1[i]);
                }
        }

	buffer.switch_buffer(data2.data(), data2.size());
        {
                std::vector<int16_t> const &current {buffer.get_current()};
                std::vector<int16_t> const &previous {buffer.get_previous()};
                for (int i {0}; i < (int) data2.size(); ++i) {
                        CPPUNIT_ASSERT_EQUAL(current[i], data2[i]);
                        CPPUNIT_ASSERT_EQUAL(previous[i], data1[i]);
                }
        }

	buffer.switch_buffer(data3.data(), data3.size());
        {
                std::vector<int16_t> const &current {buffer.get_current()};
                std::vector<int16_t> const &previous {buffer.get_previous()};
                for (int i {0}; i < (int) data2.size(); ++i) {
                        CPPUNIT_ASSERT_EQUAL(current[i], data3[i]);
                        CPPUNIT_ASSERT_EQUAL(previous[i], data2[i]);
                }
        }

        buffer.switch_buffer();
        {
                std::vector<int16_t> const &previous {buffer.get_previous()};
                for (int i {0}; i < (int) data2.size(); ++i) {
                        CPPUNIT_ASSERT_EQUAL(previous[i], data3[i]);
                }
        }
}

void CircularBufferTestCase::test_size() {
        CircularBuffer<int16_t> buffer;

        buffer.switch_buffer(data1.data(), data1.size());
        CPPUNIT_ASSERT_EQUAL(buffer.size(), data1.size());

        buffer.switch_buffer(data2.data(), data2.size());
        CPPUNIT_ASSERT_EQUAL(buffer.size(), data2.size());

        buffer.switch_buffer();
        CPPUNIT_ASSERT(buffer.size() == 0);
}
