#include "PreHeader.hpp"

#ifdef WINDOWS

#include "GameTestWindows.hpp"
#include "External\HID\hidusage.h"
#include <RendererDX11.hpp>
#include <VKeys.hpp>
#include "ComponentCamera.hpp"
#include "ControllablesStack.hpp"
#include "Material.hpp"
#include "WorldObject.hpp"

namespace GameTest
{
	HINSTANCE AppHinstance;
}

static bln RegisterInputDevices();
static void VSOutputLogFunc( CLogger::Tag tag, const char *text, uiw len );
static void CriticalOutputLogFunc( CLogger::Tag tag, const char *text, uiw len );
static LRESULT WINAPI MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
static bln PerformTest();

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int )
{
	GameTest::AppHinstance = hInstance;

	StdAbstractionLib_Initialize();

	static const CLogger::DirectionFunc dirs[] = { VSOutputLogFunc, CriticalOutputLogFunc };
	if( GameTest::Private::Main( dirs ) == false )
	{
		return 1;
	}

	if( RegisterInputDevices() == false )
	{
		SENDLOG( critical, "failed to initialize input devices, error %u : %s", GetLastError(), GameTest::SystemErrorToString( GetLastError() ).c_str() );
		return 1;
	}
	SENDLOG( info, "input devices registration has been completed\n" );

	if( PerformTest() == false )
	{
		return 1;
	}

	GameTest::Private::Shutdown();

	return 0;
}

void Lambda( i32 a, f32 b )
{
	char buf[ 128 ];
	Funcs::PrintToStr( buf, 128, "lambda %i %f\n", a, b );
	OutputDebugStringA( buf );
}

