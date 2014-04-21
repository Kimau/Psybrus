#include "Serialisation/SeJsonReader.h"

#include "Base/BcMemory.h"

#include <fstream>

//////////////////////////////////////////////////////////////////////////
// Statics
const char* SeJsonReader::SerialiserVersionEntry = "SerialiserVersion";
const char* SeJsonReader::RootIDEntry = "RootID";
const char* SeJsonReader::ObjectsEntry = "Objects";
const char* SeJsonReader::ClassEntry = "Class";
const char* SeJsonReader::IDEntry = "ID";
const char* SeJsonReader::MembersEntry = "Members";
const char* SeJsonReader::FieldEntry = "Field";
const char* SeJsonReader::ValueEntry = "Value";

//////////////////////////////////////////////////////////////////////////
// Ctor
SeJsonReader::SeJsonReader( const char* FileName ):
    InputFile_( FileName )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SeJsonReader::~SeJsonReader()
{

}

//////////////////////////////////////////////////////////////////////////
// getVersion
//virtual
BcU32 SeJsonReader::getVersion() const
{
    return SERIALISER_VERSION;
}

//////////////////////////////////////////////////////////////////////////
// getFileVersion
//virtual
BcU32 SeJsonReader::getFileVersion() const
{
    return FileVersion_;
}

//////////////////////////////////////////////////////////////////////////
// internalSerialise
//virtual
void* SeJsonReader::internalSerialise( void* pData, const ReType* pType )
{
    // Read in the json file.
    Json::Reader Reader;
    std::ifstream InStream;
    InStream.open( InputFile_ );
    Reader.parse( InStream, RootValue_ );
    InStream.close();

    Json::Value& RootIDEntry( RootValue_[ RootIDEntry ] );
    Json::Value& VersionEntry( RootValue_[ SerialiserVersionEntry ] );
    Json::Value& ObjectsValue( RootValue_[ ObjectsEntry ] );

    // Grab the file version.
    FileVersion_ = VersionEntry.asUInt();

    // Create all object types and add to list.
    for( auto It( ObjectsValue.begin() ); It != ObjectsValue.end(); ++It )
    {
        auto ObjectToSerialise( *It );
        auto ClassType( ReManager::GetClass( ObjectToSerialise[ ClassEntry ].asString() ) );
        if( ClassType->getTypeSerialiser() != nullptr )
        {
            auto ID( ObjectToSerialise[ IDEntry ].asUInt() );
            void* pClassObject = nullptr;
            if( ID == RootIDEntry.asUInt() )
            {
                if( pData != nullptr )
                {
                    pClassObject = pData;
                }
                else
                {
                    pClassObject = ClassType->constructNoInit< void >();
                    pData = pClassObject;
                }
            }
            else
            {
                pClassObject = ClassType->constructNoInit< void >();
            }

            // Add class to list for look up.
            SerialiseClasses_.push_back( SerialiseClass( ID, pClassObject, ClassType ) );
        }
    }

    // Serialise in.
    for( auto It( ObjectsValue.begin() ); It != ObjectsValue.end(); ++It )
    {
        auto ObjectToSerialise( *It );
        auto ClassType( ReManager::GetClass( ObjectToSerialise[ ClassEntry ].asString() ) );
        auto ID( ObjectToSerialise[ IDEntry ].asUInt() );
        auto ClassToSerialise( getSerialiseClass( ID, ClassType ) );

        // Add class to list for look up.
        serialiseClass( ClassToSerialise.pData_, ClassType, ObjectToSerialise );
    }

    return pData;
}

