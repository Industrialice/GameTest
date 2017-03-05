#ifndef __COMPONENT_LIGHT_SOURCE_HPP__
#define __COMPONENT_LIGHT_SOURCE_HPP__

#include "ComponentsBase.hpp"

namespace GameTest::Components
{
	class LightSource : public Component
	{
	public:
		virtual std::type_index TypeID() const override
		{
			return typeid(LightSource);
		}
	};
}

#endif