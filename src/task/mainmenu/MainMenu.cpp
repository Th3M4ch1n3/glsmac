#include <cmath>
#include <algorithm>

#include "MainMenu.h"

#include "engine/Engine.h"

#include "task/game/Game.h"

#include "menu/Lobby.h"
#include "menu/Main.h"
#include "menu/Error.h"

#include "version.h"

namespace task {
namespace mainmenu {

void MainMenu::Start() {

	auto* ui = g_engine->GetUI();
	
	ui->AddTheme( &m_theme );
	
	// background
	NEW( m_background, Surface, "MainMenuBackground" );
	ui->AddObject( m_background );

	// preview image for 'customize map'
	NEW( m_customize_map_preview, ui::object::Surface, "MainMenuCustomizeMapPreview" );
		m_customize_map_preview->Hide();
	ui->AddObject( m_customize_map_preview );
	SetCustomizeMapPreview( "S2L2C2" ); // average preview by default
	
	NEW( m_customize_map_moons, ui::object::Surface, "MainMenuCustomizeMapMoons" );
		m_customize_map_moons->Hide();
	ui->AddObject( m_customize_map_moons );

	ResizeCustomizeMapPreview();

	NEW( m_glsmac_logo, ui::object::Label, "MainMenuGLSMACLogo" );
		m_glsmac_logo->SetText( GLSMAC_VERSION_FULL );
	ui->AddObject( m_glsmac_logo );
	
	m_mouse_handler = ui->AddGlobalEventHandler( UIEvent::EV_MOUSE_DOWN, EH( this ) {
		// rightclick = back
		if ( data->mouse.button == UIEvent::M_RIGHT && m_menu_object ) {
			if ( !m_menu_history.empty() ) { // don't exit game on right-click
				return m_menu_object->MaybeClose(); // only sliding menus will close on right click
			}
		}
		return false;
	}, UI::GH_BEFORE );
	
	m_key_handler = ui->AddGlobalEventHandler( UIEvent::EV_KEY_DOWN, EH( this ) {
		// escape = back
		if ( !data->key.modifiers && data->key.code == UIEvent::K_ESCAPE && m_menu_object ) {
			return m_menu_object->MaybeClose(); // popups have their own escape handler
		}
		return false;
	}, UI::GH_BEFORE );

	NEW( m_music, SoundEffect, "MainMenuMusic" );
	g_engine->GetUI()->AddObject( m_music );
	
	g_engine->GetGraphics()->AddOnWindowResizeHandler( this, RH( this ) {
		if ( m_menu_object ) {
			m_menu_object->Align();
		}
		ResizeCustomizeMapPreview();
	});
	
	NEWV( menu, Main, this );
	ShowMenu( menu );
}

void MainMenu::Iterate() {
	if ( m_goback ) {
		m_goback = false;
		if ( !m_menu_history.empty() ) {
			if ( !m_customize_map_preview_history.empty() ) {
				m_customize_map_preview_filename = m_customize_map_preview_history.back();
				m_customize_map_preview_history.pop_back();
				SetCustomizeMapPreview( m_customize_map_preview_filename );
			}
			m_menu_object->Hide();
			DELETE( m_menu_object );
			m_menu_object = m_menu_history.back();
			m_menu_history.pop_back();
			m_menu_object->Show();
			m_menu_object->SetChoice( m_choice_history.back() );
			m_choice_history.pop_back();
		}
		else {
			g_engine->ShutDown();
		}
	}
	else if ( m_menu_next ) {
		if ( m_menu_object ) {
			m_customize_map_preview_history.push_back( m_customize_map_preview_filename );
			m_choice_history.push_back( m_menu_object->GetChoice() );
			m_menu_object->Hide();
			m_menu_history.push_back( m_menu_object );
		}
		m_menu_object = m_menu_next;
		m_menu_next = nullptr;
		m_menu_object->Show();
	}
	else {
		if ( m_menu_object ) {
			m_menu_object->Iterate();
		}
	}
}

void MainMenu::Stop() {
	
	g_engine->GetGraphics()->RemoveOnWindowResizeHandler( this );
	
	if ( m_menu_object ) {
		m_menu_object->Hide();
		DELETE( m_menu_object );
		
	}
	
	for (auto& it : m_menu_history) {
		DELETE( it );
	}
	
	if ( m_menu_next ) {
		DELETE( m_menu_next );
	}
	
	g_engine->GetUI()->RemoveObject( m_music );
	
	auto* ui = g_engine->GetUI();
	
	ui->RemoveGlobalEventHandler( m_mouse_handler );
	ui->RemoveGlobalEventHandler( m_key_handler );
	
	ui->RemoveObject( m_background );
	ui->RemoveObject( m_customize_map_preview );
	ui->RemoveObject( m_customize_map_moons );
	
	ui->RemoveObject( m_glsmac_logo );
	
	ui->RemoveTheme( &m_theme );

}

void MainMenu::GoBack() {
	ASSERT( !m_goback, "goback already set" );
	m_goback = true;
}

void MainMenu::ShowMenu( MenuObject* menu_object ) {
	ASSERT( !m_menu_next, "next menu already set" );
	m_menu_next = menu_object;
}

void MainMenu::MenuError( const std::string& error_text ) {
	NEWV( menu, Error, this, error_text );
	ShowMenu( menu );
}

void MainMenu::StartGame() {
	NEWV( task, task::game::Game, m_settings );
	g_engine->GetScheduler()->RemoveTask( this );
	g_engine->GetScheduler()->AddTask( task );
}

void MainMenu::SetCustomizeMapPreview( const std::string& preview_filename ) {
	//Log( "Set customize map preview to " + preview_filename );
	m_customize_map_preview_filename = preview_filename;
	m_customize_map_preview->SetTexture( g_engine->GetTextureLoader()->LoadTexture( preview_filename + ".pcx" ) );
	m_customize_map_preview->SetStretchTexture( true );
	m_customize_map_preview->Show();
}

const std::string& MainMenu::GetMapPreviewFilename() const {
	return m_customize_map_preview_filename;
}

void MainMenu::SetCustomizeMapMoons( const std::string& moons_filename ) {
	if ( !moons_filename.empty() ) {
		Log( "Set customize map moons to " + moons_filename );
		m_customize_map_moons_filename = moons_filename;
		m_customize_map_moons->SetTexture( g_engine->GetTextureLoader()->LoadTexture( moons_filename + ".pcx" ) );
		m_customize_map_moons->SetStretchTexture( true );
		m_customize_map_moons->Show();
	}
	else {
		Log( "Hiding customize map moons" );
		m_customize_map_moons->Hide();
	}
}

util::Random* MainMenu::GetRandom() {
	return &m_random;
}

void MainMenu::ResizeCustomizeMapPreview() {
	const auto* g = g_engine->GetGraphics();
	const auto w = g->GetViewportWidth();
	const auto h = g->GetViewportHeight();
	if ( m_customize_map_preview ) {
		m_customize_map_preview->SetWidth( floor( 416.0f / 1024.0f * w ) );
		m_customize_map_preview->SetHeight( floor( h ) );
	}
	if ( m_customize_map_moons ) {
		m_customize_map_moons->SetWidth( floor( 450.0f / 1024.0f * w ) );
		m_customize_map_moons->SetHeight( floor( 450.0f / 768.0f * h ) );
	}
}

} /* namespace mainmenu */
} /* namespace game */
