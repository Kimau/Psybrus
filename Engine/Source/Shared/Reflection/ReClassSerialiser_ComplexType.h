#ifndef __REFLECTION_CLASSSERIALISER_COMPLEXTYPE_H__
#define __REFLECTION_CLASSSERIALISER_COMPLEXTYPE_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_ComplexType.
template < typename _Ty >
class ReClassSerialiser_ComplexType:
		public ReClassSerialiser_AbstractComplexType< _Ty >
{
public:
    ReClassSerialiser_ComplexType( BcName Name ): ReClassSerialiser_AbstractComplexType< _Ty >( Name ) {}
    virtual ~ReClassSerialiser_ComplexType() {}

	void construct( void* pMemory ) const
	{
		new ( pMemory ) _Ty();
	}

	void constructNoInit( void* pMemory ) const
	{
		new ( pMemory ) _Ty( NOINIT );
	}

	void destruct( void* pMemory ) const
	{
		reinterpret_cast< _Ty* >( pMemory )->~_Ty();
	}

	void* create() const
	{
		return new _Ty();
	}

	void* createNoInit() const
	{
		return new _Ty( NOINIT );
	}

	void destroy( void* Object ) const
	{
		delete reinterpret_cast< _Ty* >( Object );
	}

	virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
	{
		return false;
	}

	virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
	{
		return false;
	}

	virtual BcBool copy( void* pDst, void* pSrc ) const
	{
		return false;
	}
};

#endif
