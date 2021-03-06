/**************************************************************************
*
* File:		BcDebug.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Debug stuff
*		
*
*
* 
**************************************************************************/

#include "Base/BcDebug.h"

#include <windows.h>
#include <string>

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcBacktraceResult BcBacktrace()
{
	BcBacktraceResult Result;

#if PLATFORM_WINDOWS
	void* Addresses[ 1024 ] = { false };
	auto NoofFrames = ::CaptureStackBackTrace( 1, ARRAYSIZE( Addresses ), Addresses, nullptr );
	for( decltype( NoofFrames ) Idx = 0; Idx < NoofFrames; ++Idx )
	{
		BcBacktraceEntry Entry = 
		{
			Addresses[ Idx ],
			std::to_string( (_Longlong)Addresses[ Idx ] ) // TODO: Symbols.
		};

		Result.Backtrace_.push_back( Entry );
	}
#endif
	return Result;
}

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type, BcMessageBoxIcon Icon )
{
#if PLATFORM_WINDOWS
	UINT MBType = MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST;

	switch( Type )
	{
		case bcMBT_OK:
			MBType |= MB_OK;
			break;
		case bcMBT_OKCANCEL:
			MBType |= MB_OKCANCEL;
			break;
		case bcMBT_YESNO:
			MBType |= MB_YESNO;
			break;
		case bcMBT_YESNOCANCEL:
			MBType |= MB_YESNOCANCEL;
			break;
	}

	switch( Icon )
	{
		case bcMBI_WARNING:
			MBType |= MB_ICONWARNING;
			break;
		case bcMBI_ERROR:
			MBType |= MB_ICONERROR;
			break;
		case bcMBI_QUESTION:
			MBType |= MB_ICONQUESTION;
			break;
		default:
			MBType |= MB_ICONWARNING;
			break;
	}

	// Log.
	PSY_LOG( "%s: %s\n", pTitle, pMessage );

	// TODO: HWND!
	int RetVal = ::MessageBoxA( NULL, pMessage, pTitle, MBType );

	switch( RetVal )
	{
		case IDOK:
			return bcMBR_OK;
			break;
		case IDYES:
			return bcMBR_YES;
			break;
		case IDNO:
			return bcMBR_NO;
			break;
		case IDCANCEL:
			return bcMBR_CANCEL;
			break;
		default:
			break;
	};

#endif
	return bcMBR_OK;
}
