#ifndef __COM_SHARED_PTR_HPP__
#define __COM_SHARED_PTR_HPP__

namespace GameTest
{
	template < typename X > class RemoveAddRefRelease : public X
	{
		/*ULONG WINAPI AddRef() = delete;
		ULONG WINAPI Release() = delete;*/
	};

	template < typename X > class COMSharedPtr
	{
		X *_ptr = nullptr;

	public:
		~COMSharedPtr()
		{
			if( this->_ptr )
			{
				this->_ptr->Release();
			}
		}

		COMSharedPtr()
		{}

		COMSharedPtr( X *ptr ) : _ptr( ptr )
		{}

		COMSharedPtr( const COMSharedPtr &source ) : _ptr( source._ptr )
		{
			if( _ptr )
			{
				_ptr->AddRef();
			}
		}

		COMSharedPtr &operator = ( const COMSharedPtr &source )
		{
			if( this != source )
			{
				if( this->_ptr )
				{
					this->_ptr->Release();
				}

				this->_ptr = source._ptr;

				if( this->_ptr )
				{
					this->_ptr->AddRef();
				}
			}
			return *this;
		}

		COMSharedPtr &operator = ( COMSharedPtr &&source )
		{
			this->_ptr = source._ptr;
			source._ptr = nullptr;
		}

		COMSharedPtr( COMSharedPtr &&source )
		{
			this->_ptr = source._ptr;
			source._ptr = nullptr;
		}

		RemoveAddRefRelease < X > *operator->()
		{
			ASSUME( _ptr );
			return (RemoveAddRefRelease < X > *)_ptr;
		}

		const RemoveAddRefRelease < X > *operator->() const
		{
			ASSUME( _ptr );
			return (RemoveAddRefRelease < X > *)_ptr;
		}

		X **AddressOf()
		{
			ASSUME( _ptr == nullptr );
			return &_ptr;
		}

		X *Get()
		{
			return _ptr;
		}

		template < typename T > COMSharedPtr < T > As()
		{
			COMSharedPtr < T > result;
			if( _ptr != nullptr )
			{
				_ptr->QueryInterface( &result._ptr );
			}
			return result;
		}

		void Release()
		{
			if( this->_ptr )
			{
				this->_ptr->Release();
				this->_ptr = nullptr;
			}
		}

		bool operator == ( std::nullptr_t ) const
		{
			return this->_ptr == nullptr;
		}

		bool operator != ( std::nullptr_t ) const
		{
			return this->_ptr != nullptr;
		}

		bool operator ! () const
		{
			return !this->_ptr;
		}
	};
}

#endif