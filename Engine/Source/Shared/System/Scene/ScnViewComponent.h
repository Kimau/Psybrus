/**************************************************************************
*
* File:		ScnViewComponent.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnViewComponent_H__
#define __ScnViewComponent_H__

#include "RsCore.h"
#include "ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewComponentRef
typedef CsResourceRef< class ScnViewComponent > ScnViewComponentRef;
typedef std::list< ScnViewComponentRef > ScnViewComponentList;
typedef ScnViewComponentList::iterator ScnViewComponentListIterator;
typedef ScnViewComponentList::const_iterator ScnViewComponentListConstIterator;
typedef std::map< std::string, ScnViewComponentRef > ScnViewComponentMap;
typedef ScnViewComponentMap::iterator ScnViewComponentMapIterator;
typedef ScnViewComponentMap::const_iterator ScnViewComponentMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnViewComponent
class ScnViewComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnViewComponent );
	DECLARE_VISITABLE( ScnViewComponent );

#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:
	RsViewport							Viewport_;
	
	struct THeader
	{
		// Viewport. Values relative to the size of the client being rendered into.
		BcReal							X_;
		BcReal							Y_;
		BcReal							Width_;
		BcReal							Height_;

		// Perspective projection.
		BcReal							HorizontalFOV_;		// Used by default.
		BcReal							VerticalFOV_;		// Used if HorizontalFOV_ is 0.0.
		BcReal							Near_;
		BcReal							Far_;
	};
	
	THeader*							pHeader_;
};

#endif