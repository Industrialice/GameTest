#include "PreHeader.hpp"
#include "RendererDX11.hpp"
#include <GameTestWindows.hpp>
#include <Texture.hpp>
#include <MTMessageQueue.hpp>
#include <atomic>
#include <ComponentCamera.hpp>
#include <COMSharedPtr.hpp>
#include <WorldManager.hpp>

static const char *ConvertDX11ErrToString( HRESULT hresult );

using namespace GameTest;

//  TODO: viewports handling
class RendererDX11Impl final : public RendererDX11
{
	COMSharedPtr < ID3D11Device > _device;
	COMSharedPtr < ID3D11DeviceContext > _immContext;
	D3D_FEATURE_LEVEL _featureLevel, _maxSupportedFeatureLevel;
	MTMessageQueue _globalMessageQueue;
	bln _is_begunFrame = false;
	std::thread _rendererMainThread;
	std::atomic_bool _is_countinueMainThreadPolling;
	std::mutex _endFrameMutex;
	std::condition_variable _endFrameCV;
	std::atomic_bool _is_frameEnded;
	vector < COMSharedPtr < IDXGISwapChain > > _swapChainsToPresent;
	bln _is_waitingForWindowResizeNotification = false;
	vector < void * > _objectsToRender;

	struct RendererBackendDataTypeSpecifier
	{
		RendererBackendDataType type = RendererBackendDataType::Undefined;
	};

	struct PrivateWindowData : RendererBackendDataTypeSpecifier
	{
		HWND hwnd = NULL;
		ui32 width = 0, height = 0;
		bln is_fullscreen = false;
		COMSharedPtr < IDXGISwapChain > swapChain;
		COMSharedPtr < ID3D11RenderTargetView > RTV;
		ui32 rtvWidth = 0, rtvHeight = 0;

		PrivateWindowData( RendererBackendDataType type )
		{
			this->type = type;
		}
	};

	struct SceneInfo
	{
		CVec < COMSharedPtr < ID3D11RenderTargetView >, void, 8 > RTVs;
		f96color clearColor;
		Components::Camera::ColorClearType colorClearType;
		f32 depthOverwriteValue;
		Components::Camera::DepthClearType depthClearType;
		GameViewPort viewPort;
	};

public:
	virtual ~RendererDX11Impl()
	{
		ASSUME( _is_begunFrame == false );
		_is_countinueMainThreadPolling = false;
		_globalMessageQueue.StopWorkWaiters();
		_rendererMainThread.join();
	}
	
