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

		static bool IsTypeExclusive()
		{
			return true;
		}

		virtual bool IsExclusive() const override
		{
			return IsTypeExclusive();
		}
	};
}

#endif