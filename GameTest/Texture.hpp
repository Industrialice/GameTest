#pragma once

#include "DataStructs.hpp"
#include "ResourcePath.hpp"
#include "Renderer.hpp"

namespace GameTest
{
	enum class TextureType
	{
		tex2D, tex3D, texCube, tex2DNormalMap, tex2DSpecularMap, tex2DDepthStencil
	};

	struct TextureParams
	{
		enum class Importance { vlow, low, norm, high, vhigh };

		Nullable < bool > is_allowSizeOptimization;
		Nullable < bool > is_allowFormatOptimization;
		Nullable < bool > is_allowMipGeneration;
		Nullable < bool > is_allowCPURead;
		Nullable < bool > is_allowCPUWrite;
		Nullable < bool > is_allowGPUUnorderedRead;
		Nullable < bool > is_allowGPUUnorderedWrite;
		Nullable < bool > is_allowGPURenderTarget;
		bool is_allowGPUShaderResource = true;
		Nullable < bool > is_makeDynamic;  //  will be often written by the CPU, only valid when is_allowCPUWrite is true
		Importance importance = Importance::norm;  //  renderer can use this flag to decide if it should, for example, decrease resolution or unload mip levels if there's not enough memory
	};

	struct TextureInfo
	{
		ui32 width = 0, height = 0;
		ui32 mipLevels = 0;
		TextureFmt format = TextureFmt::undefined;
		TextureType type = TextureType::tex2D;
	};

	//  use LoadedTexInfo to determine width, height and format
	struct LockedTextureMemory
	{
		void *memory = nullptr;  //  will be nullptr if locked memory is undefined( LockMemory can fail )
		ui32 firstLockedMip, lastLockedMip;  //  if you've locked just the top mip, these values will be 0 and 0
		ui32 stride;
	};

	class Texture : public RendererBackendData
	{
	public:
		~Texture()
		{
			DestroyRendererBackendData( *CurrentRenderer() );
		}

		Texture( const Texture & ) = delete;
		Texture &operator = ( const Texture & ) = delete;

		Texture()
		{}

		const ResourcePath &ResourcePathGet() const
		{
			return _path;
		}

		void ResourcePathSet( const ResourcePath &path )
		{
			RendererBackendDataMayBeDirty();
			_path = path;
		}

		const TextureParams &TextureParamsGet() const
		{
			return _params;
		}

		void TextureParamsSet( const TextureParams &params )
		{
			RendererBackendDataMayBeDirty();
			_params = params;
		}

		const TextureInfo &SourceTexInfoGet() const
		{
			return _sourceInfo;
		}

		//  just leave the fields you want to ignore in their default state - they will be acquired from _path
		//  if _path is undefined, all fields must be filled when you call Create
		void SourceTexInfoSet( const TextureInfo &info )
		{
			RendererBackendDataMayBeDirty();
			_sourceInfo = info;
		}

		/*const TextureInfo &LoadedTexInfo() const
		{
			return _renderer->LoadedTexInfo( this );
		}

		LockedTextureMemory LockMips( ui32 firstMipToLock, ui32 lastMipToLock )  //  if you want to lock only the top level, provide 0 and 0
		{
			if( _renderer == nullptr )
			{
				SOFTBREAK;
				return LockedTextureMemory();
			}
			return _renderer->LockTextureMips( this, firstMipToLock, lastMipToLock );
		}

		void UnlockMips()
		{
			if( _renderer == nullptr )
			{
				SOFTBREAK;
				return;
			}
			return _renderer->UnlockTextureMips( this );
		}

		bool GenerateMipChain()
		{
			if( _renderer == nullptr )
			{
				SOFTBREAK;
				return false;
			}
			return _renderer->GenerateTextureMipChain( this );
		}

		bool SetTopMipData( const void *data, ui32 stride, TextureFmt format )
		{
			if( _renderer == nullptr )
			{
				SOFTBREAK;
				return false;
			}
			return _renderer->SetTextureTopMipData( this, data, stride, format );
		}*/

	private:
		TextureParams _params;
		TextureInfo _sourceInfo;
		ResourcePath _path;
	};
}