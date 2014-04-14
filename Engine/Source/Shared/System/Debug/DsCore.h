/**************************************************************************
*
* File:		DsCore.h
* Author:	Neil Richardson 
* Ver/Date:	11/12/12	
* Description:
*		Debug system core.
*		
*
*
* 
**************************************************************************/

#ifndef __DSCORE_H__
#define __DSCORE_H__

#include "Base/BcGlobal.h"
#include "Base/BcMutex.h"
#include "Base/BcScopedLock.h"
#include "Base/BcHtml.h"
#include "System/SysSystem.h"

#include "System/Content/CsCore.h"
#include "System/Scene/ScnCore.h"
#include "../../Psybrus.h"
#include <mongoose.h>
#include <functional>
#include <map>
//////////////////////////////////////////////////////////////////////////
/**	\struct DsCoreMessage
*	\brief Storage struct for functions
*
*	
*/

// typedef const std::map < std::string, std::string>& DsParameters;
typedef const std::vector< std::string>& DsParameters;

typedef struct DsCoreMessage
{
	DsCoreMessage(std::string r, std::string display)
	: Regex_(r.c_str()),
		Text_(r),
		Display_(display),
		Visible_(true) {}

	DsCoreMessage(std::string r)
		: Regex_(r.c_str()),
		Text_(r),
		Visible_(false) {}

	BcRegex Regex_;
	std::string Text_;
	std::string Display_;
	bool Visible_;
	std::function <void(DsParameters, BcHtmlNode&)> Function_;
} DsCoreMessage;

//////////////////////////////////////////////////////////////////////////
/**	\class DsCore
*	\brief Debug System Core
*
*	Debugging system core.
*/
class DsCore :
	public BcGlobal< DsCore >,
	public SysSystem
{
public:
	static BcU32 WORKER_MASK;

public:
	DsCore();
	virtual ~DsCore();

	virtual void				open();
	virtual void				update();
	virtual void				close();

	static void					cmdMenu(DsParameters params, BcHtmlNode& Output);
	static void					cmdContent(DsParameters params, BcHtmlNode& Output);

	void						cmdContent_Resource(BcHtmlNode& Output);

	static void					cmdScene(DsParameters params, BcHtmlNode& Output);
	static void					cmdScene_Entity(ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth);
	static void					cmdScene_Component(ScnComponentRef Entity, BcHtmlNode& Output, BcU32 Depth);

	static void					cmdResource(DsParameters params, BcHtmlNode& Output);

	void						gameThreadMongooseCallback( enum mg_event Event, struct mg_connection* pConn );
	void*						mongooseCallback( enum mg_event Event, struct mg_connection* pConn );
	static void*				MongooseCallback( enum mg_event Event, struct mg_connection* pConn );

	void						writeHeader(BcHtmlNode& Output);
	void						writeFooter(BcHtmlNode& Output);
	BcU8*						writeFile(std::string filename, int& OutLength, std::string& type);
	void						registerFunction(std::string regex, std::function < void(DsParameters, BcHtmlNode&)> fn, std::string display);
	void						registerFunction(std::string regex, std::function < void(DsParameters, BcHtmlNode&)> fn);
	void						deregisterFunction(std::string regex);


private:
	mg_context*					pContext_;
	BcMutex						Lock_;
	SysFence					GameThreadWaitFence_;
	std::vector<DsCoreMessage>	MessageFunctions_;
};


#endif
