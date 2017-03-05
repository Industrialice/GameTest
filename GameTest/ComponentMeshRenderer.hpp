#ifndef __COMPONENT_MESH_RENDERER_HPP__
#define __COMPONENT_MESH_RENDERER_HPP__

#include "ComponentsBase.hpp"
#include "Material.hpp"

namespace GameTest::Components
{
	class MeshRenderer : public Component
	{
		std::shared_ptr < Material > _material;

	public:
		virtual std::type_index TypeID() const override
		{
			return typeid(MeshRenderer);
		}
	};
}

#endif