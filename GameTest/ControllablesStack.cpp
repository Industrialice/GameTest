#include "PreHeader.hpp"
#include "ControllablesStack.hpp"

using namespace GameTest;

template < typename func, typename container, typename... vargs > bln TProc( func f, container &&ls, vargs &&... args )
{
	for( uiw index = ls.size() - 1; index != uiw_max; --index )
	{
		if( (ls[ index ]->*f)( args... ) )
		{
			return true;
		}
	}
	return false;
}

bln ControllablesStack::KeyDown( VKeys::vkey_t key, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::KeyDown, _listenersStack, key, info );
}

bln ControllablesStack::KeyUp( VKeys::vkey_t key, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::KeyUp, _listenersStack, key, info );
}

bln ControllablesStack::MouseMove( f32 x, f32 y, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::MouseMove, _listenersStack, x, y, info );
}

bln ControllablesStack::MouseScroll( f32 delta, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::MouseScroll, _listenersStack, delta, info );
}

bln ControllablesStack::TouchpadDown( ui32 &index, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::TouchpadDown, _listenersStack, index, info );  //  TODO: index handling
}

bln ControllablesStack::TouchpadMove( ui32 index, f32 x, f32 y, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::TouchpadMove, _listenersStack, index, x, y, info );  //  TODO: index handling
}

bln ControllablesStack::TouchpadUp( ui32 index, const AdditionalInfo &info )
{
	return TProc( &ControllableInterface::TouchpadUp, _listenersStack, index, info );  //  TODO: index handling
}

void ControllablesStack::AddListener( ControllableInterface *iface )
{
	auto it = std::find( _listenersStack.begin(), _listenersStack.end(), iface );
	if( it != _listenersStack.end() )
	{
		_listenersStack.erase( it );
	}
	_listenersStack.push_back( iface );
}

void ControllablesStack::RemoveListener( ControllableInterface *iface )
{
	auto it = std::find( _listenersStack.begin(), _listenersStack.end(), iface );
	if( it == _listenersStack.end() )
	{
		SOFTBREAK;
		return;
	}
	_listenersStack.erase( it );
}