#ifndef RING_H
#define RING_H

#include <cstdlib>

template <typename T>
class ring {
public:
	ring(std::size_t capacity)
		: m_capacity(capacity)
		, m_size(0)
		, m_offset(0)
	{
		m_data = new T[capacity];
	}
	ring(const ring&) = delete;
	ring(ring&&) = delete;
	ring& operator=(const ring&) = delete;
	ring& operator=(ring&&) = delete;

	~ring()
	{
		delete[] m_data;
	}

	auto capacity() { return m_capacity; }
	auto size() { return m_size; }
	T& operator[](std::size_t index)
	{
		while (index < 0)
			index += m_size;
		return m_data[(m_offset + index) % m_size];
	}

	void push(const T& value)
	{
		if (m_capacity == m_size) {
			m_data[m_offset] = value;
			m_offset = (m_offset + 1) % m_size;
		} else {
			m_data[m_size++] = value;
		}
	}

private:
	std::size_t const m_capacity;
	std::size_t m_size;
	std::size_t m_offset;
	T* m_data;
};

#endif
