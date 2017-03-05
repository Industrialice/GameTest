#ifndef __COMPONENT_RIGID_BODY_HPP__
#define __COMPONENT_RIGID_BODY_HPP__

#include "ComponentsBase.hpp"

namespace GameTest::Components
{
	class RigidBody : public Component
	{
	public:
		virtual std::type_index TypeID() const override
		{
			return typeid(RigidBody);
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