#include "Image.h"

#include "scene/actor/Image.h"
#include "../shader_program/Orthographic.h"
#include "../shader_program/World.h"

#include "types/Matrix44.h"
#include "scene/actor/Image.h"
#include "engine/Engine.h"

namespace graphics {
namespace opengl {

Image::Image( scene::actor::Image *actor ) : Actor( actor ) {

	//Log( "Creating OpenGL actor" );

	ASSERT( false, "deprecated" );
	
	glGenBuffers( 1, &m_vbo );
	glGenBuffers( 1, &m_ibo );
	
	//m_update_timestamp = actor->GetImage()->UpdatedAt();
}

Image::~Image() {
	//Log( "Destroying OpenGL actor" );

	glDeleteBuffers( 1, &m_ibo );
	glDeleteBuffers( 1, &m_vbo );

}

void Image::LoadTexture() {
	// ???
}

void Image::LoadMesh() {
	Log( "Loading OpenGL actor" );
	
	
/*
	scene::actor::Mesh *mesh_actor = (scene::actor::Mesh *)m_actor;

	scene::mesh::Mesh *mesh = mesh_actor->GetMesh();
*/

	auto *actor = (scene::actor::Image *)m_actor;
	
	Log( "image width=" + std::to_string( actor->GetImage()->m_width ) + " height=" + std::to_string( actor->GetImage()->m_height ) );
	Log( "viewport width=" + std::to_string( g_engine->GetGraphics()->GetViewportWidth() ) + " height=" + std::to_string( g_engine->GetGraphics()->GetViewportHeight() ) );
	
	// width and height scaled to viewport (to have same absolute pixel size always)
	float scaled_w = (float) actor->GetImage()->m_width / g_engine->GetGraphics()->GetViewportWidth();
	float scaled_h = (float) actor->GetImage()->m_height / g_engine->GetGraphics()->GetViewportHeight();
	
	m_vertex_data = { -scaled_w, -scaled_h, scaled_w, -scaled_h, scaled_w, scaled_h, -scaled_w, scaled_h };
	m_index_data = { 0, 1, 2, 3 };

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBufferData( GL_ARRAY_BUFFER, m_vertex_data.size() * sizeof(float), (GLvoid *)m_vertex_data.data(), GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_index_data.size() * sizeof(unsigned int), (GLvoid *)m_index_data.data(), GL_STATIC_DRAW);

	m_ibo_size = m_index_data.size();

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
 
}

void Image::Draw( shader_program::ShaderProgram *shader_program, Camera *camera ) {

	//Log( "Drawing" );

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );

	shader_program->Enable();

	switch ( shader_program->GetType() ) {
		case ( shader_program::ShaderProgram::TYPE_ORTHO ): {
			auto *ortho_shader_program = (shader_program::Orthographic *)shader_program;
			//glUniform1f( ortho_shader_program->uniforms.z_index, m_actor->GetPosition().z );
			//types::Color tint_color = ((scene::actor::Mesh *)m_actor)->GetTintColor();
			/*types::Color tint_color = types::Color::WHITE();
			const GLfloat tint_color_data[4] = { tint_color.red, tint_color.green, tint_color.blue, tint_color.alpha };
			glUniform4fv( ortho_shader_program->uniforms.tint, 1, tint_color_data );*/
			break;
		}
/*		case ( shader_program::ShaderProgram::TYPE_PERSP ): {
			auto *persp_shader_program = (shader_program::World *)shader_program;

			types::Matrix44 matrix = m_actor->GetWorldMatrix();
			glUniformMatrix4fv( persp_shader_program->uniforms.world, 1, GL_TRUE, (const GLfloat*)(&matrix));

			glUniform3f( persp_shader_program->uniforms.light_color, 1.0, 1.0, 1.0 );
			glUniform1f( persp_shader_program->uniforms.light_intensity, 1.0 );

		    //glUniform3f( persp_shader_program->uniforms.campos, 0.0, 0.0, 0.0 );

			break;

		}*/
		default: {
			ASSERT( false, "unknown shader program " + std::to_string( shader_program->GetType() ) );
		}
	}

	/*
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(MyVertex), BUFFER_OFFSET(12));   //The starting point of normals, 12 bytes away
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(MyVertex), BUFFER_OFFSET(24));   //The starting point of texcoords, 24 bytes away
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );
*/

	//glEnableClientState( GL_VERTEX_ARRAY );
	//glVertexPointer( 2, GL_FLOAT, 0, (void *) 0 );
	// enable shader program?

	//glBindTexture(GL_TEXTURE_2D, mMaterialTextureObjs[0]);
	//glActiveTexture(GL_TEXTURE0);

/*
	math::Matrix44 matrix;

	matrix.Identity();
	matrix.ProjectionOrtho2D( 0.01f, 100.0f );

	glUniformMatrix4fv( m_shader_program->uniforms.position, 1, GL_TRUE, (const GLfloat *) &matrix );*/

	//math::Vec2<> matrix( 0.0f, 0.0f );
	//glUniformMatrix2fv( m_shader_program->uniforms.position, 1, GL_TRUE, (const GLfloat *) &matrix );

	//glUniformMatrix4fv(shader_program->mUWorld, 1, GL_TRUE, (const GLfloat*)(&mActorFinalMatrices[i]));

	glDrawElements( GL_QUADS, m_ibo_size, GL_UNSIGNED_INT, (void *)(0) );

	shader_program->Disable();

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

} /* namespace opengl */
} /* namespace graphics */
