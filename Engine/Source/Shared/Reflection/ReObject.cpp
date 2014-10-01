#include "Reflection/ReObject.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Object class definition
REFLECTION_DEFINE_BASE( ReObject );
	
void ReObject::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
#if REFLECTION_ENABLE_GC
		new ReField( "RefCount_", &ReObject::RefCount_, bcRFF_TRANSIENT ),
#endif
		new ReField( "Flags_", &ReObject::Flags_ ),
		new ReField( "Owner_", &ReObject::Owner_, bcRFF_SHALLOW_COPY ),
		new ReField( "Basis_", &ReObject::Basis_, bcRFF_SHALLOW_COPY | bcRFF_BASIS ),
		new ReField( "Name_", &ReObject::Name_ ),
#if REFLECTION_ENABLE_SIMPLE_UNIQUE_ID
		new ReField( "UniqueId_", &ReObject::UniqueId_, bcRFF_TRANSIENT ),
#endif
	};
		
	ReRegisterClass< ReObject >( Fields );
}

std::atomic< BcU32 > ReObject::UniqueIdCounter_;

//////////////////////////////////////////////////////////////////////////
// Ctor
ReObject::ReObject():
	Owner_( nullptr ),
	Basis_( nullptr )
{
	StaticAdd( this );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ReObject::ReObject( ReNoInit ):
	Owner_( nullptr ),
	Basis_( nullptr )
{
	StaticAdd( this );
}
	
//////////////////////////////////////////////////////////////////////////
// Dtor
ReObject::~ReObject()
{
	// Remove from global object list.
	StaticRemove( this );

	// Handle destruction notification.
	if( Flags_ & (BcU32)ReObject::Flags::NotifyOnDeletion )
	{
		std::lock_guard< std::mutex > Lock( ObjectNotifyMutex_ );
		auto ObjectNotifyListIt = ObjectNotifyMap_.find( this );

		// If we find an entry, move list out, erase entry, and call all notifiers.
		if( ObjectNotifyListIt != ObjectNotifyMap_.end() )
		{
			auto ObjectNotifyList = std::move( ObjectNotifyListIt->second );

			for( auto ObjectNotify : ObjectNotifyList )
			{
				ObjectNotify->onObjectDeleted( this );
			}

			ObjectNotifyMap_.erase( ObjectNotifyListIt );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setName
void ReObject::setName( BcName Name )
{
	Name_ = Name;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& ReObject::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// setBasis
void ReObject::setBasis( ReObject* Basis )
{
	Basis_ = Basis;
}

//////////////////////////////////////////////////////////////////////////
// getBasis
ReObject* ReObject::getBasis() const
{
	return Basis_;
}

//////////////////////////////////////////////////////////////////////////
// getFullName
std::string ReObject::getFullName() const
{
	const ReObject* Object = this;
	std::string Name = *getName();
	while( Object->Owner_ != nullptr )
	{
		Object = Object->Owner_;
		Name = *Object->getName() + "/" + Name;
	}
	return Name;
}

//////////////////////////////////////////////////////////////////////////
// addNotifier
void ReObject::addNotifier( ReIObjectNotify* ObjectNotify ) const
{
	// Add notifier flag so it knows to notify.
	Flags_ |= (BcU32)ReObject::Flags::NotifyOnDeletion;

	std::lock_guard< std::mutex > Lock( ObjectNotifyMutex_ );
	auto ObjectNotifyListIt = ObjectNotifyMap_.find( this );

	// If we find an entry, move list out, erase entry, and call all notifiers.
	if( ObjectNotifyListIt != ObjectNotifyMap_.end() )
	{
		ObjectNotifyListIt->second.push_back( ObjectNotify );
	}
	else
	{
		auto& ObjectNotifyListRet = ObjectNotifyMap_[ this ] = ObjectNotifyList( 1 );
		ObjectNotifyListRet.push_back( ObjectNotify );
	}
}
			
//////////////////////////////////////////////////////////////////////////
// removeNotifier
void ReObject::removeNotifier( ReIObjectNotify* ObjectNotify ) const
{
	BcAssertMsg( Flags_ & (BcU32)ReObject::Flags::NotifyOnDeletion, "Can't remove notifier from object that is flagged to not notify!" );

	std::lock_guard< std::mutex > Lock( ObjectNotifyMutex_ );
	auto ObjectNotifyListIt = ObjectNotifyMap_.find( this );

	BcAssertMsg( ObjectNotifyListIt != ObjectNotifyMap_.end(), "Can't remove notifier from object without a notification list." );

	ObjectNotifyListIt->second.remove( ObjectNotify );
}

//////////////////////////////////////////////////////////////////////////
// getOwner
ReObject* ReObject::getOwner() const
{
	return Owner_;
}

//////////////////////////////////////////////////////////////////////////
// setOwner
void ReObject::setOwner( ReObject* Owner )
{
		Owner_ = Owner;
}

//////////////////////////////////////////////////////////////////////////
// getRootOwner
ReObject* ReObject::getRootOwner() const
{
    ReObject* Owner = Owner_;
	while( Owner != nullptr &&
	       Owner->Owner_ != nullptr &&
		   Owner != Owner->Owner_ )
	{
		Owner = Owner->Owner_;
	}
	return Owner;
}

//////////////////////////////////////////////////////////////////////////
// setRootOwner
void ReObject::setRootOwner( ReObject* RootOwner )
{
    ReObject* Owner = this;

	for(;;)
	{
		if( Owner->Owner_ == nullptr || Owner == Owner->Owner_ )
		{
			Owner->Owner_ = RootOwner;
			return;
		}
		else
		{
			Owner = Owner->Owner_;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Statics
std::mutex ReObject::ObjectListMutex_;
ReObject::ObjectList ReObject::ObjectList_;
std::mutex ReObject::ObjectNotifyMutex_;
ReObject::ObjectNotifyMap ReObject::ObjectNotifyMap_;

//////////////////////////////////////////////////////////////////////////
// StaticAdd
//static
void ReObject::StaticAdd( ReObject* Object )
{
	std::lock_guard< std::mutex > Lock( ObjectListMutex_ );
	ObjectList_.push_back( Object );

#if REFLECTION_ENABLE_SIMPLE_UNIQUE_ID
	Object->UniqueId_ = UniqueIdCounter_++;
#endif
}

//////////////////////////////////////////////////////////////////////////
// StaticRemove
//static
void ReObject::StaticRemove( ReObject* Object )
{
	std::lock_guard< std::mutex > Lock( ObjectListMutex_ );
	ObjectList_.remove( Object );
}

//////////////////////////////////////////////////////////////////////////
// StaticRemove
//static
ReObject* ReObject::StaticFindByUniqueId( BcU32 UniqueId )
{
	std::lock_guard< std::mutex > Lock( ObjectListMutex_ );
	auto Object = std::find_if( ObjectList_.begin(), ObjectList_.end(), 
		[ UniqueId ]( ReObject* Object )
		{
			return Object->getUniqueId() == UniqueId;
		});
	return Object != ObjectList_.end() ? *Object : nullptr;
}

//////////////////////////////////////////////////////////////////////////
// StaticCollectGarbage
//static
void ReObject::StaticCollectGarbage()
{
	// This is mostly a functional placeholder before we experiment
	// with kicking off GC as a job so it can run in parallel to the
	// game.
#if REFLECTION_ENABLE_GC
	// Lock and grab a copy of the object list.
	ObjectListMutex_.lock();
	ObjectList ObjectList( ObjectList_ );
	ObjectListMutex_.unlock();

	// Iterate over all the objects, check their flags, and delete
	// if required.
	for( auto Object : ObjectList )
	{
		if( ( Object->Flags_ & (BcU32)ReObject::Flags::MarkedForDeletion ) != 0 )
		{
			delete Object;
		}
	}
#endif
}
