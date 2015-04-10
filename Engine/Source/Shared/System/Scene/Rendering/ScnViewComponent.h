/**************************************************************************
*
* File:		Rendering/ScnViewComponent.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnViewComponent_H__
#define __ScnViewComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnCoreCallback.h"
#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewComponentRef
typedef ReObjectRef< class ScnViewComponent > ScnViewComponentRef;
typedef std::list< ScnViewComponentRef > ScnViewComponentList;
typedef ScnViewComponentList::iterator ScnViewComponentListIterator;
typedef ScnViewComponentList::const_iterator ScnViewComponentListConstIterator;
typedef std::map< std::string, ScnViewComponentRef > ScnViewComponentMap;
typedef ScnViewComponentMap::iterator ScnViewComponentMapIterator;
typedef ScnViewComponentMap::const_iterator ScnViewComponentMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnViewComponent
class ScnViewComponent:
	public ScnComponent,
	public ScnCoreCallback
{
public:
	REFLECTION_DECLARE_DERIVED( ScnViewComponent, ScnComponent );

	ScnViewComponent();
	virtual ~ScnViewComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	virtual void onAttachComponent( class ScnComponent* Component ) = 0;
	virtual void onDetachComponent( class ScnComponent* Component ) = 0;


	void setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const;
	void getWorldPosition( const MaVec2d& ScreenPosition, MaVec3d& Near, MaVec3d& Far ) const;
	MaVec2d getScreenPosition( const MaVec3d& WorldPosition ) const;
	BcU32 getDepth( const MaVec3d& WorldPos ) const;
	const RsViewport& getViewport() const;

	BcBool intersect( const MaAABB& AABB ) const;

	virtual void bind( class RsFrame* pFrame, RsRenderSort Sort );
	
	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

protected:
	void recreateFrameBuffer();

protected:

	// Viewport. Values relative to the size of the client being rendered into.
	BcF32								X_;
	BcF32								Y_;
	BcF32								Width_;
	BcF32								Height_;

	// Perspective projection.
	BcF32								Near_;
	BcF32								Far_;
	BcF32								HorizontalFOV_;		// Used by default.
	BcF32								VerticalFOV_;		// Used if HorizontalFOV_ is 0.0.

	RsColour							ClearColour_;
	BcBool								EnableClearColour_;
	BcBool								EnableClearDepth_;
	BcBool								EnableClearStencil_;	
	
	BcU32								RenderMask_;		// Used to determine what objects should be rendered for this view.


	// TODO: Remove this dependency, not really needed.
	RsViewport							Viewport_;

	// Uniform block data.
	ScnShaderViewUniformBlockData		ViewUniformBlock_;
	RsBuffer*					ViewUniformBuffer_;

	// Used for culling.
	// TODO: Move into BcFrustum, or perhaps a BcConvexHull?
	MaPlane								FrustumPlanes_[ 6 ];

	// Frame buffer + render target.
	ScnTextureRef						RenderTarget_;
	ScnTextureRef						DepthStencilTarget_;
	RsFrameBufferUPtr					FrameBuffer_;
};

#endif
