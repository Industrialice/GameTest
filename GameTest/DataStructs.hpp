#ifndef __DATA_STRUCTS_HPP__
#define __DATA_STRUCTS_HPP__

namespace GameTest
{
	class Renderer;
	Renderer *CurrentRenderer();

	enum class TextureFmt
	{
		a8r8g8b8, b8r8g8a8,
		r8g8b8, b8g8r8,
		r5g6b5, b5g6r5,
		a4r4g4b4, b4g4r4a4,
		a16r16g16b16_float, b16g16r16a16_float,
		a32r32g32b32_float, b32g32r32a32_float,
		bc1, bc2, bc3, bc4, bc5, bc6, bc7,
		etc1, etc2,
		undefined
	};

	enum class RenderingQueue : ui8
	{
		Background,
		Opaque,
		AlphaTest,
		Transparent,
		Foreground
	};

	struct DisplayInfo
	{
		ui32 width, height;
		ui32 colorDepth;
		ui32 refreshRate;
		ui32 systemDisplayNumber;
		i32 positionX, positionY;  //  the main monitor will have 0;0, the monitor on the left will have -1;0, the monitor directly above 0;1
	};

	struct GameViewPort
	{
		f32 x = 0.f, y = 0.f;
		f32 width = 1.f, height = 1.f;
	};

	class RendererBackendData
	{
		friend class Renderer;

		mutable void *_rendererData = nullptr;
		mutable bln _is_updatedByFrontEnd = true;
		mutable bln _is_childrenUpdatedByFrontEnd = false;

	protected:
		void DestroyRendererBackendData( class Renderer &currentRenderer ) const;
		void RendererBackendDataMayBeDirty();
	};

	enum class RendererBackendDataType
	{
		Undefined,
		Window,
		Texture,
		RenderTarget
	};
}

#endif