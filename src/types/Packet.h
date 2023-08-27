#pragma once

#include <vector>

#include "Serializable.h"

namespace types {

CLASS( Packet, Serializable )
	
	enum packet_type_t {
		PT_NONE, // -
		PT_REQUEST_AUTH, // S->C
		PT_AUTH, // C->S
		PT_PING, // *->*
		PT_PONG, // *->*
		PT_GLOBAL_SETTINGS, // S->C
		PT_PLAYERS, // S->C
		PT_KICK, // S->C
	};

	packet_type_t type;
	
	union {
		time_t time;
		struct {
			size_t id;
		} ping;
	} udata;
	
	struct {
		size_t num;
		std::string str;
		std::vector< std::string > vec;
	} data;

	const Buffer Serialize() const;
	void Unserialize( Buffer buffer );
};

}
