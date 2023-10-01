#pragma once

#include "scene/Scene.h"
#include "scene/actor/Actor.h"
#include "../Scene.h"
#include "../actor/Actor.h"

#include "base/Module.h"

namespace graphics {
namespace opengl {

class OpenGL;

namespace routine {

CLASS( Routine, base::Module )

	Routine( OpenGL* opengl );
	~Routine();

	bool AddScene( scene::Scene* scene );
	bool RemoveScene( scene::Scene* scene );

	virtual opengl::Actor* AddCustomActor( scene::actor::Actor* actor ) {
		return NULL;
	}
	virtual void OnSceneRemove( Scene* scene ) {};

	void OnWindowResize();

protected:
	friend class opengl::OpenGL;

	OpenGL* m_opengl = nullptr;

	std::vector< scene::Scene* > m_scenes = {};
	std::vector< opengl::Scene* > m_gl_scenes = {};
	virtual bool SceneBelongs( const scene::Scene* scene ) const = 0;
};

} /* namespace routine */
} /* namespace opengl */
} /* namespace graphics */

#include "../shader_program/ShaderProgram.h"
#include "../OpenGL.h"