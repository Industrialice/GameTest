#ifndef __COMPONENT_AUDIO_SOURCE_HPP__
#define __COMPONENT_AUDIO_SOURCE_HPP__

#include "ComponentsBase.hpp"

namespace GameTest::Components
{
	class AudioSource : public Component
	{
	public:
		virtual std::type_index TypeID() const override
		{
			return typeid(AudioSource);
		}
	};
}

#endif