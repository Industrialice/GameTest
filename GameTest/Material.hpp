#ifndef __MATERIAL_HPP__
#define __MATERIAL_HPP__

#include "Texture.hpp"
#include "Shader.hpp"

namespace GameTest
{
	class Material : RendererBackendData
	{
		std::shared_ptr < Shader > _shader;
		RenderingQueue _renderingQueue;

	public:
		~Material()
		{
			DestroyRendererBackendData( *CurrentRenderer() );
		}

		using uni_id = i32;

		uni_id UniformNameToId( const char *uniformName );  //  -1 if name does not exist
		const char *UniformIdToName( uni_id id );  //  null if id does not exist

		bln SetF32( const char *name, f32 value, ui32 offset = 0 );
		bln SetF32( uni_id id, f32 value, ui32 offset = 0 );
		bln SetF32Array( const char *name, const f32 *values, ui32 count, ui32 offset = 0 );
		bln SetF32Array( uni_id id, const f32 *values, ui32 count, ui32 offset = 0 );

		bln SetI32( const char *name, i32 value, ui32 offset = 0 );
		bln SetI32( uni_id id, i32 value, ui32 offset = 0 );
		bln SetI32Array( const char *name, const i32 *values, ui32 count, ui32 offset = 0 );
		bln SetI32Array( uni_id id, const i32 *values, ui32 count, ui32 offset = 0 );

		bln SetUI32( const char *name, ui32 value, ui32 offset = 0 );
		bln SetUI32( uni_id id, ui32 value, ui32 offset = 0 );
		bln SetUI32Array( const char *name, const ui32 *values, ui32 count, ui32 offset = 0 );
		bln SetUI32Array( uni_id id, const ui32 *values, ui32 count, ui32 offset = 0 );

		bln SetBoolean( const char *name, bln value, ui32 offset = 0 );
		bln SetBoolean( uni_id id, bln value, ui32 offset = 0 );
		bln SetBooleanArray( const char *name, const bln *values, ui32 count, ui32 offset = 0 );
		bln SetBooleanArray( uni_id id, const bln *values, ui32 count, ui32 offset = 0 );

		//  TODO: texture, sampler
	};
}

#endif