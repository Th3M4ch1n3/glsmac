#include "UI.h"

#include "engine/Engine.h"

#include "event/MouseMove.h"

using namespace scene;
using namespace types;

namespace ui {

using namespace object;

void UI::Start() {
	Log( "Creating UI" );

	NEW( m_shape_scene_simple2d, Scene, "UIScene::Simple2D", SCENE_TYPE_SIMPLE2D );
	g_engine->GetGraphics()->AddScene( m_shape_scene_simple2d );
	NEW( m_shape_scene_ortho, Scene, "UIScene::Ortho", SCENE_TYPE_ORTHO_UI );
	g_engine->GetGraphics()->AddScene( m_shape_scene_ortho );
	
	NEW( m_text_scene, Scene, "UIText", SCENE_TYPE_TEXT );
	g_engine->GetGraphics()->AddScene( m_text_scene );

	m_root_object.Create();
	m_root_object.UpdateObjectArea();

	m_clamp.x.SetRange( 0.0, g_engine->GetGraphics()->GetViewportWidth(), -1.0, 1.0 );
	m_clamp.y.SetRange( 0.0, g_engine->GetGraphics()->GetViewportHeight(), -1.0, 1.0 );
	m_clamp.y.SetInversed( true );
	
	if ( !m_loader ) {
		NEW( m_loader, module::Loader );
	}

	g_engine->GetGraphics()->AddOnResizeHandler( this, RH( this ) {
#ifdef DEBUG
		for (auto& it : m_debug_frames) {
			ResizeDebugFrame( it.first, &it.second );
		}
#endif
		m_clamp.x.SetSrcRange( 0.0, g_engine->GetGraphics()->GetViewportWidth() );
		m_clamp.y.SetSrcRange( 0.0, g_engine->GetGraphics()->GetViewportHeight() );
		m_root_object.Realign();
	});
	
	m_keydown_handler = AddGlobalEventHandler( UIEvent::EV_KEY_DOWN, EH( this ) {
		// only most global handlers here
		if ( ( data->key.modifiers & UIEvent::KM_ALT ) && data->key.code == UIEvent::K_ENTER ) {
			g_engine->GetGraphics()->ToggleFullscreen();
			return true;
		}
		return false;
	}, UI::GH_BEFORE );
	
#ifdef DEBUG
	NEW( m_debug_scene, Scene, "UIDebug", SCENE_TYPE_SIMPLE2D );
	g_engine->GetGraphics()->AddScene( m_debug_scene );	
#endif
}

void UI::Stop() {
	Log( "Destroying UI" );

#ifdef DEBUG
	g_engine->GetGraphics()->RemoveScene( m_debug_scene );
	DELETE( m_debug_scene );
#endif
	
	RemoveGlobalEventHandler( m_keydown_handler );
	
	g_engine->GetGraphics()->RemoveOnResizeHandler( this );
	
	if ( m_loader ) {
		m_loader->Stop();
		DELETE( m_loader );
		m_loader = nullptr;
	}
	
	g_engine->GetGraphics()->RemoveScene( m_text_scene );
	DELETE( m_text_scene );

	g_engine->GetGraphics()->RemoveScene( m_shape_scene_simple2d );
	DELETE( m_shape_scene_simple2d );
	g_engine->GetGraphics()->RemoveScene( m_shape_scene_ortho );
	DELETE( m_shape_scene_ortho );
	
	ASSERT( m_focusable_objects.empty(), "some objects still remain focusable" );
	ASSERT( m_focusable_objects_order.empty(), "some objects still remain in focusable order" );
	
	m_root_object.Destroy();
}

void UI::AddObject( object::UIObject *object ) {
	m_root_object.AddChild( object );
}
void UI::RemoveObject( object::UIObject *object ) {
	m_root_object.RemoveChild( object );
}

Scene *UI::GetShapeScene( const types::mesh::Mesh* mesh ) {
	switch ( mesh->GetType() ) {
		case types::mesh::Mesh::MT_SIMPLE: {
			return m_shape_scene_simple2d;
		}
		case types::mesh::Mesh::MT_RENDER: {
			return m_shape_scene_ortho;
		}
		default: {
			ASSERT( false, "unknown mesh type " + std::to_string( mesh->GetType() ) );
		}
	}
	ASSERT( false, "unsupported shape mesh type" );
	return nullptr; // remove warning
}

Scene *UI::GetTextScene() {
	return m_text_scene;
}

void UI::SetWorldUIMatrix( const types::Matrix44& matrix ) {
	m_world_ui_matrix = matrix;
}

const types::Matrix44& UI::GetWorldUIMatrix() const {
	return m_world_ui_matrix;
}

const UI::coord_t UI::ClampX( const coord_t value ) const {
	return m_clamp.x.Clamp( value );
}

const UI::coord_t UI::ClampY( const coord_t value ) const {
	return m_clamp.y.Clamp( value );
}

const UI::coord_t UI::UnclampX( const coord_t value ) const {
	return m_clamp.x.Unclamp( value );
}

const UI::coord_t UI::UnclampY( const coord_t value ) const {
	return m_clamp.y.Unclamp( value );
}
	
void UI::Iterate() {
	m_root_object.Iterate();
	
	if ( !m_iterative_objects_to_remove.empty() ) {
		for ( auto& remove_it : m_iterative_objects_to_remove ) {
			auto it = m_iterative_objects.find( remove_it );
			ASSERT( it != m_iterative_objects.end(), "iterative object to be removed not found" );
			m_iterative_objects.erase( it );
		}
		m_iterative_objects_to_remove.clear();
	}
	
	for ( auto& it : m_iterative_objects ) {
		it.second();
	}
	
	if ( m_is_redraw_needed ) {
		Log( "Redrawing UI" );
		g_engine->GetGraphics()->RedrawOverlay();
		m_is_redraw_needed = false;
	}
}

void UI::TriggerGlobalEventHandlers( global_event_handler_order_t order, UIEvent* event ) {
	auto ghs1 = m_global_event_handlers.find( order );
	if ( ghs1 != m_global_event_handlers.end() ) {
		auto ghs2 = ghs1->second.find( event->m_type );
		if ( ghs2 != ghs1->second.end() ) {
			for ( auto& h : ghs2->second ) {
				if ( h->Execute( &event->m_data ) ) {
					event->SetProcessed();
					return;
				}
			}
		}
	}
}

void UI::ProcessEvent( UIEvent* event ) {
	if ( event->m_type == UIEvent::EV_MOUSE_MOVE ) {
		// need to save last mouse position to be able to trigger mouseover/mouseout events for objects that will move/resize themselves later
		m_last_mouse_position = { event->m_data.mouse.x, event->m_data.mouse.y };
	}
	
	TriggerGlobalEventHandlers( GH_BEFORE, event );
	
	if ( event->m_type == UIEvent::EV_KEY_DOWN ) {
		if ( m_focused_object && !event->m_data.key.modifiers && ( event->m_data.key.code == UIEvent::K_TAB ) ) {
			FocusNextObject();
			event->SetProcessed();
		}
	}
	
	if ( !event->IsProcessed() ) {
		m_root_object.ProcessEvent( event );
	}
	
	if ( !event->IsProcessed() ) {
		if ( event->m_type == UIEvent::EV_KEY_DOWN ) {
			if ( m_focused_object && !event->m_data.key.modifiers && (
				( event->m_data.key.key ) || // ascii key
				( event->m_data.key.code == UIEvent::K_BACKSPACE )
			) ) {
				m_focused_object->ProcessEvent( event );
			}
		}
	}

	if ( !event->IsProcessed() ) {
		TriggerGlobalEventHandlers( GH_AFTER, event );
	}
}

void UI::SendMouseMoveEvent( UIObject* object ) {
	NEWV( event, MouseMove, m_last_mouse_position.x, m_last_mouse_position.y );
	object->ProcessEvent( event );
	DELETE( event );
}

const UIEventHandler* UI::AddGlobalEventHandler( const UIEvent::event_type_t type, const UIEventHandler::handler_function_t& func, const global_event_handler_order_t order ) {
	NEWV( handler, UIEventHandler, func );
	auto its = m_global_event_handlers.find( order );
	if ( its == m_global_event_handlers.end() ) {
		m_global_event_handlers[ order ] = {};
		its = m_global_event_handlers.find( order );
	}
	auto it = its->second.find( type );
	if ( it == its->second.end() ) {
		its->second[ type ] = {};
		it = its->second.find( type );
	}
	it->second.push_back( handler );
	return handler;
}

void UI::RemoveGlobalEventHandler( const UIEventHandler* handler ) {
	for ( auto& its : m_global_event_handlers ) {
		for ( auto& handlers : its.second ) {
			auto it = find( handlers.second.begin() , handlers.second.end(), handler );
			if ( it != handlers.second.end() ) {
				DELETE( *it );
				handlers.second.erase( it );
				return;
			}
		}
	}
	ASSERT( false, "handler not found" );
}

void UI::AddTheme( theme::Theme* theme ) {
	ASSERT( m_themes.find( theme ) == m_themes.end(), "theme already set" );
	theme->Finalize();
	m_themes.insert( theme );
}

void UI::RemoveTheme( theme::Theme* theme ) {
	auto it = m_themes.find( theme );
	ASSERT( it != m_themes.end(), "theme wasn't set" );
	m_themes.erase( theme );
}

const UI::themes_t UI::GetThemes() const {
	return m_themes;
}

void UI::AddToFocusableObjects( UIObject* object ) {
	Log( "Adding " + object->GetName() + " to focusable objects" );
	ASSERT( m_focusable_objects.find( object ) == m_focusable_objects.end(), "object already focusable" );
	m_focusable_objects.insert( object );
	UpdateFocusableObjectsOrder();
	if ( !m_focused_object ) {
		FocusNextObject(); // focus it if it's first one
	}
}

void UI::RemoveFromFocusableObjects( UIObject* object ) {
	Log( "Removing " + object->GetName() + " from focusable objects" );
	auto it = m_focusable_objects.find( object );
	ASSERT( it != m_focusable_objects.end(), "object not focusable" );
	if ( m_focused_object == object ) {
		object->Defocus();
		m_focused_object = nullptr;
	}
	m_focusable_objects.erase( it );
	UpdateFocusableObjectsOrder();
}

void UI::UpdateFocusableObjectsOrder() {
	m_focusable_objects_order.clear();
	
	// TODO: tabindex property
	for ( auto& object : m_focusable_objects ) {
		m_focusable_objects_order.push_back( object );
	}
}

void UI::FocusNextObject() {
	if ( m_focusable_objects_order.empty() ) {
		return; // nothing to focus
	}
	bool select_next = false;
	for ( auto& object : m_focusable_objects_order ) {
		if ( !m_focused_object ) { // if nothing is focused - focus first available
			FocusObject( object );
			return;
		}
		else if ( object == m_focused_object ) {
			select_next = true; // focus object that is after current one
		}
		else if ( select_next ) {
			FocusObject( object );
			return;
		}
	}
	
	// maybe focused object is last in list (in this case select_next will be true)
	// so we need to rewind and focus first one in list
	ASSERT( select_next, "currently focused object not found" );
	FocusObject( m_focusable_objects_order[0] );
}

void UI::FocusObject( UIObject* object ) {
	if ( object != m_focused_object ) {
		if ( m_focused_object ) {
			m_focused_object->Defocus();
		}
		object->Focus();
		m_focused_object = object;
	}
}

const theme::Style* UI::GetStyle( const std::string& style_class ) const {
	for ( auto& theme : m_themes ) {
		auto* style = theme->GetStyle( style_class );
		if ( style ) {
			return style;
		}
	}
	ASSERT( false, "style '" + style_class + "' does not exist" );
	return nullptr;
}

void UI::AddIterativeObject( void* object, const object_iterate_handler_t handler ) {
	ASSERT( m_iterative_objects.find( object ) == m_iterative_objects.end(), "iterative object already exists" );
	m_iterative_objects[ object ] = handler;
}

void UI::RemoveIterativeObject( void* object ) {
	ASSERT( find( m_iterative_objects_to_remove.begin(), m_iterative_objects_to_remove.end(), object ) == m_iterative_objects_to_remove.end(),
		"iterative object already in removal queue"
	);
	ASSERT( m_iterative_objects.find( object ) != m_iterative_objects.end(), "iterative object not found" );
	m_iterative_objects_to_remove.push_back( object ); // can't remove here because removal can be requested from within it's handler
}

module::Loader* UI::GetLoader() const {
	ASSERT( m_loader, "loader not set" );
	return m_loader;
}

void UI::BlockEvents() {
	m_root_object.BlockEvents();
}

void UI::UnblockEvents() {
	m_root_object.UnblockEvents();
}

void UI::Redraw() {
	m_is_redraw_needed = true;
}

#ifdef DEBUG
void UI::ShowDebugFrame( const UIObject* object ) {
	auto it = m_debug_frames.find( object );
	if ( it == m_debug_frames.end() ) {
		Log("Showing debug frame for " + object->GetName());
		debug_frame_data_t data;
		
		// semi-transparent 1x1 texture with random color for every frame
		NEW( data.texture, Texture, "DebugTexture", 1, 1 );
		data.texture->SetPixel( 0, 0, Color::RGBA( rand() % 256, rand() % 256, rand() % 256, 160 ) );
		
		NEW( data.mesh, mesh::Rectangle );
		
		NEW( data.actor, actor::Mesh, "DebugFrame", data.mesh );
		data.actor->SetTexture( data.texture );
		
		ResizeDebugFrame( object, &data );
		
		m_debug_scene->AddActor( data.actor );
		
		m_debug_frames[object] = data;
	}
}

void UI::HideDebugFrame( const UIObject* object ) {
	auto it = m_debug_frames.find( object );
	if ( it != m_debug_frames.end() ) {
		Log("Hiding debug frame for " + object->GetName());
		m_debug_scene->RemoveActor( it->second.actor );
		DELETE( it->second.actor );
		DELETE( it->second.texture );
		m_debug_frames.erase( it );
	}
}

void UI::ResizeDebugFrame( const UIObject* object, const debug_frame_data_t* data ) {
	auto geom = object->GetAreaGeometry();
	data->mesh->SetCoords({
		ClampX(geom.first.x),
		ClampY(geom.first.y)
	},{
		ClampX(geom.second.x),
		ClampY(geom.second.y)
	}, -1.0 );
	
}

void UI::ResizeDebugFrame( const UIObject* object ) {
	auto it = m_debug_frames.find( object );
	if ( it != m_debug_frames.end() ) {
		ResizeDebugFrame( object, &it->second );
	}
}

#endif

} /* namespace ui */
