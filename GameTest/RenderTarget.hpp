#ifndef __RENDER_TARGET_HPP__
#define __RENDER_TARGET_HPP__

#include "Texture.hpp"
#include "Window.hpp"

namespace GameTest
{
	class RenderTarget : public RendererBackendData
	{
		RenderTarget &operator = ( const RenderTarget & ) = delete;
		RenderTarget( const RenderTarget & ) = delete;

	public:
		enum class TargetType { Undefined, Texture, Window };

	private:
		TargetType _colorTargetType = TargetType::Undefined;
		std::shared_ptr < void > _colorTarget;  //  can be either a GameWindow, or a Texture
		std::shared_ptr < Texture > _depthStencilTarget;

	public:
		~RenderTarget()
		{
			DestroyRendererBackendData( *CurrentRenderer() );
		}

		RenderTarget() = default;

		void ColorTarget( std::nullptr_t )
		{
			_colorTargetType = TargetType::Undefined;
			_colorTarget = nullptr;
		}

		void ColorTarget( const std::shared_ptr < GameWindow > &window )
		{
			if( window == nullptr )
			{
				_colorTargetType = TargetType::Undefined;
			}
			else
			{
				_colorTargetType = TargetType::Window;
			}

			_colorTarget = window;
			RendererBackendDataMayBeDirty();
		}

		void ColorTarget( const std::shared_ptr < Texture > &texture )
		{
			if( texture == nullptr )
			{
				_colorTargetType = TargetType::Undefined;
			}
			else
			{
				_colorTargetType = TargetType::Texture;
			}
				
			_colorTarget = texture;
			RendererBackendDataMayBeDirty();
		}

		TargetType ColorTargetType() const
		{
			return _colorTargetType;
		}

		std::shared_ptr < void > ColorTarget() const
		{
			return _colorTarget;
		}

		std::shared_ptr < GameWindow > ColorTargetAsWindow() const
		{
			if( _colorTargetType != TargetType::Window )
			{
				SOFTBREAK;
				return nullptr;
			}
			return std::static_pointer_cast < GameWindow >( _colorTarget );
		}

		std::shared_ptr < Texture > ColorTargetAsTexture() const
		{
			if( _colorTargetType != TargetType::Texture )
			{
				SOFTBREAK;
				return nullptr;
			}
			return std::static_pointer_cast < Texture >( _colorTarget );
		}

		void DepthStencilTarget( const std::shared_ptr < Texture > &target )
		{
			_depthStencilTarget = target;
			RendererBackendDataMayBeDirty();
		}

		const std::shared_ptr < const Texture > &DepthStencilTarget() const
		{
			return _depthStencilTarget;
		}
	};
}

#endif