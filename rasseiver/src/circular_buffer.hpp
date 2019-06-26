#ifndef __ILSIMU_RASSEIVER_CIRCULAR_BUFFER_HPP
# define __ILSIMU_RASSEIVER_CIRCULAR_BUFFER_HPP

# include <algorithm>
# include <vector>

/**
 * A generic circular buffer structure.  It internally uses two
 * std::vector to store values.
 *
 * One may access to an element part of the current buffer with an
 * index higher or equal to 0, and to an element part of the previous
 * buffer with an index lower than 0.
 *
 * The buffers may be switched with switch_buffer().
 */
template<typename T>
class CircularBuffer {
public:
	/**
	 * Default constructor.  Does not insert any value in the buffer.
	 */
	CircularBuffer() = default;

	/**
	 * Creates a circular buffer with a specified size of count.
	 *
	 * This avoids allocating memory during the processing of the buffer,
	 * which could be as reliable and fast as possible.  Dynamic memory
	 * allocation takes time and can fail on some systems.
	 *
	 * Doing this at initialisation allows the software to crash at startup,
	 * and even if annoying, it's infinitely less than in the middle of a
	 * work session.
	 *
	 * @param count The amount of memory to allocate in each buffer.
	 */
	CircularBuffer(size_t count): previous (count), current (count) {
	}

	// No need for the copy constructor and the assignment operator.
	CircularBuffer(CircularBuffer const &) = delete;
	CircularBuffer &operator=(CircularBuffer const &) = delete;

	/**
	 * Returns a reference to the previous buffer.
	 */
	std::vector<T> const &get_previous() const {
		return previous;
	}

	/**
	 * Returns a reference to the current buffer.
	 */
	std::vector<T> const &get_current() const {
		return current;
	}

	/**
	 * Switch the current buffer, and set the contents of the current
	 * buffer.
	 *
	 * The content of the current buffer and previous buffer are
	 * exchanged with std::swap(), then the content of the current
	 * buffer is overwritten with new_values.
	 *
	 * @param new_values The values to insert in the current buffer.
	 * @param count The amount of values to insert in the buffer.
	 */
	void switch_buffer(T *new_values, size_t count) {
		// std::move() may free current's table, so instead they are
		// swapped, and current's elements are overwritten.
		std::swap(previous, current);

		current.clear();
		current.reserve(count);

		std::copy_n(new_values, count, std::back_inserter(current));
	}

	/**
	 * Return the size of the current buffer.
	 */
	size_t size() const {
		return current.size();
	}

private:
	std::vector<T> previous, current;
};

#endif  /* __ILSIMU_RASSEIVER_CIRCULAR_BUFFER_HPP */
