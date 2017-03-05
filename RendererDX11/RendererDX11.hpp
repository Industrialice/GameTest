#ifndef __RENDERER_DX11_HPP__
#define __RENDERER_DX11_HPP__

#include <Renderer.hpp>
#include <d3d11.h>

namespace GameTest
{
	class RendererDX11 : public Renderer
	{
	public:
		virtual D3D_FEATURE_LEVEL FeatureLevel() = 0;
		virtual D3D_FEATURE_LEVEL MaxSupportedFeatureLevel() = 0;
	};

	RendererDX11 *CreateRendererDX11( D3D_FEATURE_LEVEL maxFeatureLevel = D3D_FEATURE_LEVEL_11_0, ui32 adaptersMask = ui32_max, Renderer::rendererError *error = 0 );
	void DestroyRendererDX11( RendererDX11 *renderer );
}

#endif