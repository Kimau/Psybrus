/**************************************************************************
*
* File:		ScnShader.cpp
* Author:	Neil Richardson 
* Ver/Date:	11/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnShader.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

#ifdef PSY_SERVER

#include "System/Scene/Import/ScnShaderImport.h"


//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnShader::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	ScnShaderImport ShaderImporter;
	return ShaderImporter.import( Importer, Object );
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnShader );

void ScnShader::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "pHeader_",			&ScnShader::pHeader_ ),
		ReField( "ShaderMappings_",		&ScnShader::ShaderMappings_ ),
		ReField( "ProgramMap_",			&ScnShader::ProgramMap_ ),
	};
		
	ReRegisterClass< ScnShader, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnShader::initialise()
{
	pHeader_ = NULL;

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnShader::create()
{
	// TODO: Move some of the fileChunk(...) code into here. Will do for now.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnShader::destroy()
{
	for( auto& ShaderMapping : ShaderMappings_ )
	{
		for( auto& ShaderEntry : ShaderMapping.Shaders_ )
		{
			RsCore::pImpl()->destroyResource( ShaderEntry.second );
		}

		ShaderMapping.Shaders_.clear();
	}

	for( auto& ProgramEntry : ProgramMap_ )
	{
		RsCore::pImpl()->destroyResource( ProgramEntry.second );
	}

	ProgramMap_.clear();
	pHeader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// getProgram
RsProgram* ScnShader::getProgram( BcU32 PermutationFlags )
{
	// Find best matching permutation.
	TProgramMapIterator BestIter = ProgramMap_.find( PermutationFlags );
	
	if( BestIter == ProgramMap_.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( TProgramMapIterator Iter = ProgramMap_.begin(); Iter != ProgramMap_.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (*Iter).first & PermutationFlags );
			if( FlagsSet > BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one.
	if( BestIter != ProgramMap_.end() )
	{
		return (*BestIter).second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getShader
RsShader* ScnShader::getShader( BcU32 PermutationFlags, ScnShader::TShaderMap& ShaderMap )
{
	// Find best matching permutation.
	TShaderMapIterator BestIter = ShaderMap.find( PermutationFlags );
	
	if( BestIter == ShaderMap.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( TShaderMapIterator Iter = ShaderMap.begin(); Iter != ShaderMap.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (*Iter).first & PermutationFlags );
			if( FlagsSet > BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one.
	if( BestIter != ShaderMap.end() )
	{
		return (*BestIter).second;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnShader::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnShader::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	BcBool NewPipeline = BcFalse;

	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		requestChunk( 0 );
		return;
	}

	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnShaderHeader*)pData;

		// Grab the rest of the chunks.
		const BcU32 TotalChunks = pHeader_->NoofProgramPermutations_ + pHeader_->NoofShaderPermutations_;
		for( BcU32 Idx = 0; Idx < TotalChunks; ++Idx )
		{
			requestChunk( ++ChunkIdx );
		}
	}
	else if( ChunkID == BcHash( "shader" ) )
	{
		ScnShaderUnitHeader* pShaderHeader = (ScnShaderUnitHeader*)pData;
		void* pShaderData = pShaderHeader + 1;
		BcU32 ShaderSize = getChunkSize( ChunkIdx ) - sizeof( ScnShaderUnitHeader );
			
		if( pShaderHeader->PermutationFlags_ != 0 )
		{
			RsShader* pShader = RsCore::pImpl()->createShader( pShaderHeader->ShaderType_, pShaderHeader->ShaderDataType_, pShaderData, ShaderSize );
			ShaderMappings_[ pShaderHeader->ShaderType_ ].Shaders_[ pShaderHeader->PermutationFlags_ ] = pShader;
		}
		else
		{
			// HACK
			if( pShaderHeader->ShaderCodeType_ == scnSCT_GLSL_150 )
			{
				RsShader* pShader = RsCore::pImpl()->createShader( pShaderHeader->ShaderType_, pShaderHeader->ShaderDataType_, pShaderData, ShaderSize );
				ShaderMappings_[ pShaderHeader->ShaderType_ ].Shaders_[ pShaderHeader->ShaderHash_ ] = pShader;
			}
		}
	}
	else if( ChunkID == BcHash( "program" ) )
	{
		BcU32 NoofShaders = 0;
		std::array< RsShader*, rsST_MAX > Shaders;

		// Generate program.
		ScnShaderProgramHeader* pProgramHeader = (ScnShaderProgramHeader*)pData;
		
		if( pProgramHeader->ShaderFlags_ != 0 )
		{
			for( BcU32 Idx = 0; Idx < rsST_MAX; ++Idx )
			{
				// If we expect this shader type, try to get it and assert on failure.
				if( ( ( 1 << Idx ) & pProgramHeader->ShaderFlags_ ) != 0 )
				{
					auto& ShaderMapping( ShaderMappings_[ Idx ] );
					RsShader* pShader = getShader( pProgramHeader->ProgramPermutationFlags_, ShaderMapping.Shaders_ );
					BcAssertMsg( pShader != NULL, "Shader for permutation %x is invalid in ScnShader %s\n", pProgramHeader->ProgramPermutationFlags_, (*getName()).c_str() );
					Shaders[ NoofShaders++ ] = pShader;
				}
			}
		}
		else
		{
			for( BcU32 Idx = 0; Idx < rsST_MAX; ++Idx )
			{
				if( pProgramHeader->ShaderHashes_[ Idx ] != 0 )
				{
					auto& ShaderMapping( ShaderMappings_[ Idx ] );
					RsShader* pShader = getShader( pProgramHeader->ShaderHashes_[ Idx ], ShaderMapping.Shaders_ );
					BcAssertMsg( pShader != NULL, "Shader for permutation %x is invalid in ScnShader %s\n", pProgramHeader->ProgramPermutationFlags_, (*getName()).c_str() );
					Shaders[ NoofShaders++ ] = pShader;
				}
			}
			NewPipeline = BcTrue;
		}

		pVertexAttributes_ = (RsProgramVertexAttribute*)( pProgramHeader + 1 );

		for( BcU32 Idx = 0; Idx < pProgramHeader->NoofVertexAttributes_; ++Idx )
		{
			markupName( pVertexAttributes_[ Idx ].AttributeName_ );
		}

		// HACK
		if( pProgramHeader->ShaderCodeType_ == scnSCT_GLSL_150 )
		{
			// Create program.
			RsProgram* pProgram = RsCore::pImpl()->createProgram( NoofShaders, &Shaders[ 0 ], pProgramHeader->NoofVertexAttributes_, pVertexAttributes_ );			
			ProgramMap_[ pProgramHeader->ProgramPermutationFlags_ ] = pProgram;
		}
	}

	// Mark ready if we've got all the programs we expect.
	if( NewPipeline == BcFalse )
	{
		if( ProgramMap_.size() == pHeader_->NoofProgramPermutations_ )
		{
			markCreate();
		}
	}
	else
	{
		// HACK
		if( ProgramMap_.size() == pHeader_->NoofProgramPermutations_ / 2 )
		{
			markCreate();
		}
	}

}
