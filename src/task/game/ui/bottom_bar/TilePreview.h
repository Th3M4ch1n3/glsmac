#pragma once

#include "BBSection.h"

#include "ui/object/Mesh.h"
#include "ui/object/Label.h"

#include "../../map/Map.h"

namespace task {
namespace game {
	using namespace map;
namespace ui {

CLASS( TilePreview, BBSection )

	TilePreview( Game* game ) : BBSection( game, "TilePreview" ) {}
	
	void Create();
	void Destroy();

	void PreviewTile( const Map* map, const Map::tile_info_t& tile_info );
	void HideTilePreview();
	
private:

	struct preview_layer_t {
		object::Mesh* object;
		types::Texture* texture;
	};
	std::vector< preview_layer_t > m_preview_layers = {}; // multiple layers of textures
	
	// TODO: multiline labels?
	std::vector< Label* > m_info_lines = {};
	
};
	
}
}
}
