#ifndef __MT_MESSAGE_QUEUE_HPP__
#define __MT_MESSAGE_QUEUE_HPP__

#include "Message.hpp"
#include <deque>
#include <mutex>
#include <condition_variable>

namespace GameTest
{
	//  it's a simple proto
	class MTMessageQueue
	{
		std::deque < MessageBase * > _messages;
		std::mutex _mutex;
		std::condition_variable _newWorkNotifier;
		std::condition_variable _becameEmptyNotifier;
		bool _is_stopWorkWaiters = false;

	public:

		template < typename MethodType, MethodType method, typename Caller, typename... VArgs > void AddDelegate( Caller &&caller, VArgs &&... args )
		{
			typedef MessageDelegate < Caller, MethodType, method, typename std::remove_reference < VArgs >::type... > messageType;
			messageType *newMessage = new messageType( std::forward < Caller >( caller ), std::forward < typename std::remove_reference < VArgs >::type >( args )... );
			_mutex.lock();
			_messages.push_back( newMessage );
			_newWorkNotifier.notify_all();
			_mutex.unlock();
		}

		template < typename FuncType, FuncType *func, typename... VArgs > void AddFunctionInline( VArgs &&... args )
		{
			typedef MessageFuncInline < FuncType, func, typename std::remove_reference < VArgs >::type... > messageType;
			messageType *newMessage = new messageType( std::forward < typename std::remove_reference < VArgs >::type >( args )... );
			_mutex.lock();
			_messages.push_back( newMessage );
			_newWorkNotifier.notify_all();
			_mutex.unlock();
		}

		template < typename FuncType, typename... VArgs > void AddFunctionPointer( FuncType *func, VArgs &&... args )
		{
			typedef MessageFuncPointer < FuncType, typename std::remove_reference < VArgs >::type... > messageType;
			messageType *newMessage = new messageType( func, std::forward < typename std::remove_reference < VArgs >::type >( args )... );
			_mutex.lock();
			_messages.push_back( newMessage );
			_newWorkNotifier.notify_all();
			_mutex.unlock();
		}

		void ExecWait()
		{
			std::unique_lock < std::mutex > cvLock( _mutex );
			_newWorkNotifier.wait( cvLock, [this]{ return !this->_messages.empty() || this->_is_stopWorkWaiters; } );

			if( _messages.size() )
			{
				MessageBase *message = _messages.front();
				_messages.pop_front();

				if( _messages.empty() )
				{
					_becameEmptyNotifier.notify_all();
				}

				cvLock.unlock();

				void (*pointer)( MessageBase *message ) = message->ProcessAndDestroy;
				pointer( message );
				delete message;
			}
		}

		bool ExecNoWait()  //  returns true if something had been exectuted
		{
			_mutex.lock();
			if( _messages.size() )
			{
				MessageBase *message = _messages.front();
				_messages.pop_front();

				if( _messages.empty() )
				{
					_becameEmptyNotifier.notify_all();
				}

				_mutex.unlock();

				void (*pointer)( MessageBase *message ) = message->ProcessAndDestroy;
				pointer( message );
				delete message;

				return true;
			}
			else
			{
				_mutex.unlock();
				return false;
			}
		}

		void WaitForEmpty()
		{
			std::unique_lock < std::mutex > lock( _mutex );
			_becameEmptyNotifier.wait( lock, [this]{ return _messages.empty(); } );
		}

		void StopWorkWaiters()
		{
			std::lock_guard < std::mutex > lock( _mutex );
			this->_is_stopWorkWaiters = true;
			_newWorkNotifier.notify_all();
		}

		void AllowWorkWaiters()
		{
			std::lock_guard < std::mutex > lock( _mutex );
			this->_is_stopWorkWaiters = false;
			_newWorkNotifier.notify_all();
		}
	};
}

#endif