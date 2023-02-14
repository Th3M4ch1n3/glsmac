#include <thread>

#ifdef DEBUG
#include <iostream>
#include <string>
#include <stdlib.h>
#endif

#include "config/Config.h"
#ifdef _WIN32
#include "error_handler/Win32.h"
#else
#include "error_handler/Stdout.h"
#endif
#include "logger/Stdout.h"

#include "loader/font/FreeType.h"
#include "loader/texture/SDL2.h"
#include "loader/sound/SDL2.h"

#include "input/sdl2/SDL2.h"
#include "graphics/opengl/OpenGL.h"
#include "audio/sdl2/SDL2.h"
#include "network/simpletcp/SimpleTCP.h"

#include "scheduler/Simple.h"
#include "ui/Default.h"

#include "game/common/Common.h"
#ifdef DEVEL
#include "game/world/World.h"
#else
#include "game/intro/Intro.h"
#endif

#include "engine/Engine.h"

#include "version.h"

#ifdef TMP_LAST_COMMIT_H_GENERATED
#include "tmp/last_commit.h"
#else
#define GLSMAC_LAST_COMMIT "???????"
#endif

// TODO: move to config
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define VSYNC true
#ifdef DEBUG
#define START_FULLSCREEN false
#else
#define START_FULLSCREEN true
#endif

#ifdef DEBUG
#include "util/System.h"
#include "util/FS.h"
#include "debug/MemoryWatcher.h"
#include "debug/DebugOverlay.h"
using namespace util;
#endif

using namespace std;

#ifdef main
#undef main
#endif

#if !defined(_WIN32) || defined( VISUAL_STUDIO )
int main(const int argc, const char *argv[]) {
#else
int main_real(const int argc, const char* argv[]) {
#endif

#ifdef DEBUG
#ifdef __linux__
	// automatically start under gdb if possible
	if ( !System::AreWeUnderGDB() && System::IsGDBAvailable() ) {
		cout << "Restarting process under GDB..." << endl;

		string cmdline = "printf \"r\\nbt\\n\" | gdb --args";
		for ( int c = 0 ; c < argc ; c++ ) {
			cmdline += (string) " " + argv[c];
		}
		cmdline += " 2>&1 | tee debug.log";
		
		cout << cmdline << endl;
		int status = system( cmdline.c_str() );
		if (status < 0) {
			cout << "Error: " << strerror(errno) << endl;
			exit( EXIT_FAILURE );
		}
		else if ( WIFEXITED(status) ) {
			cout << "Process finished, output saved to debug.log" << endl;
			exit( EXIT_SUCCESS );
		}
		else {
			cout << "Process finished, output saved to debug.log" << endl;
			exit( EXIT_FAILURE );
		}
	}
#else
	cout << "WARNING: gdb check skipped due to unsupported platform" << endl;
#endif
	debug::MemoryWatcher memory_watcher;
#endif
	
#ifdef DEBUG
	FS::CreateDirectoryIfNotExists( "./tmp" ); // to store debug stuff like dumps
#endif
	
	int result = EXIT_FAILURE;
	
	// logger needs to be outside of scope to be destroyed last
	logger::Stdout logger;
	{
		config::Config config( argc, argv );
		
#ifdef _WIN32
		error_handler::Win32 error_handler;
#else
		error_handler::Stdout error_handler;
#endif

		loader::font::FreeType font_loader;

		loader::texture::SDL2 texture_loader;
		texture_loader.SetTransparentColor(types::Color::RGBA(255, 0, 255, 255));
		
		loader::sound::SDL2 sound_loader;
		
		auto title = (string) "GLSMAC " + GLSMAC_VERSION + "-" + GLSMAC_LAST_COMMIT;
#ifdef DEBUG
		title += "-debug";
#elif PORTABLE
		title += "-portable";
#endif
		input::sdl2::SDL2 input;
		graphics::opengl::OpenGL graphics( title, WINDOW_WIDTH, WINDOW_HEIGHT, VSYNC, START_FULLSCREEN );
		audio::sdl2::SDL2 audio;
		network::simpletcp::SimpleTCP network;
		
		ui::Default ui;

		scheduler::Simple scheduler;

#ifdef DEBUG
		NEWV( debug_overlay, debug::DebugOverlay );
		scheduler.AddTask( debug_overlay );
#endif
		
		// game common stuff
		NEWV( task_common, game::Common );
		scheduler.AddTask( task_common );
		
		// game entry point
#ifdef DEVEL
		game::Settings settings;
		NEWV( task, game::world::World, settings );
#else
		NEWV( task, game::intro::Intro );
#endif
		scheduler.AddTask( task );

		engine::Engine engine(
			&config,
			&error_handler,
			&logger,
			&font_loader,
			&texture_loader,
			&sound_loader,
			&scheduler,
			&input,
			&graphics,
			&audio,
			&network,
			&ui
		);

		result = engine.Run(); 
	}

	return result;
}

#if defined(_WIN32) && !defined( VISUAL_STUDIO )
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
    main_real(__argc, (const char**)__argv);
}
#endif
