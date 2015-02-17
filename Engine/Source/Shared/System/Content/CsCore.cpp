/**************************************************************************
*
* File:		CsCore.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Content/CsCore.h"
#include "System/Content/CsRedirector.h"

#if !PLATFORM_HTML5
#include <boost/filesystem.hpp>
#endif // !PLATFORM_HTML5

SYS_CREATOR( CsCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
CsCore::CsCore()
{
	IsCollectingGarbage_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsCore::~CsCore()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void CsCore::open()
{
	// Register types for reflection.
	//CsResource::StaticRegisterReflection();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual 
void CsCore::update()
{
	processResources();
	freeUnreferencedPackages();
	processCallbacks();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual 
void CsCore::close()
{
	static BcF32 UnloadTimeout = 5.0f;
	BcTimer UnloadTimer;
	UnloadTimer.mark();
	while( PackageList_.size() > 0 )
	{
		freeUnreferencedPackages();

		// Finish processing unloading resources.
		if( Resources_.size() > 0 )
		{
			processResources();
		}

		// If we hit the timeout, just break out and shut down cleanly.
		if( UnloadTimer.time() > UnloadTimeout )
		{
			break;
		}
	}

	if( Resources_.size() > 0 )
	{
		BcPrintf( "==========================================\n" );
		BcPrintf( "CsCore: Dump Resource On Exit:\n" );
		BcPrintf( "==========================================\n" );
	
		TResourceListIterator It( Resources_.begin() );
		while( It != Resources_.end() )
		{
			CsResource* pResource = (*It);
			BcPrintf( "%s:%s \n", (*pResource->getName()).c_str(), (*pResource->getTypeName()).c_str() );
			++It;
		}
		BcPrintf( "==========================================\n" );
	}

	// Verify we don't have any left floating loaded or unloading.
	BcVerifyMsg( ProcessingResources_.size() == 0, "CsCore: Resources still loaded, but system is closing! Has the scene cleaned up properly?" );
	BcVerifyMsg( Resources_.size() == 0, "CsCore: Resources still unloading, but system is closing!" );
}

//////////////////////////////////////////////////////////////////////////
// freeUnreferencedPackages
void CsCore::freeUnreferencedPackages()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Search for existing package and move to unreferenced list.
	for( TPackageListIterator It( PackageList_.begin() ); It != PackageList_.end();  )
	{
		CsPackage* pPackage = (*It);
		if( pPackage->hasUnreferencedResources() )
		{
			pPackage->releaseUnreferencedResources();

			// If we've got no valid resources we can move to unreferenced list to destroy.
			if( pPackage->haveAnyValidResources() == BcFalse )
			{
				UnreferencedPackageList_.push_back( pPackage );
				It = PackageList_.erase( It );
			}
			else
			{
				++It;
			}
		}
		else
		{
			++It;
		}
	}

	// NOTE: WIP.
	// Delete all unreferenced packages.
	for( TPackageListIterator It( UnreferencedPackageList_.begin() ); It != UnreferencedPackageList_.end(); ++It )
	{
		CsPackage* pPackage = (*It);

		delete pPackage;
	}

	// Clear unreferenced packages.
	UnreferencedPackageList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// getResourceType
BcName CsCore::getResourceType( BcU32 Idx ) const
{
	// NOTE: Change the map to an array. We want fast lookups by index too :(
	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getNoofResourceTypes
BcU32 CsCore::getNoofResourceTypes() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// allocResource
CsResource* CsCore::allocResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	CsResource* pResource = static_cast< CsResource* >( ReConstructObject( Class, *Name, pPackage, nullptr ) );
	pResource->setIndex( Index );
	
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// getNoofResources
size_t CsCore::getNoofResources()
{
	return Resources_.size();
}

//////////////////////////////////////////////////////////////////////////
// getResource
ReObjectRef< CsResource > CsCore::getResource( size_t Idx )
{
	return Resources_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getResource
ReObjectRef< CsResource > CsCore::getResource( const BcChar* pFullName )
{
	ReObjectRef< CsResource > Handle;
	if( pFullName != NULL )
	{
		BcChar FullNameBuffer[ 1024 ];
		BcAssertMsg( BcStrLength( pFullName ) < sizeof( FullNameBuffer ), "CsPackageImporter: Full name too long." );
		BcStrCopy( FullNameBuffer, pFullName );
		BcChar* pPackageNameBuffer = NULL;
		BcChar* pResourceNameBuffer = NULL;
		BcChar* pTypeNameBuffer = NULL;

		BcAssertMsg( BcStrStr( FullNameBuffer, "." ) != NULL, "CsCore: Missing package from \"%s\"", FullNameBuffer );
		BcAssertMsg( BcStrStr( FullNameBuffer, ":" ) != NULL, "CsCore: Missing type from \"%s\"", FullNameBuffer );
		
		pPackageNameBuffer = &FullNameBuffer[ 0 ];
		pResourceNameBuffer = BcStrStr( FullNameBuffer, "." );
		pTypeNameBuffer = BcStrStr( FullNameBuffer, ":" );
		*pResourceNameBuffer++ = '\0';
		*pTypeNameBuffer++ = '\0';

		BcName PackageName = pPackageNameBuffer;
		BcName ResourceName = pResourceNameBuffer;

		internalFindResource( PackageName, ResourceName, ReManager::GetClass( pTypeNameBuffer ), Handle );

		BcAssertMsg( Handle.isValid(), "CsCore: Unable to find \"%s\"", FullNameBuffer );
	}

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// requestPackage
CsPackage* CsCore::requestPackage( const BcName& Package )
{
	CsPackage* pPackage = findPackage( Package );
	if( pPackage != NULL )
	{
		return pPackage;
	}

	// Check for a packed package.
	BcPath PackedPackage( getPackagePackedPath( Package ) );
	BcPath ImportPackage( getPackageImportPath( Package ) );
	BcBool PackageExists = FsCore::pImpl()->fileExists( (*PackedPackage).c_str() ) || FsCore::pImpl()->fileExists( (*ImportPackage).c_str() );

	// If it exists, create it. Internally it will trigger it's own load.
	if( PackageExists )
	{
		pPackage = new CsPackage( Package );
		PackageList_.push_back( pPackage );
	}	
	else
	{
		BcAssertMsg( BcFalse, "CsCore: Can't import package, missing \"%s\" or \"%s\"", (*PackedPackage).c_str(), (*ImportPackage).c_str() );
	}

	//
	return pPackage;	
}

//////////////////////////////////////////////////////////////////////////
// requestPackageReadyCallback
void CsCore::requestPackageReadyCallback( const BcName& Package, const CsPackageReadyCallback& Callback, BcU32 ID )
{
	TPackageReadyCallback PackageReadyCallback =
	{
		Package,
		Callback,
		ID
	};

	PackageReadyCallbackList_.push_back( PackageReadyCallback );
}

//////////////////////////////////////////////////////////////////////////
// findPackage
CsPackage* CsCore::findPackage( const BcName& Package )
{
	// Search for existing package.
	for( TPackageListIterator It( PackageList_.begin() ); It != PackageList_.end(); ++It )
	{
		if( (*It)->getName() == Package )
		{
			return (*It);
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getPackageImportPath
BcPath CsCore::getPackageImportPath( const BcName& Package )
{
	BcPath Path;
	Path.join( "Content", *Package + ".pkg" );
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getPackageIntermediatePath
BcPath CsCore::getPackageIntermediatePath( const BcName& Package )
{
	BcPath Path;
	if( Package != BcName::INVALID )
	{
		Path.join( "IntermediateContent", *Package + ".pak" );
	}
	else
	{
		Path = "IntermediateContent";
	}

#if !PLATFORM_HTML5
	boost::filesystem::create_directories( *Path );
#endif // !PLATFORM_HTML5

	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getPackagePackedPath
BcPath CsCore::getPackagePackedPath( const BcName& Package )
{
	BcPath Path;
	if( Package != BcName::INVALID )
	{
		Path.join( "PackedContent", *Package + ".pak" );
	}
	else
	{
		Path = "PackedContent";
	}
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getIntermediatePath
BcPath CsCore::getIntermediatePath( const std::string& SubFolder )
{
	BcPath Path;
	Path.join( "IntermediateContent", SubFolder + ".pak" );

#if !PLATFORM_HTML5
	boost::filesystem::create_directories( *Path );
#endif // !PLATFORM_HTML5
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// processResources
void CsCore::processResources()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Copy precreate in.
	for( auto Resource : PreprocessResources_ )
	{
		BcAssert( Resource->getName() != BcName::INVALID );
		BcAssert( std::find( 
			ProcessingResources_.begin(), ProcessingResources_.end(), Resource ) == ProcessingResources_.end() );
		ProcessingResources_.push_back( Resource );
	}
	PreprocessResources_.clear();
	
	// Iterate processing resources.
	while( ProcessingResources_.size() > 0 )
	{
		CsResource* Resource = *ProcessingResources_.begin();
			
		// Create resource.
		if( Resource->getInitStage() < Resource->getTargetInitStage() )
		{
			switch( Resource->getTargetInitStage() )
			{
			case CsResource::INIT_STAGE_INITIAL:
				{
					BcAssertMsg( 0, "Invalid init stage combination." );
				}
				break;

			case CsResource::INIT_STAGE_CREATE:
				{
					BcAssert( Resource->getInitStage() == CsResource::INIT_STAGE_INITIAL );
					Resource->advanceInitStage();
					Resource->create();
					BcAssert( Resource->getTargetInitStage() == CsResource::INIT_STAGE_READY );
				}
				break;

			case CsResource::INIT_STAGE_READY:
				{
					Resource->advanceInitStage();
				}
				break;

			case CsResource::INIT_STAGE_DESTROY:
				{
					// If we're ready, then destroy. If we aren't ready, skip to free.
					if( Resource->getInitStage() == CsResource::INIT_STAGE_READY )
					{
						Resource->destroy();
					}

					// Destruct and free.
					Resource->getClass()->destruct( Resource );
					BcMemFree( Resource );

					// Remove entirely.
					std::remove( Resources_.begin(), Resources_.end(), Resource );
				}
				break;
			}
		}
		else
		{
			// Erase if no processing required.
			ProcessingResources_.erase( ProcessingResources_.begin() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// processCallbacks
void CsCore::processCallbacks()
{
	TPackageReadyCallbackListIterator It = PackageReadyCallbackList_.begin();

	while( It != PackageReadyCallbackList_.end() )
	{
		TPackageReadyCallback& CallbackData( *It );
		CsPackage* pPackage = findPackage( CallbackData.Package_ );
		if( pPackage != NULL &&
			pPackage->isReady() )
		{
			CallbackData.Callback_( pPackage, CallbackData.ID_ );
			It = PackageReadyCallbackList_.erase( It );
		}
		else
		{
			++It;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// internalAddResource
void CsCore::internalAddResource( CsResource* Resource )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Put into resource list.
	BcAssert( std::find( Resources_.begin(), Resources_.end(), Resource ) == Resources_.end() );
	Resources_.push_back( Resource );

	internalAddResourceForProcessing( Resource );
}

//////////////////////////////////////////////////////////////////////////
// internalAddResourceForProcessing
void CsCore::internalAddResourceForProcessing( CsResource* Resource )
{
	// Put into preprocess resources list.
	auto FoundIt = 
		std::find( 
			PreprocessResources_.begin(), 
			PreprocessResources_.end(), 
			Resource );

	if( FoundIt == PreprocessResources_.end() )
	{
		PreprocessResources_.push_back( Resource );
	}
}

//////////////////////////////////////////////////////////////////////////
// internalCreateResource
BcBool CsCore::internalCreateResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage, ReObjectRef< CsResource >& Handle )
{
	// Generate a unique name for the resource.
	BcName UniqueName = Name.isValid() ? Name : BcName( Class->getName() ).getUnique();

	// Allocate resource with a unique name.
	Handle = allocResource( UniqueName, Class, Index, pPackage );

	//
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalRequestResource
BcBool CsCore::internalRequestResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle )
{
	// Find package
	CsPackage* pPackage = findPackage( Package );
	if( pPackage || Package == BcName::NONE )
	{
		// If we can't find resource, throw an error.
		if( internalFindResource( Package, Name, Class, Handle ) == BcFalse )
		{
			BcPrintf( "CsCore::requestResource: Resource not availible \"%s.%s:%s\" requested.\n", (*Package).c_str(), (*Name).c_str(), (*Class->getName()).c_str() );
		}
	}
	else
	{
		BcPrintf( "CsCore::requestResource: Invalid package \"%s\" requested.\n", (*Package).c_str() );
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalFindResource
BcBool CsCore::internalFindResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Make the handle null, this method must return a failure correctly.
	Handle = nullptr;

	// Function to find matching resource in a given list.
	// Used to facilitate looking into multiple lists, but we
	// have unified in to one list for all resources,
	// and individual ones for processing.
	auto findResourceInList = [ this, Package, Name, Class ]( TResourceList& List ) -> CsResource*
	{
		for( auto Resource : List )
		{
			if( Package != BcName::NONE )
			{
				if( Resource->getPackageName() == Package && 
					Resource->getName() == Name )
				{
					// Go into redirector targets.
					while( Resource->isTypeOf< CsRedirector >() )
					{
						ReObjectRef< CsRedirector > Redirector( Resource );
						Resource = Redirector->getResource();
					}
	
					if( Resource->isTypeOf( Class ) )
					{
						return Resource;
					}
				}
			}
			else
			{
				if( Resource->getName() == Name )
				{
					// Go into redirector targets.
					while( Resource->isTypeOf< CsRedirector >() )
					{
						ReObjectRef< CsRedirector > Redirector( Resource );
						Resource = Redirector->getResource();
					}
	
					if( Resource->isTypeOf( Class ) )
					{
						return Resource;
					}
				}
			}
		}

		return nullptr;
	};

	CsResource* Resource = findResourceInList( Resources_ );

	// Assign to handle and return.
	Handle = Resource;
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// getResourceByUniqueId
ReObjectRef< CsResource > CsCore::getResourceByUniqueId(BcU32 UId)
{
	return ReObject::StaticFindByUniqueId( UId );
}
