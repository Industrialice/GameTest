#include "PreHeader.hpp"
#include "ComponentsBase.hpp"
#include "WorldObject.hpp"

using namespace GameTest::Components;

void Component::IsEnabled( bool is_enabled )
{
	_is_enabled = is_enabled;
}

bool Component::IsEnabled() const
{
	return _is_enabled;
}

bool Component::IsExclusive() const
{
	return IsTypeExclusive();
}

bool Component::IsTypeExclusive()
{
	return false;
}