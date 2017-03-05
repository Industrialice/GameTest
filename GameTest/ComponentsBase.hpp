#ifndef __COMPONENTS_BASE_HPP__
#define __COMPONENTS_BASE_HPP__

#include <typeindex>

namespace GameTest::Components
{
	class Component
	{
		bln _is_enabled = true;

	public:
		virtual void IsEnabled( bln is_enabled );
		bln IsEnabled() const;
		virtual std::type_index TypeID() const = 0;
		static bln IsTypeExclusive();
		virtual bln IsExclusive() const;  //  you can add only one exclusive component to a world object
	};
}

#endif