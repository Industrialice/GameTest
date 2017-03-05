#ifndef __COMPONENT_COLLIDER_HPP__
#define __COMPONENT_COLLIDER_HPP__

#include "ComponentsBase.hpp"

namespace GameTest::Components
{
	class Collider : public Component
	{
	public:
		virtual std::type_index TypeID() const override
		{
			return typeid(Collider);
		}

		static bln IsTypeExclusive()
		{
			return true;
		}

		virtual bln IsExclusive() const override
		{
			return IsTypeExclusive();
		}
	};
}

#endif