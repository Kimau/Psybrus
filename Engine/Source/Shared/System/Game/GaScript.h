/**************************************************************************
*
* File:		GaScript.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaScript_H__
#define __GaScript_H__

#include "RsCore.h"
#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// GaScriptRef
typedef CsResourceRef< class GaScript > GaScriptRef;

//////////////////////////////////////////////////////////////////////////
// GaScript
class GaScript:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, GaScript );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	int									execute( BcBool Now );
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
private:
	const char*							pScript_;
	
};

#endif
