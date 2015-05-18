#pragma once

#include "System/Scene/ScnTypes.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponentProcessFunc
using ScnComponentProcessFunc = std::function< void( const ScnComponentList& ) >;

struct ScnComponentProcessFuncEntry
{
	BsS32 Priority_;
	ScnComponentProcessFunc Func_;
};
using ScnComponentProcessFuncEntryList = std::vector< ScnComponentProcessFuncEntry >;

//////////////////////////////////////////////////////////////////////////
// ScnComponentProcessor
class ScnComponentProcessor:
	public ReAttribute
{
public:
	REFLECTION_DECLARE_DERIVED( ScnComponentProcessor, ReAttribute );

	ScnComponentProcessor( BcS32 Priority = 0 );

	/**
	 * Called to get the process funcs.
	 * By default it will register preUpdate, update, and postUpdate
	 * calls to emulate the original. Override to implement your own.
	 */
	virtual ScnComponentProcessFuncEntryList getProcessFuncs();

private:
	BcS32 Priority_;

};
