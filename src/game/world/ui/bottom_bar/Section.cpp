#include <regex>

#include "Section.h"

#include "engine/Engine.h"

namespace game {
namespace world {
namespace ui {

void Section::Create() {
	UI::Create();
	
	NEW( m_outer, object::Section, "MapBottomBarSectionOuter" );
	UI::AddChild( m_outer );
	
	NEW( m_inner, object::Section, "MapBottomBarSectionInner" );
	m_outer->AddChild( m_inner );
	
}

void Section::Destroy() {
	
	m_outer->RemoveChild( m_inner );
	UI::RemoveChild( m_outer );
	
	UI::Destroy();
}

void Section::AddChild( UIObject *object ) {
	m_inner->AddChild( object );
}

void Section::RemoveChild( UIObject *object ) {
	m_inner->RemoveChild( object );
}

}
}
}
