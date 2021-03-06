#include "Base/BcCommandLine.h"

#include <iostream>

//////////////////////////////////////////////////////////////////////////
// Ctor
BcCommandLine::BcCommandLine()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcCommandLine::BcCommandLine( const char* CmdLine )
{
	bool InQuotes = false;
	std::string Arg;
	while( char Char = *CmdLine )
	{
		bool FlushArg = false;
		if( Char == '\"' )
		{
			InQuotes = !InQuotes;
			if( InQuotes == false )
			{
				FlushArg = true;
			}
		}
		else if( InQuotes )
		{
			Arg += Char;
		}
		else
		{
			if( Char == ' ' )
			{
				FlushArg = true;
			}
			else
			{
				Arg += Char;
			}
		}

		if( FlushArg )
		{
			if( Arg.size() > 0 )
			{
				std::cout << "Arg: " << Arg << std::endl;
				Args_.push_back( std::move( Arg ) );
			}
		}

		++CmdLine;
	}

	if( Arg.size() > 0 )
	{
		std::cout << "Arg: " << Arg << std::endl;
		Args_.push_back( std::move( Arg ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcCommandLine::BcCommandLine( int Argc, const char* Argv[] )
{
	Args_.reserve( Argc - 1 );
	for( int Idx = 1; Idx < Argc; ++Idx )
	{
		Args_.push_back( Argv[ Idx ] );
	}
}

//////////////////////////////////////////////////////////////////////////
// hasArg
bool BcCommandLine::hasArg( const char Short, const char* Long ) const
{
	for( const auto& Arg : Args_ )
	{
		if( Arg.size() == 2 )
		{
			if( Arg[ 0 ] == '-' && Arg[ 1 ] == Short )
			{
				return true;
			}
		}
		else if( Arg.size() > 2 )
		{
			if( Arg[ 0 ] == '-' && Arg[ 1 ] == '-' )
			{
				if( Arg.substr( 2, std::string::npos ) == Long )
				{
					return true;
				}
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// getArg
bool BcCommandLine::getArg( const char Short, const char* Long, std::string& Out ) const
{
	for( size_t Idx = 0; Idx < Args_.size() - 1; ++Idx )
	{
		const auto& Arg0( Args_[ Idx ] );
		const auto& Arg1( Args_[ Idx + 1 ] );

		if( Arg0.size() == 2 )
		{
			if( Arg0[ 0 ] == '-' && Arg0[ 1 ] == Short )
			{
				Out = Arg1;
				return true;
			}
		}
		else if( Arg0.size() > 2 )
		{
			if( Arg0[ 0 ] == '-' && Arg0[ 1 ] == '-' )
			{
				if( Arg0.substr( 2, std::string::npos ) == Long )
				{
					Out = Arg1;
					return true;
				}
			}
		}
	}

	return false;
}
