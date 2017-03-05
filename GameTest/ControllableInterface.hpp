#ifndef __CONTROLLABLE_INTERFACE_HPP__
#define __CONTROLLABLE_INTERFACE_HPP__

#include <VKeys.hpp>
#include "DataStructs.hpp"
#include <TimeMoment.hpp>

namespace GameTest
{
	struct ControllableInterface
	{
		struct AdditionalInfo
		{
			TimeMoment occurrenceTime;
			ui32 deviceIndex;  //  devices of the same type( mouse, keyboard, etc. ) will use different indexes( an index can be viewed as a device's number )
		#ifdef WINDOWS
			HWND hwnd;
		#endif
		};

		virtual ~ControllableInterface() {}

		//  returned true means that the input message was devoured
		virtual bool KeyDown( VKeys::vkey_t key, const AdditionalInfo &info ) = 0;
		virtual bool KeyUp( VKeys::vkey_t key, const AdditionalInfo &info ) = 0;
		virtual bool MouseMove( f32 x, f32 y, const AdditionalInfo &info ) = 0;  //  relative
		virtual bool MouseScroll( f32 delta, const AdditionalInfo &info ) = 0;
		virtual bool TouchpadDown( ui32 &index, const AdditionalInfo &info ) = 0;
		virtual bool TouchpadMove( ui32 index, f32 x, f32 y, const AdditionalInfo &info ) = 0;  //  relative
		virtual bool TouchpadUp( ui32 index, const AdditionalInfo &info ) = 0;
	};
}

#endif