#pragma once
#include <cstdint>
#include "Peer.h"

#define BYTES(value) reinterpret_cast<char *>(&value)

namespace PeerProtocol
{
#define MAX_PEERS size_t(32)

	enum class Type : uint16_t
	{
		PEER_HELLO,
		PEER_JOIN,
		PEER_WELCOME,
		PEER_ACCEPT,
	};

	struct HEADER
	{
		Type m_Type : 15;
		bool m_IsFwd : 1;
		guid_t m_SenderGuid : 32;

		HEADER(guid_t senderGuid, Type type, bool isFwd) :
			m_SenderGuid(senderGuid),
			m_Type(type),
			m_IsFwd(isFwd)
		{
		}
	};

	struct MSG
	{
		HEADER m_Header;

		MSG(guid_t senderGuid, Type type, bool isFwd) :
			m_Header(senderGuid, type, isFwd)
		{
		}

		MSG(guid_t senderGuid, Type type) :
			MSG(senderGuid, type, false)
		{
		}
	};

#define GUID_LIST_CAPACITY size_t(32)

	struct GUID_LIST
	{
		size_t Size;
		guid_t Guids[GUID_LIST_CAPACITY];

		GUID_LIST() :
			Size(0),
			Guids()
		{
		}

		void operator+=(const guid_t &guid)
		{
			if (Size < GUID_LIST_CAPACITY)
				Guids[Size++] = guid;
		}

		guid_t At(size_t i) const
		{
			return Guids[i];
		}

		guid_t operator[](size_t i) const
		{
			return At(i);
		}
	};

	struct FWD_MSG : public MSG
	{
		GUID_LIST m_RecvdGuids;

		FWD_MSG(guid_t senderGuid, Type type, GUID_LIST *recvdGuids) :
			MSG(senderGuid, type, true),
			m_RecvdGuids()
		{
			if (!recvdGuids)
				return;

			for (size_t i = 0; i < recvdGuids->Size; i++)
				m_RecvdGuids += recvdGuids->At(i);
		}
	};

	struct HELLO_MSG : public MSG
	{
		const char *m_Host;
		const char *m_Port;

		HELLO_MSG(const char *host, const char *port) :
			MSG(NULL, Type::PEER_HELLO, true),
			m_Host(host),
			m_Port(port)
		{
		}
	};

	struct NOTIFY_JOIN_MSG : public FWD_MSG
	{
		guid_t m_JoinedGuid;
		const char *m_JoinedHost;
		const char *m_JoinedPort;

		NOTIFY_JOIN_MSG(guid_t senderGuid, GUID_LIST *recvdGuids, guid_t joinedGuid, const char *joinedHost, const char *joinedPort) :
			FWD_MSG(senderGuid, Type::PEER_JOIN, recvdGuids),
			m_JoinedGuid(joinedGuid),
			m_JoinedHost(joinedHost),
			m_JoinedPort(joinedPort)
		{
		}
	};

	struct ACCEPT_MSG : public MSG
	{
		guid_t m_NewGuid;

		ACCEPT_MSG(guid_t senderGuid, guid_t newGuid) :
			MSG(senderGuid, Type::PEER_ACCEPT),
			m_NewGuid(newGuid)
		{
		}
	};
}
