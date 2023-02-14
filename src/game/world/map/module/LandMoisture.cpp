#include "LandMoisture.h"

namespace game {
namespace world {
namespace map {

void LandMoisture::GenerateTile( const Tile* tile, Map::tile_state_t* ts, Map::map_state_t* ms ) {
	
	const auto w = Map::s_consts.pcx_texture_block.dimensions.x;
	const auto h = Map::s_consts.pcx_texture_block.dimensions.y;
	
	Map::pcx_texture_coordinates_t tc = {};
	uint8_t rotate = 0;
	
	NEW( ts->moisture_original, Texture, "MoistureOriginal", w, h );

	switch ( tile->moisture ) {
		case Tile::M_NONE: {
			// invisible tile (for dev/test purposes)
			break;
		}
		case Tile::M_ARID: {
			tc = Map::s_consts.pcx_textures.arid[ 0 ];
			rotate = RandomRotate();
			break;
		}
		case Tile::M_MOIST: {
			auto txinfo = m_map->GetTileTextureInfo( tile, Map::TG_MOISTURE );
			tc = Map::s_consts.pcx_textures.moist[ txinfo.texture_variant ];
			rotate = txinfo.rotate_direction;
			break;
		}
		case Tile::M_RAINY: {
			auto txinfo = m_map->GetTileTextureInfo( tile, Map::TG_MOISTURE );
			tc = Map::s_consts.pcx_textures.rainy[ txinfo.texture_variant ];
			rotate = txinfo.rotate_direction;
			break;
		}
		default:
			ASSERT( false, "invalid moisture value" );
	}
	
	m_map->GetTexture( ts->moisture_original, tc, Texture::AM_DEFAULT, rotate );
}

}
}
}
