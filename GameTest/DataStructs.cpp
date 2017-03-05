#include "PreHeader.hpp"
#include "DataStructs.hpp"
#include "Renderer.hpp"

void GameTest::RendererBackendData::DestroyRendererBackendData( class Renderer &currentRenderer ) const
{
	currentRenderer.FreeBackendData( *this );
	_is_updatedByFrontEnd = true;
}

void GameTest::RendererBackendData::RendererBackendDataMayBeDirty()
{
	_is_updatedByFrontEnd = true;
}