/**************************************************************************
*
* File:		Rendering/ScnModel.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMODEL_H__
#define __SCNMODEL_H__

#include "System/Renderer/RsCore.h"

#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"

#include "System/Scene/Rendering/ScnModelFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelRef
typedef ReObjectRef< class ScnModel > ScnModelRef;

//////////////////////////////////////////////////////////////////////////
// ScnModelComponentRef
typedef ReObjectRef< class ScnModelComponent > ScnModelComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnModel
class ScnModel:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnModel, CsResource );
		
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	friend class ScnModelComponent;
	
	// Cached pointers for internal use.
	ScnModelHeader* pHeader_;
	ScnModelNodeTransformData* pNodeTransformData_;
	ScnModelNodePropertyData* pNodePropertyData_;
	BcU8* pVertexBufferData_;
	BcU8* pIndexBufferData_;
	RsVertexElement* pVertexElements_;
	ScnModelMeshData* pMeshData_;
	
	ScnModelMeshRuntimeList MeshRuntimes_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelComponent
class ScnModelComponent:
	public ScnRenderableComponent
{
public:
	DECLARE_RESOURCE( ScnModelComponent, ScnRenderableComponent );

	virtual void						initialise( const Json::Value& Object, ScnModelRef Parent );
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();

	virtual MaAABB						getAABB() const;

	BcU32								findNodeIndexByName( const BcName& Name ) const;
	void								setNode( BcU32 NodeIdx, const MaMat4d& LocalTransform );
	const MaMat4d&						getNode( BcU32 NodeIdx ) const;
	BcU32								getNoofNodes() const;

	ScnMaterialComponentRef				getMaterialComponent( BcU32 Index );
	ScnMaterialComponentRef				getMaterialComponent( const BcName& MaterialName );
	ScnMaterialComponentList			getMaterialComponents( const BcName& MaterialName );
	
public:
	virtual void						postUpdate( BcF32 Tick );

	void								updateNodes( MaMat4d RootMatrix );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	void								render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	
protected:
	ScnModel*							Parent_;
	ScnModelNodeTransformData*			pNodeTransformData_;
	BcU32								Layer_;
	BcU32								Pass_;
	SysFence							UploadFence_;
	SysFence							UpdateFence_;

	MaAABB								AABB_;

	struct TPerComponentMeshData
	{
		ScnMaterialComponentRef MaterialComponentRef_;
		RsBuffer*		UniformBuffer_;
	};
	
	typedef std::vector< TPerComponentMeshData > TPerComponentMeshDataList;
	
	TPerComponentMeshDataList		PerComponentMeshDataList_;
};

#endif