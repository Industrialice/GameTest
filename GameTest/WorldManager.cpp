#include "PreHeader.hpp"
#include "WorldManager.hpp"
#include "WorldObject.hpp"

using namespace GameTest;

WorldManager::~WorldManager()
{
	for( const auto &it : _objects )
	{
		it->AddToWorld( nullptr, nullptr );  //  somebody may hold references to some of these objects, so we reset their world manager and parent parameters
	}
}

vector < std::shared_ptr < WorldObject > > WorldManager::CollectChildren( const WorldObject &object )
{
	vector < std::shared_ptr < WorldObject > > collected;

	for( const auto &it : _objects )
	{
		if( it->Parent() == &object )
		{
			ASSUME( it.get() != &object );
			collected.push_back( it );
		}
	}

	return collected;
}

void WorldManager::CollectChildren( const WorldObject &object, void *container, void (*AddChild)( void *container, const std::shared_ptr < class WorldObject > &child ) )
{
	for( const auto &it : _objects )
	{
		if( it->Parent() == &object )
		{
			ASSUME( it.get() != &object );
			AddChild( container, it );
		}
	}
}