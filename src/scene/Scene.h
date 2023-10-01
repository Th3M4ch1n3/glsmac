#pragma once

#include <vector>
#include <unordered_set>

#include "base/Base.h"

#include "actor/Actor.h"
#include "Camera.h"
#include "Light.h"
#include "types/Texture.h"
#include "types/Vec3.h"

namespace scene {

enum scene_type_t {
	SCENE_TYPE_NONE,
	SCENE_TYPE_SIMPLE2D,
	SCENE_TYPE_ORTHO,
	SCENE_TYPE_ORTHO_UI, // same as ortho, but is drawn after (and on top of) overlay
	SCENE_TYPE_PERSP,
};

CLASS( Scene, base::Base )
	Scene( const std::string& name, const scene_type_t scene_type )
		: m_scene_type( scene_type ) {
		m_name = name;
	}
	~Scene();
	void AddActor( actor::Actor* actor );
	void RemoveActor( actor::Actor* actor );
	std::vector< actor::Actor* >* GetActors() {
		return &m_actors;
	}
	const scene_type_t GetType() const {
		return m_scene_type;
	}
	void SetCamera( Camera* camera );
	Camera* GetCamera() const;
	void AddLight( Light* light );
	std::unordered_set< Light* >* GetLights();
	void SetSkyboxTexture( types::Texture* skybox_texture );
	types::Texture* GetSkyboxTexture();

	typedef std::vector< Vec3 > instance_positions_t;
	void SetWorldInstancePositions( const instance_positions_t& world_instance_positions );
	const instance_positions_t& GetWorldInstancePositions() const;

protected:
	friend class scene::Camera;

	Camera* m_local_camera = nullptr;

	const scene_type_t m_scene_type = SCENE_TYPE_NONE;

	std::vector< actor::Actor* > m_actors = {};

	Camera* m_camera = nullptr;
	std::unordered_set< Light* > m_lights = {};
	types::Texture* m_skybox_texture = nullptr;

	instance_positions_t m_game_instance_positions = {
		{
			0.0f,
			0.0f,
			0.0f
		}
	}; // only 1 instance by default
};

} /* namespace scene */
