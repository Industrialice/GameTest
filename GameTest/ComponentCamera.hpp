#ifndef __CAMERA_COMPONENT_HPP__
#define __CAMERA_COMPONENT_HPP__

#include "ComponentsBase.hpp"
#include "RenderTarget.hpp"

namespace GameTest::Components
{
	//  you're allowed to have different cameras attached to the same object, they may have different paraments and render targets
	class Camera : public Component
	{
	public:
		enum { MaxRenderTargets = 8 };
		enum class ColorClearType { None, SolidColor, Skybox };
		enum class DepthClearType { None, OverwriteByValue };

	private:
		std::shared_ptr < RenderTarget > _rts[ MaxRenderTargets ];  //  must be same sized
		f32 _horizontalFov;  //  we need to know aspect ratio to convert between horizontal and vertical FOVs, the camera isn't attached to any aspect ratio, so it is reasonable to pick only one FOV dimension here
		f32 _nearPlane;
		f32 _farPlane;
		f96color _clearColor;
		ColorClearType _colorClearType;
		f32 _depthOverwriteValue;
		DepthClearType _depthClearType;
		GameViewPort _viewPort;

	public:
		virtual std::type_index TypeID() const override
		{
			return typeid(Camera);
		}

		Camera()
		{
			_horizontalFov = 75.f;
			_nearPlane = 0.1f;
			_farPlane = 1000.f;
			_clearColor = f96color( 1.f, 1.f, 1.f );
			_colorClearType = ColorClearType::SolidColor;
			_depthOverwriteValue = 1.f;
			_depthClearType = DepthClearType::OverwriteByValue;
		}

		void HorizontalFOVSet( f32 fov )
		{
			_horizontalFov = fov;
		}

		f32 HorizontalFOVGet() const
		{
			return _horizontalFov;
		}

		void NearPlaneSet( f32 value )
		{
			_nearPlane = value;
		}

		f32 NearPlaneGet() const
		{
			return _nearPlane;
		}

		void FarPlaneSet( f32 value )
		{
			_farPlane = value;
		}

		f32 FarPlaneGet() const
		{
			return _farPlane;
		}

		void ClearColorSet( const f96color &color )
		{
			_clearColor = color;
		}

		const f96color &ClearColorGet() const
		{
			return _clearColor;
		}

		void ColorClearTypeSet( ColorClearType type )
		{
			ASSUME( type != ColorClearType::Skybox );  //  Skybox isn't supported yet
			_colorClearType = type;
		}
		
		ColorClearType ColorClearTypeGet() const
		{
			return _colorClearType;
		}

		void DepthOverwriteValueSet( f32 value )
		{
			_depthOverwriteValue = value;
		}

		f32 DepthOverwriteValueGet() const
		{
			return _depthOverwriteValue;
		}

		void DepthClearTypeSet( DepthClearType type )
		{
			_depthClearType = type;
		}

		DepthClearType DepthClearTypeGet() const
		{
			return _depthClearType;
		}

		void ViewPortSet( const GameViewPort &vp )
		{
			_viewPort = vp;
		}

		const GameViewPort &ViewPortGet() const
		{
			return _viewPort;
		}

		void RenderTargetSet( ui32 index, const std::shared_ptr < RenderTarget > &rt )
		{
			if( index >= MaxRenderTargets )
			{
				SOFTBREAK;
				return;
			}
			_rts[ index ] = rt;
		}

		std::shared_ptr < RenderTarget > RenderTargetGet( ui32 index ) const
		{
			if( index >= MaxRenderTargets )
			{
				SOFTBREAK;
				return nullptr;
			}
			return _rts[ index ];
		}
	};
}

#endif