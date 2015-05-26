#include "System/Scene/Rendering/ScnTileMap.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTileMapImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTileMap );

void ScnTileMap::StaticRegisterClass()
{
	auto& Class = ReRegisterClass< ScnTileMap, Super >();
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	//// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnTileMapImport::StaticGetClass(), 0 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMap::ScnTileMap():
	TileMapData_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
ScnTileMap::~ScnTileMap()
{
}

//////////////////////////////////////////////////////////////////////////
// findLayerIndexByName
BcU32 ScnTileMap::findLayerIndexByName( const BcChar* Name )
{
	for( BcU32 Idx = 0; Idx < TileMapData_->NoofLayers_; ++Idx )
	{
		auto& Layer = TileMapData_->Layers_[ Idx ];
		if( strcmp( Name, getString( Layer.Name_ ) ) == 0 )
		{
			return Idx;
		}
	}
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// isTileValid
bool ScnTileMap::isTileValid( BcU32 LayerIdx, BcS32 X, BcS32 Y ) const
{
	if( LayerIdx < TileMapData_->NoofLayers_ )
	{
		auto& Layer = TileMapData_->Layers_[ LayerIdx ];
		if( X >= 0 && X < Layer.Width_ && 
		    Y >= 0 && Y < Layer.Height_ )
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// getTile
const ScnTileMapTile* ScnTileMap::getTile( BcU32 LayerIdx, BcS32 X, BcS32 Y ) const
{
	BcAssert( isTileValid( LayerIdx, X, Y ) );
	auto& Layer = TileMapData_->Layers_[ LayerIdx ];
	BcU32 Index = X + ( Y * Layer.Width_ );
	return &Layer.Tiles_[ Index ];
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnTileMap::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, TileMapData_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnTileMap::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "data" ) )
	{
		TileMapData_ = reinterpret_cast< ScnTileMapData* >( pData );

		markCreate();
		markReady();
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTileMapComponent );

void ScnTileMapComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "MaterialName_", &ScnTileMapComponent::MaterialName_, bcRFF_IMPORTER ),
		new ReField( "TileMap_", &ScnTileMapComponent::TileMap_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

		new ReField( "Canvas_", &ScnTileMapComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "Material_", &ScnTileMapComponent::Material_, bcRFF_TRANSIENT ),
	};

	using namespace std::placeholders;
	ReRegisterClass< ScnTileMapComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update",
					ScnComponentPriority::TILEMAP_UPDATE,
					std::bind( &ScnTileMapComponent::update, _1 ) ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMapComponent::ScnTileMapComponent():
	MaterialName_(),
	TileMap_( nullptr ),
	Canvas_( nullptr ),
	Material_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnTileMapComponent::~ScnTileMapComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// getTileLocalPosition
const ScnTileMapTile* ScnTileMapComponent::getTileLocalPosition( 
		BcU32 LayerIdx, const MaVec2d& LocalPosition ) const
{	
	BcS32 X = LocalPosition.x();
	BcS32 Y = LocalPosition.y();
	if( TileMap_->isTileValid( LayerIdx, X, Y ) )
	{
		return TileMap_->getTile( LayerIdx, X, Y );
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getTileWorldPosition
const ScnTileMapTile* ScnTileMapComponent::getTileWorldPosition( 
		BcU32 LayerIdx, const MaVec2d& WorldPosition ) const
{	
	// Transform to appropriate position.
	MaMat4d Transform = getParentEntity()->getWorldMatrix();
	Transform.inverse();
	auto LocalPosition = WorldPosition * Transform; 
	return getTileLocalPosition( LayerIdx, LocalPosition );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnTileMapComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	// Find a canvas to use for rendering (someone in ours, or our parent's hierarchy).
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	BcAssertMsg( Canvas_ != nullptr, "Sprite component needs to be attached to an entity with a canvas component in any parent!" );

	// Find a canvas to use for rendering (someone in ours, or our parent's hierarchy).
	Material_ = Parent->getComponentAnyParentByType< ScnMaterialComponent >( MaterialName_ );
	BcAssertMsg( Material_ != nullptr, "Sprite component needs to be attached to an entity with a material component in any parent!" );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnTileMapComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Canvas_ = nullptr;
	Material_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// draw
void ScnTileMapComponent::draw()
{
	ScnTileMapData* TileMapData = TileMap_->TileMapData_;

	const MaMat4d& Transform = getParentEntity()->getWorldMatrix();

	MaVec2d TileSize( TileMapData->TileWidth_, TileMapData->TileHeight_ );

	Canvas_->setMaterialComponent( Material_ );
	for( BcU32 LayerIdx = 0; LayerIdx < TileMapData->NoofLayers_; ++LayerIdx )
	{
		auto& Layer = TileMapData->Layers_[ LayerIdx ];
		auto LayerSize = MaVec2d( Layer.Width_, Layer.Height_ );

		for( BcU32 Y = 0; Y < Layer.Height_; ++Y )
		{
			for( BcU32 X = 0; X < Layer.Width_; ++X )
			{
				auto& Tile = Layer.Tiles_[ X + ( Y * Layer.Width_ ) ];
				MaVec2d Position = 
					MaVec2d( 
						static_cast< BcF32 >( X ) - LayerSize.x() * 0.5f, 
						static_cast< BcF32 >( Y ) - LayerSize.y() * 0.5f ) * TileSize;
				
				if( Tile.GID_ != 0 )
				{	
					Canvas_->drawSprite( 
						Position,
						TileSize,
						Tile.GID_ - 1,
						RsColour( 1.0f, 1.0f, 1.0f, Layer.Opacity_ ),
						LayerIdx );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//static
void ScnTileMapComponent::update( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnTileMapComponent >() );
		auto* TileMapComponent = static_cast< ScnTileMapComponent* >( Component.get() );

		TileMapComponent->draw();
	}
}
