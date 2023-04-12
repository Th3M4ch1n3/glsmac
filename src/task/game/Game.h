#pragma once

// TODO: merge this class with Map?

#include <unordered_set>

#include "base/Task.h"

#include "../Settings.h"

#include "scene/Scene.h"
#include "scene/actor/Instanced.h"

#include "util/Clamper.h"
#include "util/Random.h"
#include "util/Scroller.h"

#include "map/Map.h"
#include "map_editor/MapEditor.h"

#include "../../ui/UI.h"
#include "ui/event/UIEventHandler.h"
#include "ui/style/Theme.h"
#include "ui/bottom_bar/BottomBar.h"

#include "actor/TileSelection.h"

using namespace ui;
namespace ui {
	using namespace event;
}

using namespace util;
using namespace scene;

namespace task {
namespace game {
	
using namespace map;

CLASS( Game, base::Task )
	
	Game( Settings& settings );
	~Game();
	
	void Start();
	void Stop();
	void Iterate();

	struct consts_t {
		const struct {
			const struct {
				const float zoom_speed = 0.3f;
			} smooth_scrolling;
			struct {
				struct {
					const size_t fullscreen = 1;
					const size_t windowed = 10; // harder to place cursor at edge of window
				} edge_distance_px;
				const uint16_t scroll_step_ms = 2;
				const Vec2< float > speed = {
					0.002f * scroll_step_ms,
					0.003f * scroll_step_ms,
				};
			} static_scrolling;
			struct {
				const float scroll_if_selected_tile_farther_from_center_than = 0.4f;
			} key_scrolling;
		} map_scroll;
		const struct {
			const size_t draw_frequency_ms = 60; // TODO: this value doesn't seem realistic, why?
		} map_editing;
	};
	static const consts_t s_consts;
	
	Map* GetMap() const;
	
	map_editor::MapEditor* GetMapEditor() const;
	
	void CenterAtCoordinatePercents( const Vec2< float > position_percents );
	
	void SmoothScroll( const float scroll_value );
	void SmoothScroll( const Vec2< float > position, const float scroll_value );

	util::Random* GetRandom() const;
	
	void CloseMenus();
	void ReturnToMainMenu();
	
	const size_t GetBottomBarMiddleHeight() const;
	const size_t GetViewportHeight() const;
	
	void AddMessage( const std::string& text );
	
	void GenerateMap();
	void LoadMap( const std::string& path );
	void SaveMap( const std::string& path );
	
	void ConfirmExit( ::ui::ui_handler_t on_confirm );
	
protected:

private:
	
	void SetCameraPosition( const Vec3 camera_position );
	
	// game settings (tmp)
	Settings m_settings;
	
	// seed needs to be consistent during session (to prevent save-scumming and for easier reproducing of bugs)
	Random* m_random = nullptr;
	
	// map rendering stuff
	Map* m_map = nullptr;
	Vec3 m_camera_position; // { x, y, zoom }
	Vec3 m_camera_angle;
	Camera* m_camera = nullptr;
	Light* m_light_a = nullptr; // Alpha Centauri A
	Light* m_light_b = nullptr; // Alpha Centauri B
	Scene* m_world_scene = nullptr;
	
	map_editor::MapEditor* m_map_editor = nullptr;
	bool m_is_editing_mode = false;
	map_editor::MapEditor::draw_mode_t m_editing_draw_mode = map_editor::MapEditor::DM_NONE;
	util::Timer m_editing_draw_timer;
	
	struct {
		util::Clamper<float> x;
		util::Clamper<float> y;
	} m_clamp = {};
	
	util::Scroller< types::Vec3 > m_scroller;
	
	struct {
		bool is_dragging = false;
		Vec2< float > last_drag_position;
		struct {
			util::Timer timer;
			Vec2< float > speed = { 0, 0 };
		} edge_scrolling;
		Vec2< float > last_mouse_position;
		float key_zooming = 0;
	} m_map_control = {};
	
	Map::tile_info_t m_selected_tile_info = {};
	
	struct {
		size_t window_width;
		size_t window_height;
		Vec2< size_t > min;
		Vec2< size_t > max;
		size_t bottom_bar_overlap;
		Vec2< float > ratio;
		size_t width;
		size_t height;
		float aspect_ratio;
		float window_aspect_ratio;
		bool is_fullscreen;
	} m_viewport;
	struct {
		Vec3 min;
		Vec3 max;
	} m_camera_range;
	
	// shift x to center instance when needed
	const float GetFixedX( float x ) const;
	void FixCameraX();
	
	struct {
		const UIEventHandler* keydown_before;
		const UIEventHandler* keydown_after;
		const UIEventHandler* keyup;
		const UIEventHandler* mousedown;
		const UIEventHandler* mousemove;
		const UIEventHandler* mouseup;
		const UIEventHandler* mousescroll;
	} m_handlers;
	void UpdateViewport();
	void UpdateCameraPosition();
	void UpdateCameraAngle();
	void UpdateCameraScale();
	void UpdateCameraRange();
	void UpdateMapInstances();
	void UpdateUICamera();
	
	// UI stuff
	
	struct {
		ui::style::Theme theme;
		ui::BottomBar* bottom_bar = nullptr;
	} m_ui;
	
	// tiles stuff
	void SelectTileAtPoint( const size_t x, const size_t y );
	void SelectTile( const Map::tile_info_t& tile_info );
	void DeselectTile();
	
	struct {
		actor::TileSelection* tile_selection = nullptr;
	} m_actors;
	
	// some additional management of world actors such as calling Iterate()
	// note that all world actors must be instanced
	std::unordered_map< actor::Actor*, scene::actor::Instanced* > m_actors_map = {};
	
	void AddActor( actor::Actor* actor );
	void RemoveActor( actor::Actor* actor );
	
	const Vec2< float > GetTileWindowCoordinates( const Map::tile_state_t* ts );
	
	void ScrollTo( const Vec3& target );
	void ScrollToTile( const Map::tile_state_t* ts );
	void CenterAtTile( const Map::tile_state_t* ts );
	
	void UpdateMinimap();
	
	void ResetMapState();
};

}
}
