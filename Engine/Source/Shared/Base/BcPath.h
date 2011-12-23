/**************************************************************************
*
* File:		BcPath.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		File system path
*		
*
*
* 
**************************************************************************/

#ifndef __BCPATH_H__
#define __BCPATH_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcPath
class BcPath
{
public:
	static const BcChar BcPath::Seperator;

public:
	BcPath();
	BcPath( const std::string& Value );
	BcPath( const BcChar* pValue );
	virtual ~BcPath();

	const BcChar* operator * () const;
	void join( const BcPath& PathA );
	void join( const BcPath& PathA, const BcPath& PathB );
	void join( const BcPath& PathA, const BcPath& PathB, const BcPath& PathC );

protected:
	void fixSeperators();
	static std::string stripLeadingSeperator( const std::string& Path );
	static std::string appendTrailingSeperator( const std::string& Path );

private:
	std::string InternalValue_;

};


#endif