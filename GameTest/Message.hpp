#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <tuple>

namespace GameTest
{		
	template < typename T > struct ArgConverter
	{
		typedef T stored;

		static T &FromStored( stored &arg )
		{
			return arg;
		}

		template < typename = std::enable_if < std::is_arithmetic < T >::value == false >::type > static T &ToStored( T &arg )
		{
			return arg;
		}

		template < typename = std::enable_if < std::is_arithmetic < T >::value == true >::type > static T ToStored( T arg )
		{
			return arg;
		}
	};

	/*template < typename T > struct ArgConverter < T & >
	{
		typedef T stored;

		static T &FromStored( stored &arg )
		{
			static_assert( false, "modifiable references are forbidden" );
		}

		static T &ToStored( T &arg )
		{
			static_assert( false, "modifiable references are forbidden" );
		}
	};

	template < typename T > struct ArgConverter < const T & >
	{
		typedef T stored;

		static const T &FromStored( const stored &arg )
		{
			return arg;
		}

		static const T &ToStored( const T &arg )
		{
			return arg;
		}
	};*/

	template < typename T > struct ArgConverter < T * >
	{
		using stored = T *;

		static T *FromStored( stored arg )
		{
			return arg;
		}

		static T *ToStored( T *arg )
		{
			return arg;
		}
	};

	template < typename T > struct ArgConverter < const T * >
	{
		using stored = const T *;

		static const T *FromStored( stored arg )
		{
			return arg;
		}

		static const T *ToStored( const T *arg )
		{
			return arg;
		}
	};

	template < typename T > struct ArgConverter < T [] >
	{
		using stored = T *;

		static T *FromStored( stored *arg )
		{
			return arg;
		}

		static T *ToStored( T *arg )
		{
			return arg;
		}
	};

	template < typename T > struct ArgConverter < const T [] >
	{
		using stored = const T *;

		static const T *FromStored( stored arg )
		{
			return arg;
		}

		static const T *ToStored( const T *arg )
		{
			return arg;
		}
	};

	template < typename T, size_t size > struct ArgConverter < T [ size ] >
	{
		using stored = T *;

		static T *FromStored( stored *arg )
		{
			return arg;
		}

		static T *ToStored( T *arg )
		{
			return arg;
		}
	};

	template < typename T, size_t size > struct ArgConverter < const T [ size ] >
	{
		using stored = const T *;

		static const T *FromStored( stored arg )
		{
			return arg;
		}

		static const T *ToStored( const T *arg )
		{
			return arg;
		}
	};

	template < typename T > struct ArgConverter < T && >
	{
		using stored = T;

		static T &&FromStored( stored &arg )
		{
			return std::move( arg );
		}

		static T &ToStored( T &&arg )
		{
			return std::move( arg );
		}
	};

	template < int... > struct seq
	{};
 
	template < int N, int... S > struct gens : gens < N - 1, N - 1, S... >
	{};
 
	template < int... S > struct gens < 0, S... >
	{
		typedef seq < S... > type;
	};

	template < typename T > struct GetCallablePointer;

	template < typename T > struct GetCallablePointer < T * >
	{
		static T *Get( T *caller )
		{
			return caller;
		}
	};

	template < typename T > struct GetCallablePointer < const T * >
	{
		static const T *Get( T *caller )
		{
			return caller;
		}
	};

	template < typename T > struct GetCallablePointer < T * const >
	{
		static T *Get( T *caller )
		{
			return caller;
		}
	};

	template < typename T > struct GetCallablePointer < const T * const >
	{
		static const T *Get( T *caller )
		{
			return caller;
		}
	};

	template < typename T > struct GetCallablePointer < T & >
	{
		static T *Get( T &caller )
		{
			return &caller;
		}
	};

	template < typename T > struct GetCallablePointer < const T & >
	{
		static const T *Get( const T &caller )
		{
			return &caller;
		}
	};
 
	class MessageBase
	{
	public:
		void (*ProcessAndDestroy)( MessageBase * ) = 0;
	};
 
	template < typename Caller, typename MethodType, MethodType methodToCall, typename... MessageArgs > struct MessageDelegate : public MessageBase
	{
		Caller caller;
		std::tuple < typename ArgConverter < MessageArgs >::stored... > args;
 
		template < int... S > static void callFunc( MessageBase *object, seq < S... > )
		{
			MessageDelegate *helper = (MessageDelegate *)object;
			(GetCallablePointer < Caller >::Get( helper->caller )->*methodToCall)(ArgConverter < MessageArgs >::FromStored( std::get < S >( helper->args ) )...);
			helper->~MessageDelegate();
		}
 
		static void CallFunc( MessageBase *object )
		{
			callFunc( object, typename gens < sizeof...(MessageArgs) >::type() );
		}
 
		template < typename TCaller, typename... TMessageArgs > MessageDelegate( TCaller &&caller, TMessageArgs &&... args ) : 
			caller( std::forward < TCaller >( caller ) ), 
			args( ArgConverter < TMessageArgs >::ToStored( std::forward < TMessageArgs >( args ) )... )
		{
			this->ProcessAndDestroy = &CallFunc;
		}
	};
 
	template < typename FuncType, FuncType *funcToCall, typename... MessageArgs > struct MessageFuncInline : public MessageBase
	{
		std::tuple < typename ArgConverter < MessageArgs >::stored... > args;
 
		template < int... S > static void callFunc( MessageBase *object, seq < S... > )
		{
			MessageFuncInline *helper = (MessageFuncInline *)object;
			funcToCall(ArgConverter < MessageArgs >::FromStored( std::get < S >( helper->args ) )...);
			helper->~MessageFuncInline();
		}
 
		static void CallFunc( MessageBase *object )
		{
			callFunc( object, typename gens < sizeof...(MessageArgs) >::type() );
		}
 
		template < typename... TMessageArgs > MessageFuncInline( TMessageArgs &&... args ) : 
			args( ArgConverter < TMessageArgs >::ToStored( std::forward < TMessageArgs >( args ) )... )
		{
			this->ProcessAndDestroy = &CallFunc;
		}
	};
 
	template < typename FuncType, typename... MessageArgs > struct MessageFuncPointer : public MessageBase
	{
		FuncType *func;
		std::tuple < typename ArgConverter < MessageArgs >::stored... > args;
 
		template < int... S > static void callFunc( MessageBase *object, seq < S... > )
		{
			MessageFuncPointer *helper = (MessageFuncPointer *)object;
			helper->func(ArgConverter < MessageArgs >::FromStored( std::get < S >( helper->args ) )...);
			helper->~MessageFuncPointer();
		}
 
		static void CallFunc( MessageBase *object )
		{
			callFunc( object, typename gens < sizeof...(MessageArgs) >::type() );
		}
 
		template < typename... TMessageArgs > MessageFuncPointer( FuncType func, TMessageArgs &&... args ) : 
			func( func ),
			args( ArgConverter < TMessageArgs >::ToStored( std::forward < TMessageArgs >( args ) )... )
		{
			this->ProcessAndDestroy = &CallFunc;
		}
	};

	#define CALLMETHOD( method ) decltype(method), method
}

#endif