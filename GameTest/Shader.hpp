#ifndef __SHADER_HPP__
#define __SHADER_HPP__

#include "DataStructs.hpp"

namespace GameTest
{
	class Shader : RendererBackendData
	{
		std::unique_ptr < byte > _uniformDefaultValues;
		ui32 _uniformsBytesSize = 0;
		ResourcePath _path;

		friend class Material;

	public:
		~Shader()
		{
			DestroyRendererBackendData( *CurrentRenderer() );
		}

		const ResourcePath &ResourcePathGet() const
		{
			return _path;
		}

		void ResourcePathSet( const ResourcePath &path )
		{
			RendererBackendDataMayBeDirty();
			_path = path;
		}
	};
}

#endif