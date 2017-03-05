#ifndef __WORLD_MANAGER_HPP__
#define __WORLD_MANAGER_HPP__

#include "GameTest.hpp"

namespace GameTest
{
	class WorldManager
	{
		vector < std::shared_ptr < class WorldObject > > _objects;
		string _name;

		void AttachObject( const std::shared_ptr < class WorldObject > &object )
		{
			_objects.push_back( object );
		}

		void DetachObject( class WorldObject &object )
		{
			auto it = std::find_if( _objects.begin(), _objects.end(), [&object](const auto &other) { return other.get() == &object; } );
			if( it == _objects.end() )
			{
				SENDLOG( error, "Trying to remove an object that isn't attached to this world\n" );
				return;
			}

			_objects.erase( it );
		}

		friend class WorldObject;

	public:

		~WorldManager();

		vector < std::shared_ptr < class WorldObject > > CollectChildren( const WorldObject &object );
		void CollectChildren( const WorldObject &object, void *container, void (*AddChild)( void *container, const std::shared_ptr < class WorldObject > &child ) );

		const decltype(_objects) &Objects() const
		{
			return _objects;
		}

		const string &Name() const
		{
			return _name;
		}

		void Name( const char *name )
		{
			_name = name;
		}
	};
}

#endif