bln PerformTest()
{
	using namespace GameTest;

	GameWindowParameters windowParameters;
	windowParameters.x = 0;
	windowParameters.y = 0;
	windowParameters.width = 1920;
	windowParameters.height = 1080;
	windowParameters.title = "proto_engine";
	windowParameters.is_fullscreen = false;
	windowParameters.style = WS_SYSMENU;

	std::shared_ptr < GameWindow > window = std::make_shared < GameWindow >( windowParameters );
	gameTestError error;
	if( !CreateSystemWindow( *window.get(), &error ) )
	{
		SENDLOG( critical, "failed to create test window, error %s : %s", error.Description(), error.Addition() );
		return false;
	}
	SENDLOG( info, "test window has been created\n" );

	/*std::shared_ptr < GameWindow > window2 = std::make_shared < GameWindow >();
	window2->x = 1920;
	window2->y = 0;
	window2->width = 1920;
	window2->height = 1080;
	window2->title = "proto_engine_window_2";
	window2->is_fullscreen = true;
	window2->style = WS_SYSMENU;
	if( !CreateSystemWindow( window2.get(), &error ) )
	{
		SENDLOG( critical, "failed to create test window2, error %s : %s", error.Description(), error.Addition() );
		return false;
	}
	SENDLOG( info, "test window2 has been created\n" );*/
	
	Renderer::rendererError rendererError;
	RendererDX11 *renderer = CreateRendererDX11( D3D_FEATURE_LEVEL_11_0, ui32_max, &rendererError );
	if( renderer == nullptr )
	{
		SENDLOG( critical, "failed to initialize dx11 renderer, error %s : %s", rendererError.Description(), rendererError.Addition() );
		return false;
	}
	SENDLOG( info, "test renderer has been created\n" );
	CurrentRenderer( renderer );

	std::shared_ptr < RenderTarget > rt = std::make_shared < RenderTarget >();
	rt->ColorTarget( window );

	/*std::shared_ptr < RenderTarget > rt2 = std::make_shared < RenderTarget >();
	rt2->ColorTargetSet( window2 );*/

	auto worldObject = WorldObject::New();
	Components::Camera *camera = worldObject->ComponentAdd < Components::Camera >();
	if( camera == nullptr )
	{
		SENDLOG( error, "worldObject.ComponentAdd failed\n" );
		return false;
	}

	camera->RenderTargetSet( 0, rt );
	//camera->RenderTargetSet( 1, rt2 );

	MSG msg;

	for( uiw index = 0; index < 1; ++index )
	{
		TimeMoment endMoment = TimeMoment::CreateShiftedSec( TimeMoment::CreateCurrent(), 2.5f );

		while( TimeMoment::CreateCurrent() < endMoment )
		{
			while( PeekMessageA( &msg, 0, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessageA( &msg );
			}

			camera->ClearColorSet( f96color( Funcs::RandomF32(), Funcs::RandomF32(), Funcs::RandomF32() ) );

			if( !CurrentRenderer()->BeginFrame() )
			{
				SENDLOG( critical, "failed to begin frame\n" );
			}

			CurrentRenderer()->RenderScene( WorldManagers(), *camera );

			if( !CurrentRenderer()->EndFrame() )
			{
				SENDLOG( critical, "failed to end frame\n" );
			}
		}
		
		auto parameters = window->Parameters();
		parameters.width = 1920;
		parameters.height = 1080;
		parameters.is_fullscreen = false;
		window->Parameters( parameters );

		/*window2->width = 1920;
		window2->height = 1080;
		window2->is_fullscreen = true;*/
	}

	CurrentRenderer( nullptr );
	DestroyRendererDX11( renderer );

	return true;
}

bln GameTest::CreateSystemWindow( GameWindow &window, gameTestError *error )
{
	auto parameters = window.Parameters();

	if( parameters.hwnd != NULL )
	{
		DSA( error, gameTestError( Error::InvalidArgument(), "called with an already created window" ) );
		return false;
	}

	string className = "game_test_";
	className += parameters.title;

	WNDCLASSA wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;  //  means that we need to redraw the whole window if its size changes, not just a new portion
	wc.lpfnWndProc = MsgProc;  //  note that the message procedure runs in the same thread that created the window( it's a requirement )
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AppHinstance;
	wc.hIcon = ::LoadIconA( 0, IDI_APPLICATION );
	wc.hCursor = ::LoadCursorA( 0, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)::GetStockObject( NULL_BRUSH );
	wc.lpszMenuName = 0;
	wc.lpszClassName = className.c_str();

	if( !RegisterClassA( &wc ) )
	{
		DSA( error, gameTestError( Error::UnknownError(), "failed to register class" ) );
		return false;
	}

	DWORD style = /*window.is_fullscreen ? WS_POPUP : */parameters.style;

	RECT R = { 0, 0, parameters.width, parameters.height };
	AdjustWindowRect( &R, style, false );

	ui32 width = R.right - R.left;
	ui32 height = R.bottom - R.top;

	parameters.hwnd = CreateWindowA( className.c_str(), parameters.title.c_str(), style, parameters.x, parameters.y, width, height, 0, 0, AppHinstance, 0 );
	if( !parameters.hwnd )
	{
		DSA( error, gameTestError( Error::UnknownError(), "failed to create requested window" ) );
		return false;
	}

	ShowWindow( parameters.hwnd, SW_SHOW );
	UpdateWindow( parameters.hwnd );

	window.Parameters( parameters );

	DSA( error, Error::Ok() );
	return true;
}

bln GameTest::SetWindowTitle( GameWindow &window, const char *title, gameTestError *error )
{
	DSA( error, Error::Unimplemented() );
	return false;
}

bln GameTest::UpdateWindowState( GameWindow &window, i32 x, i32 y, ui32 width, ui32 height, gameTestError *error )
{
	DSA( error, Error::Unimplemented() );
	return false;
}
	
bln GameTest::SetWindowStyle( GameWindow &window, DWORD style, gameTestError *error )
{
	DSA( error, Error::Unimplemented() );
	return false;
}

string GameTest::SystemErrorToString( DWORD error )
{
	struct Deleter
	{
		void operator()( void *value )
		{
			LocalFree( value );
		}
	};

	char *str = nullptr;

    DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&str, 0, NULL);

	std::unique_ptr < char[], Deleter > messageBuffer( str );

	return string( messageBuffer.get(), size );
}

bln RegisterInputDevices()
{
    RAWINPUTDEVICE hids[ 2 ];

    hids[ 0 ].usUsagePage = HID_USAGE_PAGE_GENERIC;
    hids[ 0 ].usUsage = HID_USAGE_GENERIC_MOUSE;
    hids[ 0 ].dwFlags = RIDEV_NOLEGACY;
    hids[ 0 ].hwndTarget = NULL;

    hids[ 1 ].usUsagePage = HID_USAGE_PAGE_GENERIC;
    hids[ 1 ].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    hids[ 1 ].dwFlags = RIDEV_NOLEGACY | RIDEV_APPKEYS;
    hids[ 1 ].hwndTarget = NULL;

    if( !RegisterRawInputDevices( hids, COUNTOF(hids), sizeof(*hids) ) )
    {
        return false;
    }

	return true;
}

void VSOutputLogFunc( CLogger::Tag tag, const char *text, uiw len )
{
	const char *tagStr = GETLOGTAG( tag, "[", "] ");
	OutputDebugStringA( tagStr );
	OutputDebugStringA( text );
}

void CriticalOutputLogFunc( CLogger::Tag tag, const char *text, uiw len )
{
	if( tag == CLogger::Tag::critical )
	{
		MessageBoxA( NULL, text, "CRITICAL ERROR", 0 );
	}
}

static void HandleHIDInput( HWND hwnd, WPARAM wParam, LPARAM lParam );

