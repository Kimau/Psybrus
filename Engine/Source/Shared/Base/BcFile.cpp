/**************************************************************************
*
* File:		BcFile.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "BcFile.h"
#include "BcDebug.h"
#include "BcString.h"
#include "BcMemory.h"

#include <fcntl.h>

#if PLATFORM_WINDOWS

//////////////////////////////////////////////////////////////////////////
// Windows Includes
#include <io.h>

//////////////////////////////////////////////////////////////////////////
// Windows Defines
#define lowLevelOpen		::open
#define lowLevelClose		::close
#define lowLevelReadFlags	(_O_RDONLY | _O_BINARY)

#define streamOpen			fopen
#define streamClose			fclose


#elif defined( PLATFORM_LINUX ) || defined( PLATFORM_OSX )
//////////////////////////////////////////////////////////////////////////
// *nix Includes
#include <sys/stat.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////
// *nix Defines
#define lowLevelOpen		::open
#define lowLevelClose		::close
#define lowLevelReadFlags	(O_RDONLY)	//Binary flag does not exist on posix

#define streamOpen			fdopen
#define streamClose			fclose

#endif

//////////////////////////////////////////////////////////////////////////
// Ctor & Dtor
BcFile::BcFile()
{
	FileDescriptor_ = -1;
	FileHandle_ = NULL;
}

BcFile::~BcFile()
{
	if ( FileHandle_ != NULL )
	{
		close();
	}
}

//////////////////////////////////////////////////////////////////////////
// calcFileSize
void BcFile::calcFileSize( void )
{
#if PLATFORM_WINDOWS

	if( FileHandle_ != NULL )
	{
		FileSize_ = _filelength( _fileno( FileHandle_ ) );
	}

#elif PLATFORM_LINUX || PLATFORM_OSX
	//Early out if no file open
	if(FileDescriptor_ == -1)
	{
		return;
	}

	struct stat _stat;
	if ( 0 == fstat(FileDescriptor_, &_stat) )
	{
		FileSize_ = _stat.st_size;
	}

#endif
}

//////////////////////////////////////////////////////////////////////////
// open
BcBool BcFile::open( const BcChar* FileName, eBcFileMode AccessMode )
{
	switch ( AccessMode )
	{
	case bcFM_READ:
		{
#if PLATFORM_WINDOWS
			// Open the stream
			FileHandle_ = streamOpen(FileName, "rb");

			// Grab the size of the file
			calcFileSize();
#elif PLATFORM_LINUX || PLATFORM_OSX
			// Open the file
			FileDescriptor_ = lowLevelOpen(FileName, lowLevelReadFlags);

			//Did the file open successfully?
			if(FileDescriptor_ != -1)
			{
				//Yep - grab the size of the file
				calcFileSize();

				// Open the stream
				FileHandle_ = streamOpen(FileDescriptor_, "rb");
			}
#endif
			AccessMode_ = AccessMode;
		}
		break;

	case bcFM_WRITE:
		{
			FileHandle_ = fopen( FileName, "wb+" );
		}
		break;

	default:
		return BcFalse;
		break;
	};

	return ( FileHandle_ != 0 );
}

//////////////////////////////////////////////////////////////////////////
// close
void BcFile::close()
{
	streamClose( FileHandle_ );
	FileHandle_ = 0;

#if PLATFORM_LINUX || PLATFORM_OSX
	if( FileDescriptor_ != -1)
	{
		lowLevelClose(FileDescriptor_);
		FileDescriptor_ = -1;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// eof
BcBool BcFile::eof()
{
	return feof( FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// flush
void BcFile::flush()
{
	fflush( FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// tell
BcU32 BcFile::tell()
{
	return ftell( FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// seek
void BcFile::seek( BcU32 Position )
{
	fseek( FileHandle_, Position, 0 );
}

//////////////////////////////////////////////////////////////////////////
// read
void BcFile::read( void* pDest, BcU32 nBytes )
{
	BcAssert( FileHandle_ );
	
	fread( pDest, nBytes, 1, FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// readLine
void BcFile::readLine( BcChar* pBuffer, BcU32 Size )
{
	BcU32 BytesRead = 0;
	BcMemZero( pBuffer, Size );
	while( BytesRead < Size && !eof() )
	{
		read( &pBuffer[ BytesRead++ ], 1 );
		if( pBuffer[ BytesRead - 1 ] == '\n' )
		{
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// write
void BcFile::write( const void* pSrc, BcU32 nBytes )
{
	BcAssert( FileHandle_ );

	fwrite( pSrc, nBytes, 1, FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// writeLine
void BcFile::writeLine( const BcChar* pTest )
{
	BcAssert( FileHandle_ );

	fwrite( pTest, BcStrLength( pTest ), 1, FileHandle_ );
	fwrite( "\n", 2, 1, FileHandle_ );
}
