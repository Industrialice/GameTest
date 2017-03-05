#ifndef __GAME_TEST_HPP__
#define __GAME_TEST_HPP__

#include "DataStructs.hpp"

namespace GameTest
{
	using gameTestError = CError < const char * >;

	Renderer *CurrentRenderer();
	void CurrentRenderer( Renderer *renderer );

	void AddWorldManager( const std::shared_ptr < class WorldManager > &manager );
	CRefVec < std::shared_ptr < class WorldManager > > WorldManagers();
	void RemoveWorldManager( const std::shared_ptr < class WorldManager > &manager );

	bool ValidateDisplayInfos( const CRefVec < const DisplayInfo > &displays );  //  will return true if displays is empty
	CLogger &GetDefaultLogger();
	class ControllablesStack &GetGlobalControlsHandler();
#ifdef WINDOWS
	class GameWindow *FindWindowByHWND( HWND hwnd );
#endif

#define GETLOGTAG( tag, prepend, append ) \
	[]( CLogger::Tag tag ) -> const char * \
	{ \
		switch( tag ) \
		{ \
		case CLogger::Tag::info: \
			return CONCAT( CONCAT( prepend, "inf" ), append ); \
		case CLogger::Tag::warning: \
			return CONCAT( CONCAT( prepend, "wrn" ), append ); \
		case CLogger::Tag::error: \
			return CONCAT( CONCAT( prepend, "err" ), append ); \
		case CLogger::Tag::debug: \
			return CONCAT( CONCAT( prepend, "dgb" ), append ); \
		case CLogger::Tag::user: \
			return CONCAT( CONCAT( prepend, "usr" ), append ); \
		case CLogger::Tag::important: \
			return CONCAT( CONCAT( prepend, "imp" ), append ); \
		case CLogger::Tag::critical: \
			return CONCAT( CONCAT( prepend, "crl" ), append ); \
		} \
		\
		return CONCAT( CONCAT( prepend, "???" ), append ); \
	} ( tag );

	namespace Private
	{
		void AllocateWindow( class GameWindow &window );
		void DeallocateWindow( class GameWindow &window );
		bool Main( const CRefVec < const CLogger::DirectionFunc > &loggerDirections );
		void Shutdown();
	}

	#define SENDLOG( tag, ... ) GameTest::GetDefaultLogger().Message( CLogger::Tag::tag, __VA_ARGS__ )
}

#endif