//////////////////////////////////////////////////////////////////////////
// serialiseClass
//virtual
void SeJsonReader::serialiseClass( void* pData, const ReClass* pClass, Json::Value& InputValue )
{
    // Get type serialiser.
    auto Serialiser = pClass->getTypeSerialiser();
    std::string OutString;

    bool Success = false;

    // Attempt conversion to string.
    if( InputValue.type() == Json::stringValue &&
        Serialiser->serialiseFromString( pData, InputValue.asString() ) )
    {
        Success = true;
    }
    else if( InputValue.type() == Json::objectValue )
    {
        Json::Value ValueValue = InputValue.get( ValueEntry, Json::nullValue );
        if( ValueValue.type() == Json::stringValue &&
            Serialiser->serialiseFromString( pData, ValueValue.asString() ) )
        {
            Success = true;
        }
    }

    if( Success == false )
    {
        // Iterate over members to add, all supers too.
        const ReClass* pProcessingClass = pClass;
        while( pProcessingClass != nullptr )
        {
            // If this class has fields, then iterate over them.
            if( pProcessingClass->getNoofFields() > 0 )
            {
                Json::Value& MembersValue( InputValue[ MembersEntry ] );
                for( BcU32 Idx = 0; Idx < pProcessingClass->getNoofFields(); ++Idx )
                {
                    const ReField* pField = pProcessingClass->getField( Idx );
                    auto Members = MembersValue.getMemberNames();
                    if( std::find( Members.begin(), Members.end(), *pField->getName() ) != Members.end() )
                    {
                        serialiseField( pData, pField, MembersValue[ *pField->getName() ] );
                    }
                }
            }
            pProcessingClass = pProcessingClass->getSuper();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
void SeJsonReader::serialiseField( void* pData, const ReField* pField, Json::Value& InputValue )
{
    // Select the appropriate serialise method to use if we
    // have some data to serialise.
    if( pData != nullptr )
    {
        if( pField->isContainer() )
        {
            if( pField->getKeyType() == nullptr )
            {
                serialiseArray( pData, pField, InputValue );
            }
            else
            {
                serialiseDict( pData, pField, InputValue );
            }
        }
        else
        {
            if( pField->getType()->isTypeOf< ReClass >() )
            {
                if( pField->isPointerType() == false )
                {
                    serialiseClass( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ), InputValue );
                }
                else
                {
                    void* pFieldData = nullptr;
                    serialisePointer( pFieldData, static_cast< const ReClass* >( pField->getType() ), pField->getFlags(), InputValue, true );
                    void** pOutputFieldData = reinterpret_cast< void** >( reinterpret_cast< BcU8* >( pData ) + pField->getOffset() );
                    *pOutputFieldData = pFieldData;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// serialisePointer
//virtual
void SeJsonReader::serialisePointer( void*& pData, const ReClass* pClass, BcU32 FieldFlags, Json::Value& InputValue, BcBool IncrementRefCount )
{
    auto ClassToSerialise = getSerialiseClass( InputValue.asUInt(), pClass );
    if( ClassToSerialise.pData_ != nullptr && ClassToSerialise.pData_ != pData )
    {
        if( ( FieldFlags & bcRFF_SIMPLE_DEREF ) != 0 )
        {
            pData = ClassToSerialise.pData_;

#if REFLECTION_ENABLE_GC
            // Object references need ref count increased.
            if( ( FieldFlags & bcRFF_OBJECT_REFERENCE ) != 0 && IncrementRefCount )
            {
                reinterpret_cast< ReObject* >( pData )->incRefCount();
            }
#endif
        }
    }
    else
    {
        pData = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
// serialiseArray
//virtual
void SeJsonReader::serialiseArray( void* pData, const ReField* pField, Json::Value& InputValue )
{
    Json::Value ArrayValue( Json::arrayValue );
    auto pFieldValueType = pField->getValueType();
    auto ValueSerialiser = pFieldValueType->getTypeSerialiser();
    auto pWriteIterator = pField->newWriteIterator( pField->getData< void >( pData ) );

    // Clear container out.
    pWriteIterator->clear();

    // Construct a temporary value.
    void* pTemporaryValue = static_cast< const ReClass* >( pFieldValueType )->construct< void >();

    // Iterate over Json values.
    for( auto ValueIt( InputValue.begin() ); ValueIt != InputValue.end(); ++ValueIt )
    {
        if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
        {
            serialiseClass( pTemporaryValue, static_cast< const ReClass* >( pFieldValueType ), (*ValueIt) );
            pWriteIterator->add( pTemporaryValue );
        }
        else
        {
            void* pTemporaryPointer = nullptr;
            serialisePointer( pTemporaryPointer, static_cast< const ReClass* >( pFieldValueType ), pField->getValueFlags(), (*ValueIt), false );
            pWriteIterator->add( &pTemporaryPointer );
        }
    }

    // Free temporary value.
    BcMemFree( pTemporaryValue );

    delete pWriteIterator;
}

//////////////////////////////////////////////////////////////////////////
// serialiseDict
//virtual
void SeJsonReader::serialiseDict( void* pData, const ReField* pField, Json::Value& InputValue )
{
    Json::Value ArrayValue( Json::arrayValue );
    auto pFieldKeyType = pField->getKeyType();
    auto pFieldValueType = pField->getValueType();
    auto KeySerialiser = pFieldKeyType->getTypeSerialiser();
    auto ValueSerialiser = pFieldValueType->getTypeSerialiser();
    auto pWriteIterator = pField->newWriteIterator( pField->getData< void >( pData ) );

    // Clear container out.
    pWriteIterator->clear();

    // Construct a temporary value & key.
    void* pTemporaryKey = static_cast< const ReClass* >( pFieldKeyType )->construct< void >();
    void* pTemporaryValue = static_cast< const ReClass* >( pFieldValueType )->construct< void >();

    // Iterate over Json member values.
    auto MemberKeys = InputValue.getMemberNames();
    for( auto KeyIt( MemberKeys.begin() ); KeyIt != MemberKeys.end(); ++KeyIt )
    {
        auto Value = InputValue[ *KeyIt ];

        // Serialise key.
        if( KeySerialiser->serialiseFromString( pTemporaryKey, *KeyIt ) )
        {
            if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
            {
                // Serialise value.
                serialiseClass( pTemporaryValue, static_cast< const ReClass* >( pFieldValueType ), Value );
                pWriteIterator->add( pTemporaryKey, pTemporaryValue );
            }
            else
            {
                void* pTemporaryPointer = nullptr;
                serialisePointer( pTemporaryPointer, static_cast< const ReClass* >( pFieldValueType ), pField->getValueFlags(), Value, false );
                pWriteIterator->add( pTemporaryKey, &pTemporaryPointer );
            }
        }
    }

    // Free temporary value.
	BcMemFree( pTemporaryKey );
    BcMemFree( pTemporaryValue );

    delete pWriteIterator;
}

//////////////////////////////////////////////////////////////////////////
// getSerialiseClass
SeJsonReader::SerialiseClass SeJsonReader::getSerialiseClass( size_t ID, const ReType* pType )
{
    auto FoundClass = std::find( SerialiseClasses_.begin(), SerialiseClasses_.end(), SerialiseClass( ID, nullptr, pType ) );
    if( FoundClass != SerialiseClasses_.end() )
    {
        return *FoundClass;
    }
    return SerialiseClass( 0, nullptr, nullptr );
}