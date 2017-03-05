#pragma once

#include "DataStructs.hpp"

namespace GameTest
{
	namespace Components
	{
		class Camera;
	}

	class WorldObject;

	class Renderer
	{
		Renderer &operator = ( const Renderer & ) = delete;
		Renderer( const Renderer & ) = delete;

	protected:
		void *GetRendererBackendData( class RendererBackendData &object );
		void SetRendererBackendData( class RendererBackendData &object, void *data );
		bool GetRendererBackendState( class RendererBackendData &object );
		void SetRendererBackendState( class RendererBackendData &object, bool is_changed );
		virtual void FreeRendererBackendData( void *data ) = 0;

	public:
		using rendererError = CError < const char * >;

		virtual ~Renderer() {}
		Renderer() = default;

		virtual void NotifyWindowResized( class GameWindow *window ) = 0;

		/*virtual CCRefVec < DisplayInfo > CurrentDisplaysGet() = 0;
		virtual std::shared_ptr < class GameWindow > CurrentDisplaysSet( const CCRefVec < DisplayInfo > &targetDisplays, rendererError *error = 0 ) = 0;
		virtual CCRefVec < DisplayInfo > AvailibleDisplaysGet() = 0;*/

		virtual const struct TextureInfo &LoadedTexInfo( const class Texture *texture ) = 0;
		virtual struct LockedTextureMemory LockTextureMips( class Texture *texture, ui32 firstMipToLock, ui32 lastMipToLock ) = 0;
		virtual void UnlockTextureMips( class Texture *texture ) = 0;
		virtual bool GenerateTextureMipChain( class Texture *texture ) = 0;
		virtual bool SetTextureTopMipData( class Texture *texture, const void *data, ui32 stride, TextureFmt format ) = 0;

		virtual bool BeginFrame() = 0;
		virtual bool EndFrame() = 0;  //  will present the final frame to the target windows
		virtual void RenderScene( CRefVec < std::shared_ptr < class WorldManager > > worldManagers, const class Components::Camera &camera ) = 0;

		void FreeBackendData( WorldObject &object );
		void FreeBackendData( const RendererBackendData &data );

		void FrustumCullScene( CRefVec < WorldManager * > worldManagers, const Components::Camera &camera, std::vector < RendererBackendData > &visibleObjects );
	};
}