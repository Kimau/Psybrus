#include "MainShared.h"
#include "SysKernel.h"

#include "OsWindowWindows.h"

BcHandle GWindowDC_ = NULL;
BcHandle GWindowRC_ = NULL;

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	( void )hPrevInstance;
	( void )lpCmdLine;
	( void )nCmdShow;

	// Perform unit tests.
	MainUnitTests();
	
	// Set command line params.
	SysArgs_ = lpCmdLine;
	
	// Create kernel.
	new SysKernel();
	
	// Register systems for creation.
	SYS_REGISTER( "GaCore", GaCore );
	//SYS_REGISTER( "RmCore", RmCore );
	SYS_REGISTER( "OsCore", OsCoreImplSDL );
	SYS_REGISTER( "FsCore", FsCoreImplWindows );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImplGL );
	SYS_REGISTER( "SsCore", SsCoreImplAL );
	
	// Create window.
	OsWindowWindows MainWindow;
	if( MainWindow.create( "Psybrus Testbed", (BcHandle)hInstance, 1280, 720, BcFalse ) == BcFalse )
	{
		BcPrintf( "Failed to create window!\n" );
		return 1;
	}

	// Cache handle globally.
	GWindowDC_ = MainWindow.getDC();
	
	BcMessageBox( "Something has happened.", "These are the details about whatever has happened.", bcMBT_OK );
	BcMessageBox( "Something has happened.", "These are the details about whatever has happened.", bcMBT_OKCANCEL );
	BcMessageBox( "Something has happened.", "These are the details about whatever has happened.", bcMBT_YESNO );
	BcMessageBox( "Something has happened.", "These are the details about whatever has happened.", bcMBT_YESNOCANCEL );

	// Main shared.
	MainShared();

	// Game init.
	extern void PsyGameInit();
	PsyGameInit();

	// Run kernel threaded.
	SysKernel::pImpl()->run( BcTrue );

	// Do event pump.
	for(;;)
	{
		MSG Msg;
		
		while( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) == TRUE )
		{
			::TranslateMessage( &Msg );
			::DispatchMessage( &Msg );

			// Check for quit.
			if( Msg.message == WM_QUIT )
			{
				// Stop, join, and free kernel.
				SysKernel::pImpl()->stop();
				SysKernel::pImpl()->join();
				delete SysKernel::pImpl();

				// Destroy main window.
				MainWindow.destroy();

				// Done!
				return 0;
			}
		}

		BcYield();
	}

	// Done.
	return 0;
}

void PsyGameInit()
{
	// Do game init here.
}
