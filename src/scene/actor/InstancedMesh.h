#pragma once

#include <vector>

#include "Mesh.h"

#include "types/Matrix44.h"
#include "scene/Scene.h"

namespace scene {
namespace actor {

CLASS( InstancedMesh, Mesh )
	InstancedMesh( const std::string &name, const types::mesh::Render *mesh = nullptr );
	~InstancedMesh();
	
	typedef std::vector< types::Matrix44 > world_matrices_t;
	const world_matrices_t& GetWorldMatrices();
	types::Matrix44 & GetWorldMatrix();
	
	void UpdateWorldMatrix();
	void UpdatePosition();
	void UpdateMatrix();
	
protected:
	
	world_matrices_t m_world_matrices = {};
	
	struct instanced_matrices_t {
		types::Matrix44 translate;
		types::Matrix44 matrix;
		types::Matrix44 world;
	};
	
	std::vector< instanced_matrices_t > m_instance_matrices = {};
	
private:
	const scene::Scene::instances_t* GetInstances();
};

}
}
