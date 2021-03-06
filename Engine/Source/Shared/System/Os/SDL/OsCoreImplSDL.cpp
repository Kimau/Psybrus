/**************************************************************************
*
* File:		OsCoreImplSDL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Os/SDL/OsCoreImplSDL.h"
#include "System/Os/SDL/OsClientSDL.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplSDL );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplSDL::OsCoreImplSDL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplSDL::~OsCoreImplSDL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplSDL::open()
{
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
	{
		PSY_LOG( "SDL_Init Error: %u\n", SDL_GetError() );
		BcBreakpoint;
	}


}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplSDL::update()
{
	OsCore::update();

	SDL_Event Event;
	{
		while( SDL_PollEvent( &Event ) )
		{
			switch( Event.type )
			{
			case SDL_QUIT:
				pKernel()->stop();
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
			case SDL_WINDOWEVENT:
				for( auto Client : ClientList_ )
				{
					OsClientSDL* SDLClient = dynamic_cast< OsClientSDL* >( Client );
					if( SDLClient->getWindowId() == Event.window.windowID )
					{
						SDLClient->handleEvent( Event );
					}
				}
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplSDL::close()
{
	SDL_Quit();
}
