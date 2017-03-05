#include "PreHeader.hpp"
#include "ComponentsBase.hpp"
#include "WorldObject.hpp"

using namespace GameTest::Components;

void Component::IsEnabled( bln is_enabled )
{
	_is_enabled = is_enabled;
}

bln Component::IsEnabled() const
{
	return _is_enabled;
}

bln Component::IsExclusive() const
{
	return IsTypeExclusive();
}

bln Component::IsTypeExclusive()
{
	return false;
}