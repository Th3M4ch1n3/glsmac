#include <cmath>
#include <cstring>

#include "Render.h"

#include "util/Math.h"

using namespace util;

namespace types {
namespace mesh {

Render::Render( const size_t vertex_count, const size_t surface_count )
	: Mesh( MT_RENDER, VERTEX_SIZE, vertex_count, surface_count )
{
	
}

Render::index_t Render::AddVertex( const Vec3 &coord, const Vec2<coord_t> &tex_coord, const Color tint, const Vec3 &normal ) {
	ASSERT( !m_is_final, "addvertex on already finalized mesh" );
	ASSERT( m_vertex_i < m_vertex_count, "vertex out of bounds (" + std::to_string( m_vertex_i ) + " >= " + std::to_string( m_vertex_count ) + ")" );
	size_t offset = m_vertex_i * VERTEX_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( coord ) ), &coord, sizeof(coord) );
	offset += VERTEX_COORD_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( tex_coord ) ), &tex_coord, sizeof( tex_coord ) );
	offset += VERTEX_TEXCOORD_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( tint ) ), &tint, sizeof( tint ) );
	offset += VERTEX_TINT_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( normal ) ), &normal, sizeof( normal ) );
	index_t ret = m_vertex_i;
	m_vertex_i++;
	return ret;
}
Render::index_t Render::AddVertex( const Vec2<coord_t> &coord, const Vec2<coord_t> &tex_coord, const Color tint, const Vec3 &normal ) {
	return AddVertex( Vec3( coord.x, coord.y, 0.0f ), tex_coord, tint, normal );
}

void Render::SetVertex( const index_t index, const Vec3 &coord, const Vec2<coord_t> &tex_coord, const Color tint, const Vec3 &normal ) {
	ASSERT( index < m_vertex_count, "index out of bounds" );
	size_t offset = index * VERTEX_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( coord ) ), &coord, sizeof( coord ) );
	offset += VERTEX_COORD_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( tex_coord ) ), &tex_coord, sizeof( tex_coord ) );
	offset += VERTEX_TEXCOORD_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( tint ) ), &tint, sizeof( tint ) );
	offset += VERTEX_TINT_SIZE * sizeof( coord_t );
	memcpy( ptr( m_vertex_data, offset, sizeof( normal ) ), &normal, sizeof( normal ) );
	Update();
}
void Render::SetVertex( const index_t index, const Vec2<coord_t> &coord, const Vec2<coord_t> &tex_coord, const Color tint, const Vec3 &normal ) {
	SetVertex( index, { coord.x, coord.y, 0.0f }, tex_coord, tint, normal );
}

void Render::SetVertexTexCoord( const index_t index, const Vec2<coord_t> &tex_coord ) {
	ASSERT( index < m_vertex_count, "index out of bounds" );
	memcpy( ptr( m_vertex_data, index * VERTEX_SIZE * sizeof( coord_t ) + VERTEX_COORD_SIZE * sizeof( coord_t ), sizeof( tex_coord ) ), &tex_coord, sizeof( tex_coord ) );
	Update();
}

void Render::SetVertexTint( const index_t index, const Color tint ) {
	ASSERT( index < m_vertex_count, "index out of bounds" );
	memcpy( ptr( m_vertex_data, index * VERTEX_SIZE * sizeof( coord_t ) + ( VERTEX_COORD_SIZE + VERTEX_TEXCOORD_SIZE ) * sizeof( coord_t ), sizeof( Color ) ), &tint, sizeof( tint ) );
}

void Render::SetVertexNormal( const index_t index, const Vec3& normal ) {
	ASSERT( index < m_vertex_count, "index out of bounds" );
	memcpy( ptr( m_vertex_data, index * VERTEX_SIZE * sizeof( coord_t ) + ( VERTEX_COORD_SIZE + VERTEX_TEXCOORD_SIZE + VERTEX_TINT_SIZE ) * sizeof( coord_t ), sizeof( normal ) ), &normal, sizeof( normal ) );
}

void Render::GetVertexTexCoord( const index_t index, Vec2<coord_t>* coord ) const {
	ASSERT( index < m_vertex_count, "index out of bounds" );
	memcpy( coord, ptr( m_vertex_data, index * VERTEX_SIZE * sizeof( coord_t ) + VERTEX_COORD_SIZE * sizeof( coord_t ), sizeof( Vec2<coord_t> ) ), sizeof( Vec2<coord_t> ) );
}

const Vec3 Render::GetVertexNormal( const index_t index ) const {
	ASSERT( index < m_vertex_count, "index out of bounds" );
	Vec3 normal;
	memcpy( &normal, ptr( m_vertex_data, index * VERTEX_SIZE * sizeof( coord_t ) + ( VERTEX_COORD_SIZE + VERTEX_TEXCOORD_SIZE + VERTEX_TINT_SIZE ) * sizeof( coord_t ), sizeof( normal ) ), sizeof( normal ) );
	return normal;
}

void Render::Finalize() {
	Mesh::Finalize();
	
	UpdateNormals();
}

void Render::UpdateNormals() {
	//Log( "Updating normals");
	
	const surface_t* surface;
	Vec3 *a, *b, *c;
	Vec3 ab, ac, n;
	const size_t vo = VERTEX_COORD_SIZE + VERTEX_TEXCOORD_SIZE + VERTEX_TINT_SIZE;
	
	for ( size_t v = 0 ; v < m_vertex_i ; v++ ) {
		memset( ptr( m_vertex_data, ( v * VERTEX_SIZE + vo ) * sizeof( coord_t ), sizeof( Vec3 ) ), 0, sizeof( Vec3 ) );
	}
	for ( size_t s = 0 ; s < m_surface_i ; s++ ) {
		surface = (surface_t*)ptr( m_index_data, s * SURFACE_SIZE * sizeof( index_t ), sizeof( surface_t ) );
		a = (Vec3*)ptr( m_vertex_data, surface->v1 * VERTEX_SIZE * sizeof( coord_t ), sizeof( coord_t ) );
		b = (Vec3*)ptr( m_vertex_data, surface->v2 * VERTEX_SIZE * sizeof( coord_t ), sizeof( coord_t ) );
		c = (Vec3*)ptr( m_vertex_data, surface->v3 * VERTEX_SIZE * sizeof( coord_t ), sizeof( coord_t ) );
        ab = *b - *a;
        ac = *c - *a;
        n = Math::Cross( ab, ac );
		*(Vec3*)ptr( m_vertex_data, ( surface->v1 * VERTEX_SIZE + vo ) * sizeof( coord_t ), sizeof( Vec3 ) ) += n;
		*(Vec3*)ptr( m_vertex_data, ( surface->v2 * VERTEX_SIZE + vo ) * sizeof( coord_t ), sizeof( Vec3 ) ) += n;
		*(Vec3*)ptr( m_vertex_data, ( surface->v3 * VERTEX_SIZE + vo ) * sizeof( coord_t ), sizeof( Vec3 ) ) += n;
	}
	for ( size_t v = 0 ; v < m_vertex_i ; v++ ) {
		*(Vec3*)ptr( m_vertex_data, ( v * VERTEX_SIZE + vo ) * sizeof( coord_t ), sizeof( Vec3 ) )
			=
		Math::Normalize( *(Vec3*)ptr( m_vertex_data, ( v * VERTEX_SIZE + vo ) * sizeof( coord_t ), sizeof( Vec3 ) ) );
	}
	
	Update();
}

}
}
