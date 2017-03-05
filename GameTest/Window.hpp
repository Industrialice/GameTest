#ifndef __GAME_WINDOW_HPP__
#define __GAME_WINDOW_HPP__

#include "DataStructs.hpp"

namespace GameTest
{
	class GameWindow;

	namespace Private
	{
		void AllocateWindow( GameWindow &window );
		void DeallocateWindow( GameWindow &window );
	}

	struct GameWindowParameters
	{
		i32 width = 0, height = 0;
		i32 x = 0, y = 0;
		bln is_fullscreen = false;
		string title;
		vector < DisplayInfo > targetDisplays;  //  can be empty if unknown
	#ifdef WINDOWS
		HWND hwnd = NULL;  //  this parameter is going to be ignored if you try to update it, you can change it only if the current value is NULL
		DWORD style = 0;
	#endif
	};

	class GameWindow : public RendererBackendData
	{
		GameWindowParameters _parameters;

		GameWindow &operator = ( const GameWindow & ) = delete;
		GameWindow( const GameWindow & ) = delete;

	public:
		~GameWindow()
		{
			DestroyRendererBackendData( *CurrentRenderer() );

			Private::DeallocateWindow( *this );
			#ifdef WINDOWS
				DestroyWindow( _parameters.hwnd );
			#endif
		}

		GameWindow( const GameWindowParameters &parameters = GameWindowParameters() ) : _parameters( parameters )
		{
			Private::AllocateWindow( *this );
		}

		const GameWindowParameters &Parameters() const
		{
			return _parameters;
		}

		void Parameters( const GameWindowParameters &parameters )  //  TODO: handle changes properly!
		{
		#ifdef WINDOWS
			::HWND currentHwnd = _parameters.hwnd;
		#endif

			_parameters = parameters;

		#ifdef WINDOWS
			if( currentHwnd != NULL && _parameters.hwnd != currentHwnd )
			{
				SOFTBREAK;
				_parameters.hwnd = currentHwnd;
			}
		#endif

			RendererBackendDataMayBeDirty();
		}

		i32 Width() const
		{
			return _parameters.width;
		}

		i32 Height() const
		{
			return _parameters.height;
		}

		i32 X() const
		{
			return _parameters.x;
		}

		i32 Y() const
		{
			return _parameters.y;
		}

		bln IsFullscreen() const
		{
			return _parameters.is_fullscreen;
		}

		const string &Title() const
		{
			return _parameters.title;
		}

		const vector < DisplayInfo > &TargetDisplays() const
		{
			return _parameters.targetDisplays;
		}

	#ifdef WINDOWS
		HWND HWND() const
		{
			return _parameters.hwnd;
		}

		DWORD Style() const
		{
			return _parameters.style;
		}
	#endif
	};
}

#endif