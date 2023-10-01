#include <vector>

#include "types/Serializable.h"
#include "Slot.h"

namespace game {

CLASS( Slots, types::Serializable )

	const size_t GetCount() const;
	void Resize( const size_t size );
	Slot& GetSlot( const size_t index );
	std::vector< Slot >& GetSlots();
	void Clear();

	const types::Buffer Serialize() const override;
	void Unserialize( types::Buffer buf ) override;

private:
	std::vector< Slot > m_slots = {};

};

}