LRESULT WINAPI MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
    case WM_INPUT:
        HandleHIDInput( hwnd, wParam, lParam );
		return 0;
	case WM_ACTIVATE:
		//  switching window's active state
		break;
	case WM_SIZE:
	{
		GameTest::GameWindow *window = GameTest::FindWindowByHWND( hwnd );
		if( window != nullptr )
		{
			//window->width = LOWORD( lParam );
			//window->height = HIWORD( lParam );
			if( GameTest::CurrentRenderer() != nullptr )
			{
				GameTest::CurrentRenderer()->NotifyWindowResized( window );
			}
			return 0;
		}
		else
		{
			//SOFTBREAK;
		}
	} break;
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		break;
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	case WM_EXITSIZEMOVE:
		break;
		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		break;
		// somebody has asked our window to close
	case WM_CLOSE:
		break;
		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		//((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		//((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		break;
	}

	return DefWindowProcA( hwnd, msg, wParam, lParam );
}

//  TODO: don't call these control events directly, just put new events on a message queue and handle them outside of WNDPROC
void HandleHIDInput( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    RAWINPUT data;
    UINT dataSize = sizeof(data);
    if( ::GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, &data, &dataSize, sizeof(RAWINPUTHEADER) ) == -1 )
    {
        return;
    }
    ASSUME( dataSize == sizeof(data) );

	TimeMoment tm = TimeMoment::CreateCurrent();

	if( data.header.dwType == RIM_TYPEMOUSE )
	{
		const RAWMOUSE &mouse = data.data.mouse;

        if( mouse.usFlags & MOUSE_MOVE_ABSOLUTE )
        {
            HARDBREAK;
        }
        if( mouse.usFlags & MOUSE_MOVE_NOCOALESCE )
        {
            HARDBREAK;
        }

		if( mouse.lLastX || mouse.lLastY )
		{
			GameTest::GetGlobalControlsHandler().MouseMove( mouse.lLastX, mouse.lLastY, { tm, 0, hwnd } );
		}

		if( mouse.usButtonFlags )
		{
			static const struct
			{
				ui32 up;
				ui32 down;
			} keys[ 5 ] =
			{
				{ RI_MOUSE_BUTTON_1_UP, RI_MOUSE_BUTTON_1_DOWN },
				{ RI_MOUSE_BUTTON_2_UP, RI_MOUSE_BUTTON_2_DOWN },
				{ RI_MOUSE_BUTTON_3_UP, RI_MOUSE_BUTTON_3_DOWN },
				{ RI_MOUSE_BUTTON_4_UP, RI_MOUSE_BUTTON_4_DOWN },
				{ RI_MOUSE_BUTTON_5_UP, RI_MOUSE_BUTTON_5_DOWN }
			};

			for( ui32 key = 0; key < COUNTOF(keys); ++key )
			{
				if( mouse.usButtonFlags & keys[ key ].down )
				{
					GameTest::GetGlobalControlsHandler().KeyDown( (VKeys::vkey_t)(VKeys::MButton0 + key), { tm, 0, hwnd } );
				}
				else if( mouse.usButtonFlags & keys[ key ].up )
				{
					GameTest::GetGlobalControlsHandler().KeyUp( (VKeys::vkey_t)(VKeys::MButton0 + key), { tm, 0, hwnd } );
				}
			}

            if( mouse.usButtonFlags & RI_MOUSE_WHEEL )
            {
				//  TODO: some kind of normalization?
				GameTest::GetGlobalControlsHandler().MouseScroll( mouse.usButtonData / WHEEL_DELTA, { tm, 0, hwnd } );
            }
		}
	}
	else if( data.header.dwType == RIM_TYPEKEYBOARD )  //  TODO: fix left and right controls and alt keys
	{
		const RAWKEYBOARD &kb = data.data.keyboard;
		VKeys::vkey_t key = (VKeys::vkey_t)VKeys::GetPlatformMappingStruct()[ kb.VKey ];

		if( key == VKeys::Shift )
		{
			key = (kb.MakeCode == 0x2A) ? VKeys::LShift : VKeys::RShift;
		}
		else if( key == VKeys::Control )
		{
			//  TODO: left/right
		}
		else if( key == VKeys::Alt )
		{
			//  TODO: left/right
		}
		else if( key == VKeys::Enter )
		{
			//  TODO: left/right
		}

		if( kb.Flags == RI_KEY_BREAK )
		{
			GameTest::GetGlobalControlsHandler().KeyUp( key, { tm, 0, hwnd } );
		}
		else
		{
			ASSUME( kb.Flags == RI_KEY_MAKE );
			GameTest::GetGlobalControlsHandler().KeyDown( key, { tm, 0, hwnd } );
		}
	}
}

#endif