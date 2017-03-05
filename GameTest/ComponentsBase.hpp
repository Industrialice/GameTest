#ifndef __COMPONENTS_BASE_HPP__
#define __COMPONENTS_BASE_HPP__

#include <typeindex>

namespace GameTest::Components
{
	class Component
	{
		bool _is_enabled = true;

	public:
		virtual void IsEnabled( bool is_enabled );
		bool IsEnabled() const;
		virtual std::type_index TypeID() const = 0;
		static bool IsTypeExclusive();
		virtual bool IsExclusive() const;  //  you can add only one exclusive component to a world object
	};
}

#endif