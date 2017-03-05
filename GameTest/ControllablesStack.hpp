#ifndef __CONTROLS_STACK_HPP__
#define __CONTROLS_STACK_HPP__

#include "ControllableInterface.hpp"
#include <CVector.hpp>

namespace GameTest
{
	class ControllablesStack final : public ControllableInterface
	{
		vector < ControllableInterface * > _listenersStack;

	public:
		virtual bool KeyDown( VKeys::vkey_t key, const AdditionalInfo &info ) override;
		virtual bool KeyUp( VKeys::vkey_t key, const AdditionalInfo &info ) override;
		virtual bool MouseMove( f32 x, f32 y, const AdditionalInfo &info ) override;
		virtual bool MouseScroll( f32 delta, const AdditionalInfo &info ) override;
		virtual bool TouchpadDown( ui32 &index, const AdditionalInfo &info ) override;
		virtual bool TouchpadMove( ui32 index, f32 x, f32 y, const AdditionalInfo &info ) override;
		virtual bool TouchpadUp( ui32 index, const AdditionalInfo &info ) override;

		void AddListener( ControllableInterface *iface );  //  adding existent listener means removing existent listener from its current position on the stack and placing it on the top of the stack
		void RemoveListener( ControllableInterface *iface );
	};
}

#endif
