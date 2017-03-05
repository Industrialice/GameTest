#ifndef __SCENE_OBJECT_HPP__
#define __SCENE_OBJECT_HPP__

#include "GameTest.hpp"
#include "ComponentsBase.hpp"
#include "WorldManager.hpp"

namespace GameTest
{
	class WorldObject : public std::enable_shared_from_this < WorldObject >, public RendererBackendData
	{
		WorldObject( const WorldObject & ) = delete;
		WorldObject &operator = ( const WorldObject & ) = delete;

		vec3 _position = vec3( 0, 0, 0 );
		vec3 _rotation = vec3( 0, 0, 0 );
		vec3 _scale = vec3( 1, 1, 1 );
		WorldObject *_parent = nullptr;
		bool _is_enabled = true;
		bool _is_static = false;  //  if the object is static, you can't change any of its properties during the gameplay, changing is available only during edit time
		WorldManager *_worldManager = nullptr;
		CVec < std::unique_ptr < Components::Component >, void > _components;
		string _objectName = "WorldObject";

	protected:
		WorldObject() {}

	public:
		~WorldObject() 
		{
			DestroyRendererBackendData( *CurrentRenderer() );
		}

		static std::shared_ptr < WorldObject > New()
		{
			class Proxy : public WorldObject
			{
			public: Proxy() : WorldObject() {}
			};

			return std::make_shared < Proxy >();
		}

		static std::shared_ptr < WorldObject > New( WorldManager *worldManager, WorldObject *parent = nullptr )
		{
			class Proxy : public WorldObject
			{
			public: Proxy() : WorldObject() {}
			};

			auto ref = std::make_shared < Proxy >();

			ref->AddToWorld( worldManager, parent );

			return ref;
		}

		template < typename ComponentType > ComponentType *ComponentAdd()  //  returns nullptr if the component cannot be added
		{
			using ClearComponentType = std::remove_cv<std::remove_pointer<std::remove_reference<ComponentType>::type>::type>::type;

			if( ComponentType::IsTypeExclusive() )
			{
				std::type_index typeIndex = typeid(ClearComponentType);

				for( const auto &storedComponent : _components )
				{
					if( typeIndex == storedComponent->TypeID() )
					{
						SENDLOG( error, "Cannot add component <component_name> to the object %s because it already contains such component and the component is exclusive\n", _objectName.c_str() );
						return nullptr;
					}
				}
			}

			RendererBackendDataMayBeDirty();

			_components.EmplaceBack( new ClearComponentType );
			return (ComponentType *)_components.Back().get();
		}

		bool ComponentRemove( const Components::Component *component )
		{
			auto it = std::find_if( _components.begin(), _components.end(), [component]( const auto &stored ) { return stored.get() == component; } );
			if( it == _components.end() )
			{
				SENDLOG( error, "Trying to remove a non-existing component from the object %s\n", _objectName.c_str() );
				return false;
			}

			RendererBackendDataMayBeDirty();

			_components.Erase( it );
			return true;
		}

		CRefVec < std::unique_ptr < Components::Component > > ComponentsGet()
		{
			_components.ToRef();
		}

		CRefVec < const std::unique_ptr < const Components::Component > > ComponentsGet() const
		{
			_components.ToCRef();
		}

		template < typename ComponentType, typename Target > uiw ComponentsOfTypeGet( Target &appendTo, bool is_includeDisabled = false )
		{
			using ClearComponentType = std::remove_cv<std::remove_pointer<std::remove_reference<ComponentType>::type>::type>::type;

			uiw appended = 0;
			std::type_index typeIndex = typeid(ClearComponentType);

			for( auto &stored : _components )
			{
				if( is_includeDisabled || stored->IsEnabled() )
				{
					if( stored->TypeID() == typeIndex )
					{
						appendTo.Append( (ClearComponentType *)stored.get() );
						++appended;
					}
				}
			}
			
			return appended;
		}

		template < typename ComponentType, typename Target > uiw ComponentsOfTypeGet( Target &appendTo, bool is_includeDisabled = false ) const
		{
			using ClearComponentType = std::remove_cv<std::remove_pointer<std::remove_reference<ComponentType>::type>::type>::type;

			uiw appended = 0;
			std::type_index typeIndex = typeid(ClearComponentType);

			for( const auto &stored : _components )
			{
				if( is_includeDisabled || stored->IsEnabled() )
				{
					if( stored->TypeID() == typeIndex )
					{
						appendTo.Append( (ClearComponentType *)stored.get() );
						++appended;
					}
				}
			}

			return appended;
		}

		const string &Name() const
		{
			return _objectName;
		}

		void Name( const char *name )
		{
			_objectName = name;
		}

		WorldObject *Parent() const
		{
			return _parent;
		}

		bool IsStatic() const
		{
			return _is_static;
		}

		void IsStatic( bool is_static )
		{
			_is_static = is_static;
		}

		bool IsEnabledSelf() const
		{
			return _is_enabled;
		}

		void IsEnabledSelf( bool is_enabled )
		{
			RendererBackendDataMayBeDirty();
			_is_enabled = is_enabled;
		}

		bool IsEnabledInHierarchy() const
		{
			return _is_enabled && (_parent == nullptr || _parent->IsEnabledInHierarchy());
		}

		WorldManager *WorldManager() const
		{
			return _worldManager;
		}

		//  will also affect all of the object's children, they're going to be either transferred together with the object, or removed from the world( also with the object )
		void AddToWorld( class WorldManager *worldManager, WorldObject *parent )  //  pass nullptr to remove the object from any world, also used to change parents for objects in the same world
		{
			if( (worldManager == nullptr) && (parent != nullptr) )
			{
				SENDLOG( error, "Trying to set parent without specifying a world manager, object %s\n", _objectName.c_str() );
				parent = nullptr;
			}
			else if( parent == this )
			{
				SENDLOG( error, "Trying to set parent to self, object %s\n", _objectName.c_str() );
				parent = nullptr;
			}
			else if( parent != nullptr && parent->WorldManager() != worldManager )
			{
				SENDLOG( error, "Trying to add object to a parent with different world manager, object %s\n", _objectName.c_str() );
				parent = nullptr;
			}

			class WorldManager *oldWorldManager = _worldManager;

			_worldManager = worldManager;
			_parent = parent;

			if( oldWorldManager != worldManager )
			{
				CVec < std::shared_ptr < WorldObject >, void, 16 > children;
				auto this_ptr = shared_from_this();

				if( worldManager != nullptr )
				{
					worldManager->AttachObject( this_ptr );
				}

				if( oldWorldManager != nullptr )
				{
					auto callback = []( void *container, const std::shared_ptr < WorldObject > &child )
					{
						((CVec < std::shared_ptr < WorldObject >, void, 16 > *)container)->Append( child );
					};

					oldWorldManager->CollectChildren( *this, &children, callback );

					for( const auto &object : children )
					{
						object->AddToWorld( worldManager, (worldManager ? this : nullptr) );
					}

					oldWorldManager->DetachObject( *this );
				}
			}
			
			RendererBackendDataMayBeDirty();
		}
	};
}

#endif