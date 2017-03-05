#include "PreHeader.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "WorldManager.hpp"
#include "WorldObject.hpp"

void *GameTest::Renderer::GetRendererBackendData( class RendererBackendData &object )
{
	return object._rendererData;
}

void GameTest::Renderer::SetRendererBackendData( class RendererBackendData &object, void *data )
{
	object._rendererData = data;
}

bln GameTest::Renderer::GetRendererBackendState( class RendererBackendData &object )
{
	return object._is_updatedByFrontEnd;
}

void GameTest::Renderer::SetRendererBackendState( class RendererBackendData &object, bln is_changed )
{
	object._is_updatedByFrontEnd = is_changed;
}

void GameTest::Renderer::FreeBackendData( WorldObject &object )  //  TODO:
{
}

void GameTest::Renderer::FreeBackendData( const RendererBackendData &data )
{
	this->FreeRendererBackendData( data._rendererData );
	data._rendererData = nullptr;
}

void GameTest::Renderer::FrustumCullScene( CRefVec < WorldManager * > worldManagers, const Components::Camera &camera, std::vector < RendererBackendData > &visibleObjects )
{
	visibleObjects.clear();

	for( const auto &worldManager : worldManagers )
	{
		for( const auto &object : worldManager->Objects() )
		{
			visibleObjects.push_back( *object );
		}
	}
}