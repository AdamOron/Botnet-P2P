#pragma once
#include <iostream>

struct SocketArray
{
private:
	SOCKET m_Sockets[FD_SETSIZE];
	size_t m_Size;

public:
	SocketArray() :
		m_Sockets(),
		m_Size(0)
	{
	}

	SOCKET Get(size_t index) const
	{
		return m_Sockets[index];
	}

	SOCKET operator[](size_t index) const
	{
		return Get(index);
	}

	size_t Size() const
	{
		return m_Size;
	}

	size_t Add(SOCKET socket)
	{
		m_Sockets[m_Size] = socket;
		return m_Size++;
	}

	void Remove(size_t index)
	{
		for (size_t i = index; i < m_Size - 1; i++)
			m_Sockets[i] = m_Sockets[i + 1];

		m_Sockets[--m_Size] = INVALID_SOCKET;
	}
};
