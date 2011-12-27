/**************************************************************************
*
* File:		CsTypes.h
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSTYPES_H__
#define __CSTYPES_H__

#include "BcTypes.h"
#include "BcDebug.h"
#include "BcName.h"
#include "BcPath.h"

#include "FsFile.h"
#include "BcAtomic.h"
#include "BcEndian.h"
#include "BcHash.h"
#include "BcMutex.h"
#include "BcScopedLock.h"

//////////////////////////////////////////////////////////////////////////
// CsDependancyList
typedef std::list< class CsDependancy > CsDependancyList;
typedef CsDependancyList::iterator CsDependancyListIterator;

//////////////////////////////////////////////////////////////////////////
// CsDependancy
class CsDependancy
{
public:
	CsDependancy( const BcPath& FileName );
	CsDependancy( const BcPath& FileName, const FsStats& Stats );
	CsDependancy( const CsDependancy& Other );
	~CsDependancy();

	/**
	 * Get file name.
	 */
	const BcPath& getFileName() const;

	/**
	 * Get stats.
	 */
	const FsStats& getStats() const;

	/**
	 * Had dependancy changed?
	 */
	BcBool hasChanged();

	/**
	 * Update stats.
	 */
	void updateStats();


private:
	BcPath FileName_;
	FsStats Stats_;
};

#endif
