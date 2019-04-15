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
	 * Returns the value at the specified index.
	 *
	 * If the index is higher or equal to 0, the value will be taken
	 * from the current buffer.  Otherwise, it will be taken from the
	 * previous buffer, from its end (ie. index -1 will be the last
	 * element of the previous buffer).
	 *
	 * @param index Index of the element to return.
	 * @returns A constant reference to the element.
	 */
	const T &operator[](int index) const {
		if (index < 0) {
			// index is negative, so it must be added, not substracted.
			return previous[previous.size() + index];
		} else {
			return current[index];
		}
	}

	/**
	 * Switch the current buffer, and empties the current buffer.
	 */
	void switch_buffer() {
		switch_buffer(nullptr, 0);
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

		std::copy_n(new_values, count, current.begin());
	}

	/**
	 * Insert a value at the end of the current buffer.
	 *
	 * @param new_value The value to insert in the current buffer.
	 */
	void push_back(T new_value) {
		current.push_back(new_value);
	}

	/**
	 * Copy a part of the buffer to dst.  The range filled is [ifrom,
	 * ito).  If some parts that should be copied are out of bound, they
	 * will be filled with empty elements.
	 *
	 * @param ifrom The element to copy from.  It is included in the
	 * copy.
	 * @param ito The element to copy to.  It is not included in the
	 * copy.
	 * @param dst The destination buffer.
	 */
	void copy_buffer(int ifrom, int ito, std::vector<T> &dst) const {
		const T default_ {};

		dst.clear();
		dst.reserve(ito - ifrom); // Allocate the whole destination at once

		for (int i = ifrom; i < ito; ++i) {
			if ((i < 0 && -i >= previous.size()) ||
			    (i >= 0 && i >= current.size())) {
				dst.push_back(default_);
			} else {
				// might be optimised with std::copy()?
				dst.push_back(this[i]);
			}
		}
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
