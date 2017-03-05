#include "PreHeader.hpp"
#include "GameTest.hpp"
#include "ControllablesStack.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "WorldManager.hpp"

namespace
{
	CLogger *DefaultLogger;
	GameTest::ControllablesStack GlobalControlsHandler;
	vector < GameTest::GameWindow * > Windows;
	GameTest::Renderer *GTCurrentRenderer;
	vector < std::shared_ptr < GameTest::WorldManager > > GTWorldManagers;
}

GameTest::Renderer *GameTest::CurrentRenderer()
{
	return GTCurrentRenderer;
}

void GameTest::CurrentRenderer( Renderer *renderer )
{
	//  TODO: global state cleanup

	if( GTCurrentRenderer )
	{
		for( auto window : Windows )
		{
			GTCurrentRenderer->FreeBackendData( *window );
		}
	}

	GTCurrentRenderer = renderer;
}

void GameTest::AddWorldManager( const std::shared_ptr < class WorldManager > &manager )
{
	GTWorldManagers.emplace_back( manager );
}

auto GameTest::WorldManagers() -> CRefVec < std::shared_ptr < WorldManager > >
{
	return MakeRefVec( GTWorldManagers.data(), GTWorldManagers.size() );
}

void GameTest::RemoveWorldManager( const std::shared_ptr < class WorldManager > &manager )
{
	auto it = std::find( GTWorldManagers.begin(), GTWorldManagers.end(), manager );
	if( it == GTWorldManagers.end() )
	{
		SENDLOG( error, "GameTest::RemoveWorldManager -> there's no such world manager, failed to remove\n" );
		return;
	}

	GTWorldManagers.erase( it );
}

bool GameTest::ValidateDisplayInfos( const CRefVec < const DisplayInfo > &displays )
{
	return true;  //  TODO:
}

CLogger &GameTest::GetDefaultLogger()
{
	return *DefaultLogger;
}

GameTest::ControllablesStack &GameTest::GetGlobalControlsHandler()
{
	return GlobalControlsHandler;
}

#ifdef WINDOWS
GameTest::GameWindow *GameTest::FindWindowByHWND( HWND hwnd )
{
	auto it = std::find_if( Windows.begin(), Windows.end(), [hwnd]( GameWindow *window ) { return window->HWND() == hwnd; } );
	if( it != Windows.end() )
	{
		return *it;
	}
	SENDLOG( error, "GameTest::FindWindowByHWND -> there's no such window\n" );
	return nullptr;
}
#endif

void GameTest::Private::AllocateWindow( GameWindow &window )
{
	Windows.push_back( &window );
}

void GameTest::Private::DeallocateWindow( GameWindow &window )
{
	auto it = std::find( Windows.begin(), Windows.end(), &window );
	if( it == Windows.end() )
	{
		SENDLOG( error, "GameTest::Private::DeallocateWindow -> there's no such window, failed to deallocate\n" );
		return;
	}

	Windows.erase( it );
}

bool GameTest::Private::Main( const CRefVec < const CLogger::DirectionFunc > &loggerDirections )
{
	DefaultLogger = CLogger::Create( "default_logger", true, true );
	for( CLogger::DirectionFunc dir : loggerDirections )
	{
		DefaultLogger->DirectionAdd( dir );
	}

	return true;
}

void GameTest::Private::Shutdown()
{
	CLogger::Delete( DefaultLogger );
	DefaultLogger = nullptr;
}
