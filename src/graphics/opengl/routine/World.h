#pragma once

#include "Routine.h"

#include "../shader_program/Orthographic.h"
#include "../shader_program/OrthographicData.h"

#include "scene/Scene.h"

namespace graphics {
namespace opengl {
namespace routine {

CLASS( World, Routine )
	World(
		OpenGL* opengl,
		const scene::scene_type_t scene_type,
		shader_program::Orthographic* shader_program,
		shader_program::OrthographicData* data_shader_program
	);

	bool SceneBelongs( const scene::Scene* scene ) const override;

	void Start() override;
	void Stop() override;
	void Iterate() override;

protected:

	const scene::scene_type_t m_scene_type;

	shader_program::Orthographic* m_shader_program;
	shader_program::OrthographicData* m_data_shader_program;
};

} /* namespace routine */
} /* namespace opengl */
} /* namespace graphics */
