/**************************************************************************
*
* File:		CsRedirector.h
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*			Resource used to redirect to another. Will all be transparent 
*			to the user of the content system at the CsCore & CsPackage 
*			level.
*
*
**************************************************************************/

#ifndef __CSREDIRECTOR_H__
#define __CSREDIRECTOR_H__

#include "System/Content/CsResource.h"
#include "System/Content/CsResourceImporter.h"

//////////////////////////////////////////////////////////////////////////
// CsRedirectorImport
class CsRedirectorImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( CsRedirectorImport, CsResourceImporter );

	CsRedirectorImport();
	CsRedirectorImport( ReNoInit );
	virtual ~CsRedirectorImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& );

private:
	CsCrossRefId Target_;
};

//////////////////////////////////////////////////////////////////////////
// CsRedirector
class CsRedirector:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsRedirector, CsResource );

	virtual void initialise();

	/**
	 * Get resource we redirect to.
	 */
	CsResource* getResource();

private:
	virtual void fileReady();
	virtual void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );


private:
	CsCrossRefId CrossRefId_;
	CsResource* Target_;
};


#endif // __CSREDIRECTOR_H__