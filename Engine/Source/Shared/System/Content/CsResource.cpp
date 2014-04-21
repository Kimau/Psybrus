/**************************************************************************
*
* File:		CsResource.cpp
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Content/CsResource.h"
 
#include "System/Content/CsCore.h"
#include "System/Content/CsPackage.h"

std::atomic< BcU32 > CsResource::UniqueIdCounter_ = 0;

//////////////////////////////////////////////////////////////////////////
// Define CsResource
REFLECTION_DEFINE_DERIVED( CsResource );

//////////////////////////////////////////////////////////////////////////
// Reflection
void CsResource::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Index_",				&CsResource::Index_ ),
		ReField( "InitStage_",			&CsResource::InitStage_ ),
		ReField( "UniqueId_",			&CsResource::UniqueId_ ),
	};
		
	ReRegisterClass< CsResource, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResource::CsResource():
	Index_( BcErrorCode ),
	InitStage_( INIT_STAGE_INITIAL ),
	UniqueId_( UniqueIdCounter_++ )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsResource::~CsResource()
{
	
}

//////////////////////////////////////////////////////////////////////////
// preInitialise
void CsResource::preInitialise( const BcName& Name, BcU32 Index, CsPackage* pPackage )
{
	BcAssertMsg( Name != BcName::INVALID, "Resource can not have an invalid name." );
	BcAssertMsg( Name != BcName::NONE, "Resource can not have a none name." );

	setName( Name );
	setOwner( pPackage );
	Index_ = Index;
}

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool CsResource::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	BcUnusedVar( Importer );
	BcUnusedVar( Object );

	// TODO: Generic property save out?

	return BcTrue;
}
#endif

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void CsResource::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void CsResource::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void CsResource::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void CsResource::fileReady()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool CsResource::isReady() const
{
	BcAssertMsg( InitStage_ >= INIT_STAGE_INITIAL && InitStage_ <= INIT_STAGE_READY, "CsResource: Invalid ready state." );
	return InitStage_ == INIT_STAGE_READY ? BcTrue : BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getInitStage
BcU32 CsResource::getInitStage() const
{
	return InitStage_;
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void CsResource::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// getName
CsPackage* CsResource::getPackage() const
{
	CsPackage* RetVal = nullptr;
	if( getOwner() != nullptr )
	{
		BcAssert( getOwner()->isTypeOf< CsPackage >() );
		RetVal = static_cast< CsPackage* >( getOwner() );
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& CsResource::getPackageName() const
{
	return getPackage() != nullptr ? getPackage()->getName() : BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getIndex
BcU32 CsResource::getIndex() const
{
	return Index_;
}

//////////////////////////////////////////////////////////////////////////
// serialiseProperties
//virtual
void CsResource::serialiseProperties()
{
	BcPrintf("=============================================\n");

	// Iterate over all properties and do stuff.
	const ReClass* pClass = getClass();

	// NOTE: Do not want to hit this. Ever.
	if( pClass == NULL )
	{
		int a = 0 ; ++a;
	}

	BcU8* pClassData = reinterpret_cast< BcU8* >( this );

#if 0
	// Iterate over to grab offsets for classes.
	while( pClass != NULL )
	{
		BcPrintf("Class: %s (Size: 0x%x)\n", (*pClass->getName()).c_str(), pClass->getSize());
		for( BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx )
		{
			const BcReflectionField* pField = pClass->getField( Idx );
			const BcReflectionType* pType = pField->getType();
			if( pType != NULL )
			{
				BcPrintf(" - %s %s; // Offset 0x%x, Size 0x%x, Flags: 0x%x\n", (*pType->getName()).c_str(), (*pField->getName()).c_str(), pField->getOffset(), pType->getSize(), pField->getFlags() );
				if( pType->getName() == "BcU8" )
				{
					const BcU8* pData = reinterpret_cast< const BcU8* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %u\n", *pData );
				}
				else if( pType->getName() == "BcU16" )
				{
					const BcU16* pData = reinterpret_cast< const BcU16* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %u\n", *pData );
				}
				else if( pType->getName() == "BcU32" )
				{
					const BcU32* pData = reinterpret_cast< const BcU32* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %u\n", *pData );
				}
				else if( pType->getName() == "BcF32" )
				{
					const BcF32* pData = reinterpret_cast< const BcF32* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %f\n", *pData );
				}
				else if( pType->getName() == "BcName" )
				{
					const BcName* pData = reinterpret_cast< const BcName* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %s\n", (**pData).c_str() );
				}
				else if( pType->getName() == "BcBool" )
				{
					const BcBool* pData = reinterpret_cast< const BcBool* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %u\n", *pData );
				}
				else if( pType->getName() == "MaVec2d" )
				{
					const MaVec2d* pData = reinterpret_cast< const MaVec2d* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %f, %f\n", pData->x(), pData->y() );
				}
				else if( pType->getName() == "MaVec3d" )
				{
					const MaVec3d* pData = reinterpret_cast< const MaVec3d* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %f, %f, %f\n", pData->x(), pData->y(), pData->z() );
				}
				else if( pType->getName() == "MaVec4d" )
				{
					const MaVec4d* pData = reinterpret_cast< const MaVec4d* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %f, %f, %f, %f\n", pData->x(), pData->y(), pData->z(), pData->w() );
				}
				else if( pType->getName() == "MaMat4d" )
				{
					const MaMat4d* pData = reinterpret_cast< const MaMat4d* >( &pClassData[ pField->getOffset() ] );
					BcPrintf( " - - %f, %f, %f, %f\n", pData->row0().x(), pData->row0().y(), pData->row0().z(), pData->row0().w() );
					BcPrintf( " - - %f, %f, %f, %f\n", pData->row1().x(), pData->row1().y(), pData->row1().z(), pData->row1().w() );
					BcPrintf( " - - %f, %f, %f, %f\n", pData->row2().x(), pData->row2().y(), pData->row2().z(), pData->row2().w() );
					BcPrintf( " - - %f, %f, %f, %f\n", pData->row3().x(), pData->row3().y(), pData->row3().z(), pData->row3().w() );
				}
			}
		}

		pClass = pClass->getSuper();
	}
#else
	BcBreakpoint;
#endif 
	BcPrintf("=============================================\n");
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsResource::getString( BcU32 Offset ) const
{
	return getPackage()->getString( Offset );
}

//////////////////////////////////////////////////////////////////////////
// markupName
void CsResource::markupName( BcName& Name ) const
{
	getPackage()->markupName( Name );
}

//////////////////////////////////////////////////////////////////////////
// getChunk
void CsResource::requestChunk( BcU32 Chunk, void* pDataLocation )
{
	if( !getPackage()->requestChunk( Index_, Chunk, pDataLocation ) )
	{

	}
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsResource::getChunkSize( BcU32 Chunk )
{
	return getPackage()->getChunkSize( Index_, Chunk );
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsResource::getNoofChunks() const
{
	return getPackage()->getNoofChunks( Index_ );
}

//////////////////////////////////////////////////////////////////////////
// markCreate
void CsResource::markCreate()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_CREATE );
	BcAssertMsg( OldStage == INIT_STAGE_INITIAL, "CsResource: Trying to mark \"%s\" for creation when it's not in the initial state.", (*getName()).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// markReady
void CsResource::markReady()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_READY );
	BcAssertMsg( OldStage == INIT_STAGE_CREATE, "CsResource: Trying to mark \"%s\" as ready when it's not in creation.", (*getName()).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// markReady
void CsResource::markDestroy()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_DESTROY );
	BcAssertMsg( OldStage == INIT_STAGE_READY, "CsResource: Trying to mark \"%s\" for destruction when it's not ready.", (*getName()).c_str() );

	CsCore::pImpl()->destroyResource( this );
}

//////////////////////////////////////////////////////////////////////////
// onFileReady
void CsResource::onFileReady()
{
	fileReady();
}

//////////////////////////////////////////////////////////////////////////
// onFileChunkReady
void CsResource::onFileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	fileChunkReady( ChunkIdx, ChunkID, pData );
}
