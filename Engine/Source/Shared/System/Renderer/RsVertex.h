/**************************************************************************
*
* File:		RsVertex.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Vertex stuff
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXDATA_H__
#define __RSVERTEXDATA_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsVertexElement
struct RsVertexElement
{
	RsVertexElement()
	{
	}

	RsVertexElement( BcU32 StreamIdx,
	                 BcU32 Offset,
	                 BcU32 Components,
	                 eRsVertexDataType DataType,
	                 eRsVertexUsage Usage,
	                 BcU32 UsageIdx ):
		StreamIdx_( StreamIdx ),
		Offset_( Offset ),
		Components_( Components ),
		DataType_( DataType ),
		Usage_( Usage ),
		UsageIdx_( UsageIdx )
	{
	}


	BcU32				StreamIdx_;
	BcU32				Offset_;
	BcU32				Components_;
	eRsVertexDataType	DataType_;
	eRsVertexUsage		Usage_;
	BcU32				UsageIdx_;
};

#endif