	RendererDX11Impl( D3D_FEATURE_LEVEL maxFeatureLevel, ui32 adaptersMask, rendererError *error )
	{
		ASSUME( error && _is_frameEnded.is_lock_free() && _is_countinueMainThreadPolling.is_lock_free() );

		COMSharedPtr < IDXGIFactory1 > dxgiFactory;
		HRESULT dxgiFactoryResult = CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void **)dxgiFactory.AddressOf() );
		if( FAILED( dxgiFactoryResult ) )
		{
			*error = rendererError( Error::UnknownError(), "failed to create DXGI factory" );
			return;
		}

		ASSUME( dxgiFactory != nullptr );

		vector < COMSharedPtr < IDXGIAdapter > > adapters;
		for( ui32 index = 0; index < 32; ++index )
		{
			if( ((1 << index) & adaptersMask) == 0 )
			{
				continue;
			}

			IDXGIAdapter *adapter;
			if( dxgiFactory->EnumAdapters( index, &adapter ) == DXGI_ERROR_NOT_FOUND )
			{
				break;
			}

			adapters.emplace_back( adapter );
		}

		if( adapters.empty() )
		{
			*error = rendererError( Error::UnknownError(), "adapters list is empty, try another adaptersMask" );
			return;
		}

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

		#if defined(DEBUG)
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		vector < D3D_FEATURE_LEVEL > featureLevels { D3D_FEATURE_LEVEL_9_1, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };
		for( uiw index = 1; index < featureLevels.size(); ++index )
		{
			if( featureLevels[ index ] > maxFeatureLevel )
			{
				featureLevels.resize( index );
				break;
			}
		}

		HRESULT result = D3D11CreateDevice( 
			adapters[ 0 ].Get(), 
			D3D_DRIVER_TYPE_UNKNOWN, 
			NULL, 
			createDeviceFlags, 
			featureLevels.data(),
			featureLevels.size(),
			D3D11_SDK_VERSION, 
			_device.AddressOf(),
			&_featureLevel,
			_immContext.AddressOf() );

		if( FAILED( result ) )
		{
			*error = rendererError( Error::UnknownError(), "failed to create device" );
			return;
		}

		_maxSupportedFeatureLevel = _device->GetFeatureLevel();  //  TODO: this can be off

		ASSUME( _device != nullptr && _immContext != nullptr );

		*error = Error::Ok();
	}

	void HandleWindowSizeFullScreenChanges( GameWindow *window )
	{
		HWND prevFocus = NULL;

		PrivateWindowData *pwd = (PrivateWindowData *)Renderer::GetRendererBackendData( *window );

		_is_waitingForWindowResizeNotification = true;

		DXGI_MODE_DESC mode;
		mode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mode.Width = window->Width();
		mode.Height = window->Height();
		mode.RefreshRate.Denominator = 0;
		mode.RefreshRate.Numerator = 0;
		mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		if( pwd->is_fullscreen != window->IsFullscreen() || pwd->width != window->Width() || pwd->height != window->Height() )
		{
			pwd->RTV.Release();

			if( HRESULT hresult = pwd->swapChain->ResizeTarget( &mode ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( HandleWindowSizeFullScreenChanges ): swap chain's ResizeTarget failed, error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			}
			else
			{
				SENDLOG( info, "Renderer DX11( CreateWindowAssociation ): called ResizeTarget, size %u x %u\n", window->Width(), window->Width() );
				pwd->width = window->Width();
				pwd->height = window->Height();
			}
		}

		if( pwd->is_fullscreen != window->IsFullscreen() )
		{
			prevFocus = SetFocus( window->HWND() );

			//  will trigger a WM_SIZE event that can change current window's size
			if( HRESULT hresult = pwd->swapChain->SetFullscreenState( window->IsFullscreen() ? TRUE : FALSE, 0 ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( HandleWindowSizeFullScreenChanges ): failed to change fullscreen state of the swap chain, error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			}
			else
			{
				pwd->is_fullscreen = window->IsFullscreen();
				SENDLOG( info, "Renderer DX11( HandleWindowSizeFullScreenChanges ): changed the swap chain's fullscreen state to %s\n", pwd->is_fullscreen ? "fullscreen" : "windowed" );
			}
			
			mode.RefreshRate.Denominator = 0;
			mode.RefreshRate.Numerator = 0;
			if( HRESULT hresult = pwd->swapChain->ResizeTarget( &mode ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( HandleWindowSizeFullScreenChanges ): second swap chain's ResizeTarget failed, error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			}
			else
			{
				SENDLOG( info, "Renderer DX11( CreateWindowAssociation ): called ResizeTarget, size %u x %u\n", window->Width(), window->Height() );
			}
		}

		_is_waitingForWindowResizeNotification = false;

		CreateWindowRenderTargetView( window );

		if( prevFocus != NULL )
		{
			SetFocus( prevFocus );
		}
	}
	
	bln MakeWindowAssociation( GameWindow *window )
	{
		if( Renderer::GetRendererBackendData( *window ) == nullptr )
		{
			Renderer::SetRendererBackendData( *window, new PrivateWindowData( RendererBackendDataType::Window ) );
		}

		PrivateWindowData *pwd = (PrivateWindowData *)Renderer::GetRendererBackendData( *window );

		if( pwd->hwnd == NULL || !pwd->swapChain )  //  swap chain hasn't been created yet
		{
			ASSUME( pwd->width == 0 && pwd->height == 0 && pwd->RTV == nullptr && pwd->rtvHeight == 0 && pwd->rtvWidth == 0 );

			DXGI_SWAP_CHAIN_DESC sd;
			sd.BufferDesc.Width  = window->Width();
			sd.BufferDesc.Height = window->Height();
			sd.BufferDesc.RefreshRate.Numerator = 0;
			sd.BufferDesc.RefreshRate.Denominator = 0;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.SampleDesc.Count   = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount  = 2;
			sd.OutputWindow = window->HWND();
			sd.Windowed     = true;
			sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
			sd.Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			auto procError = [&]
			{
				SOFTBREAK;
			};

			COMSharedPtr < IDXGIDevice > dxgiDevice;
			if( HRESULT hresult = _device->QueryInterface( __uuidof(IDXGIDevice), (void**)dxgiDevice.AddressOf() ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( CreateWindowAssociation ): failed to get DXGI device with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
				procError();
				return false;
			}
	  
			COMSharedPtr < IDXGIAdapter > dxgiAdapter;
			if( HRESULT hresult = dxgiDevice->GetParent( __uuidof(IDXGIAdapter), (void**)dxgiAdapter.AddressOf() ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( CreateWindowAssociation ): failed to get DXGI adapter with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
				procError();
				return false;
			}

			COMSharedPtr < IDXGIFactory > dxgiFactory;
			if( HRESULT hresult = dxgiAdapter->GetParent( __uuidof(IDXGIFactory), (void**)dxgiFactory.AddressOf() ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( CreateWindowAssociation ): failed to get DXGI factory with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
				procError();
				return false;
			}

			if( HRESULT hresult = dxgiFactory->CreateSwapChain( _device.Get(), &sd, pwd->swapChain.AddressOf() ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( CreateWindowAssociation ): create swap chain for the current window failed with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
				procError();
				return false;
			}

			if( HRESULT hresult = dxgiFactory->MakeWindowAssociation( window->HWND(), DXGI_MWA_NO_WINDOW_CHANGES ) != S_OK )
			{
				SENDLOG( error, "Renderer DX11( CreateWindowAssociation ): failed to make window association with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			}
				
			pwd->hwnd = window->HWND();
			pwd->is_fullscreen = false;
			pwd->width = window->Width();
			pwd->height = window->Height();

			SENDLOG( info, "Renderer DX11( CreateWindowAssociation ): created a new swap chain, size %u x %u\n", window->Width(), window->Height() );
		}  //  ended creating a swap chain

		HandleWindowSizeFullScreenChanges( window );

		return true;
	}

	void CreateWindowRenderTargetView( GameWindow *window )
	{
		PrivateWindowData *pwd = (PrivateWindowData *)Renderer::GetRendererBackendData( *window );

		if( pwd->swapChain == nullptr )
		{
			return;
		}

		if( pwd->rtvWidth != pwd->width || pwd->rtvHeight != pwd->height )
		{
			pwd->RTV.Release();
		}

		if( pwd->RTV != nullptr )
		{
			return;
		}

		if( HRESULT hresult = pwd->swapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 ) != S_OK )
		{
			SENDLOG( error, "Renderer DX11( CreateWindowRenderTargetView ): failed to resize swap chain's buffers with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			return;
		}
			
		COMSharedPtr < ID3D11Texture2D > backBufferTexture;
		if( HRESULT hresult = pwd->swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void **)backBufferTexture.AddressOf() ) != S_OK )
		{
			SENDLOG( error, "Renderer DX11( CreateWindowRenderTargetView ): get back buffer for the current window's swap chain failed with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			return;
		}
			
		if( HRESULT hresult = _device->CreateRenderTargetView( backBufferTexture.Get(), 0, pwd->RTV.AddressOf() ) != S_OK )
		{
			SENDLOG( error, "Renderer DX11( CreateWindowRenderTargetView ): create render target view for the current window failed with error 0x%h %s\n", hresult, ConvertDX11ErrToString( hresult ) );
			return;
		}

		pwd->rtvWidth = pwd->width;
		pwd->rtvHeight = pwd->height;

		SENDLOG( info, "Renderer DX11( CreateWindowRenderTargetView ): created swap chain's RTV, size is %ix%i\n", pwd->rtvWidth, pwd->rtvHeight );
	}

	static void FreeWindowData( PrivateWindowData *data )
	{
		delete data;
	}

	static void CleanupBackendData( void *resourceAddress )
	{
		const RendererBackendDataTypeSpecifier &typed = *(RendererBackendDataTypeSpecifier *)resourceAddress;

		switch( typed.type )
		{
		case RendererBackendDataType::Window:
			FreeWindowData( (PrivateWindowData *)resourceAddress );
			break;
		case RendererBackendDataType::RenderTarget:
			NOT_IMPLEMENTED;
			break;
		case RendererBackendDataType::Texture:
			NOT_IMPLEMENTED;
			break;
		case RendererBackendDataType::Undefined:
		default:
			SOFTBREAK;
			SENDLOG( error, "Renderer DX11( FreeRendererBackendData ): trying to free data with incorrect type\n" );
		}
	}

	virtual void FreeRendererBackendData( void *rendererMemory ) override
	{
		if( _is_frameEnded )
		{
			CleanupBackendData( rendererMemory );
		}
		else
		{
			_globalMessageQueue.AddDelegate < CALLMETHOD(&RendererDX11Impl::CleanupBackendDataThreaded) >( this, rendererMemory );
		}
	}

	virtual void NotifyWindowResized( GameWindow *window ) override
	{
		if( !_is_waitingForWindowResizeNotification )
		{
			return;
		}

		CreateWindowRenderTargetView( window );
	}

	virtual D3D_FEATURE_LEVEL FeatureLevel() override
	{
		return _featureLevel;
	}

	virtual D3D_FEATURE_LEVEL MaxSupportedFeatureLevel() override
	{
		return _maxSupportedFeatureLevel;
	}

	/*virtual CCRefVec < DisplayInfo > CurrentDisplaysGet() override
	{
		return CCRefVec < DisplayInfo >();  //  TODO:
	}
	
	virtual std::shared_ptr < GameWindow > CurrentDisplaysSet( const CCRefVec < DisplayInfo > &targetDisplays, rendererError *error = 0 ) override
	{
		return nullptr;  //  TODO:
	}
	
	virtual CCRefVec < DisplayInfo > AvailibleDisplaysGet() override
	{
		return CCRefVec < DisplayInfo >();  //  TODO:
	}*/
	
	virtual const struct TextureInfo &LoadedTexInfo( const class Texture *texture ) override
	{
		static TextureInfo info;
		return info;  //  TODO:
	}
	
	virtual struct LockedTextureMemory LockTextureMips( class Texture *texture, ui32 firstMipToLock, ui32 lastMipToLock ) override
	{
		return LockedTextureMemory();  //  TODO:
	}
	
	virtual void UnlockTextureMips( class Texture *texture ) override
	{
		//  TODO:
	}
	
	virtual bln GenerateTextureMipChain( class Texture *texture ) override
	{
		return false;  //  TODO:
	}
	
	virtual bln SetTextureTopMipData( class Texture *texture, const void *data, ui32 stride, TextureFmt format ) override
	{
		return false;  //  TODO:
	}
		
	virtual bln BeginFrame() override
	{
		if( _is_begunFrame )
		{
			SOFTBREAK;
			return false;
		}

		_is_frameEnded = false;

		if( _rendererMainThread.get_id() == std::thread::id() )  //  thread is not started
		{
			_is_countinueMainThreadPolling = true;
			_rendererMainThread = std::thread( ThreadFunc, &_is_countinueMainThreadPolling, &_globalMessageQueue );
		}
			
		_globalMessageQueue.AddDelegate < CALLMETHOD(&RendererDX11Impl::BeginFrameThreaded) >( this );

		_is_begunFrame = true;
		return true;
	}
	
	virtual bln EndFrame() override
	{
		if( !_is_begunFrame )
		{
			SOFTBREAK;
			return false;
		}

		_globalMessageQueue.AddDelegate < CALLMETHOD(&RendererDX11Impl::EndFrameThreaded) >( this );

		std::unique_lock < std::mutex > lock( _endFrameMutex );
		_endFrameCV.wait( lock, [this] { return _is_frameEnded == true; } );

		for( auto &sc : _swapChainsToPresent )
		{
			/*PrivateWindowData *pwd = (PrivateWindowData *)_targetWindow->rendererMemory;

			D3D11_VIEWPORT vp;
			vp.Height = pwd->height;
			vp.Width = pwd->width;
			vp.MaxDepth = 1.0f;
			vp.MinDepth = 0.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			_immContext->RSSetViewports( 1, &vp );*/

			sc->Present( 0, 0 );  //  it'll return an error if the window was minimized, for example, ignore it for now
		}

		_is_begunFrame = false;
		return true;
	}

	bln UpdateWindow( GameWindow *window )  //  handles changes to windows
	{
		ASSUME( window );
		PrivateWindowData *pwd = (PrivateWindowData *)Renderer::GetRendererBackendData( *window );
		if( pwd == nullptr )
		{
			_globalMessageQueue.WaitForEmpty();

			if( !MakeWindowAssociation( window ) )
			{
				return false;
			}

			pwd = (PrivateWindowData *)Renderer::GetRendererBackendData( *window );
		}
		if( pwd->width != window->Width() || pwd->height != window->Height() || pwd->is_fullscreen != window->IsFullscreen() )
		{
			_globalMessageQueue.WaitForEmpty();

			HandleWindowSizeFullScreenChanges( window );
		}

		if( pwd->RTV == nullptr )
		{
			return false;
		}

		_swapChainsToPresent.emplace_back( pwd->swapChain );

		return true;
	}

	virtual void RenderScene( CRefVec < std::shared_ptr < WorldManager > > worldManagers, const Components::Camera &camera ) override  //  TODO: check rendertarget sizes
	{
		SceneInfo info;

		uiw swapChainsToPresentCount = _swapChainsToPresent.size();

		for( ui32 rtIndex = 0; rtIndex < Components::Camera::MaxRenderTargets; ++rtIndex )
		{
			const RenderTarget *rt = camera.RenderTargetGet( rtIndex ).get();
			if( rt == nullptr )
			{
				break;
			}

			if( rt->ColorTargetType() == RenderTarget::TargetType::Window )
			{
				GameWindow *window = rt->ColorTargetAsWindow().get();

				if( !UpdateWindow( window ) )
				{
					_swapChainsToPresent.resize( swapChainsToPresentCount );
					return;
				}

				PrivateWindowData *pwd = (PrivateWindowData *)Renderer::GetRendererBackendData( *window );
				
				info.RTVs.EmplaceBack( pwd->RTV );
			}
			else if( rt->ColorTargetType() == RenderTarget::TargetType::Texture )
			{
				SOFTBREAK;  //  TODO: unimplemented
				break;
			}
			else
			{
				break;
			}
		}

		if( !info.RTVs.IsEmpty() )
		{
			info.clearColor = camera.ClearColorGet();
			info.colorClearType = camera.ColorClearTypeGet();
			info.depthClearType = camera.DepthClearTypeGet();
			info.depthOverwriteValue = camera.DepthOverwriteValueGet();
			info.viewPort = camera.ViewPortGet();

			_globalMessageQueue.AddDelegate < CALLMETHOD(&RendererDX11Impl::RenderSceneThreaded) >( this, info );
		}
	}

	void RenderSceneThreaded( SceneInfo info )
	{
		if( info.colorClearType == Components::Camera::ColorClearType::SolidColor )
		{
			f128color color = info.clearColor;

			for( auto &rt : info.RTVs )
			{
				_immContext->ClearRenderTargetView( rt.Get(), color.arr );
			}
		}
		else if( info.colorClearType == Components::Camera::ColorClearType::Skybox )
		{
			//  TODO: unimplemented
			SOFTBREAK;
		}

		if( info.depthClearType == Components::Camera::DepthClearType::OverwriteByValue )
		{
			//  TODO: unimplemented
			//SOFTBREAK;
		}
	}

	void BeginFrameThreaded()
	{
	}

	void EndFrameThreaded()
	{
		std::lock_guard < std::mutex > lock( _endFrameMutex );
		_is_frameEnded = true;
		_endFrameCV.notify_all();
	}

	void CleanupBackendDataThreaded( void *resourceAddress )
	{

	}

	static void ThreadFunc( std::atomic_bool *is_countinue, MTMessageQueue *mq )
	{
		while( is_countinue->load() == true )
		{
			mq->ExecWait();
		}
	}
};

RendererDX11 *GameTest::CreateRendererDX11( D3D_FEATURE_LEVEL maxFeatureLevel, ui32 adaptersMask, Renderer::rendererError *error )
{
	Renderer::rendererError rerror;
	RendererDX11Impl *renderer = new RendererDX11Impl( maxFeatureLevel, adaptersMask, &rerror );
	if( rerror != Error::Ok() )
	{
		delete renderer;
		DSA( error, rerror );
		return nullptr;
	}
	return renderer;
}

void GameTest::DestroyRendererDX11( RendererDX11 *renderer )
{
	delete renderer;
}
            
const char *ConvertDX11ErrToString( HRESULT hresult )
{
	switch( hresult )
    {
        case D3D11_ERROR_FILE_NOT_FOUND:
            return "D3D11_ERROR_FILE_NOT_FOUND";
        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
            return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
        case DXGI_ERROR_INVALID_CALL:
            return "DXGI_ERROR_INVALID_CALL";
        case DXGI_ERROR_WAS_STILL_DRAWING:
            return "DXGI_ERROR_WAS_STILL_DRAWING";
        case E_FAIL:
            return "E_FAIL";
        case E_INVALIDARG:
            return "E_INVALIDARG";
        case E_OUTOFMEMORY:
            return "E_OUTOFMEMORY";
        case S_FALSE:
            return "S_FALSE";
		default:
			return "unidentified error";
    }
}