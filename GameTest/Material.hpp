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

		bool SetF32( const char *name, f32 value, ui32 offset = 0 );
		bool SetF32( uni_id id, f32 value, ui32 offset = 0 );
		bool SetF32Array( const char *name, const f32 *values, ui32 count, ui32 offset = 0 );
		bool SetF32Array( uni_id id, const f32 *values, ui32 count, ui32 offset = 0 );

		bool SetI32( const char *name, i32 value, ui32 offset = 0 );
		bool SetI32( uni_id id, i32 value, ui32 offset = 0 );
		bool SetI32Array( const char *name, const i32 *values, ui32 count, ui32 offset = 0 );
		bool SetI32Array( uni_id id, const i32 *values, ui32 count, ui32 offset = 0 );

		bool SetUI32( const char *name, ui32 value, ui32 offset = 0 );
		bool SetUI32( uni_id id, ui32 value, ui32 offset = 0 );
		bool SetUI32Array( const char *name, const ui32 *values, ui32 count, ui32 offset = 0 );
		bool SetUI32Array( uni_id id, const ui32 *values, ui32 count, ui32 offset = 0 );

		bool SetBoolean( const char *name, bool value, ui32 offset = 0 );
		bool SetBoolean( uni_id id, bool value, ui32 offset = 0 );
		bool SetBooleanArray( const char *name, const bool *values, ui32 count, ui32 offset = 0 );
		bool SetBooleanArray( uni_id id, const bool *values, ui32 count, ui32 offset = 0 );

		//  TODO: texture, sampler
	};
}

#endif