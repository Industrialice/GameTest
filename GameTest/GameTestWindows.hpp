#if defined(WINDOWS) && !defined(__GAME_TEST_WINDOWS_HPP__)
#define __GAME_TEST_WINDOWS_HPP__

#include "GameTest.hpp"

namespace GameTest
{
	extern HINSTANCE AppHinstance;

	bool CreateSystemWindow( GameWindow &window, gameTestError *error = 0 );
	bool SetWindowTitle( GameWindow &window, const char *title, gameTestError *error = 0 );
	bool UpdateWindowState( GameWindow &window, i32 x, i32 y, ui32 width, ui32 height, gameTestError *error = 0 );
	bool SetWindowStyle( GameWindow &window, DWORD style, gameTestError *error = 0 );
	string SystemErrorToString( DWORD error );
}

#endif
