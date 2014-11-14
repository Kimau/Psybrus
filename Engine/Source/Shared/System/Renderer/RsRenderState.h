/**************************************************************************
*
* File:		RsRenderState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render state creation and management.
*		
*
*
* 
**************************************************************************/

#ifndef __RENDERSTATE_H__
#define __RENDERSTATE_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsRenderTargetBlendState
struct RsRenderTargetBlendState
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsRenderTargetBlendState );
	RsRenderTargetBlendState( ReNoInit );
	RsRenderTargetBlendState();

	BcBool Enable_;
	RsBlendType SrcBlend_;
	RsBlendType DestBlend_;
	RsBlendOp BlendOp_;
	RsBlendType SrcBlendAlpha_;
	RsBlendType DestBlendAlpha_;
	RsBlendOp BlendOpAlpha_;
	BcU8 WriteMask_;
};

//////////////////////////////////////////////////////////////////////////
// RsBlendState
struct RsBlendState
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsBlendState );
	RsBlendState( ReNoInit );
	RsBlendState();

	RsRenderTargetBlendState RenderTarget_[ 8 ];
};

//////////////////////////////////////////////////////////////////////////
// RsStencilFaceState
struct RsStencilFaceState
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsStencilFaceState );
	RsStencilFaceState( ReNoInit );
	RsStencilFaceState();

	RsStencilOp Fail_;
	RsStencilOp DepthFail_;
	RsStencilOp Pass_;
	RsCompareMode Func_;
	BcU32 Ref_;
	BcU8 Mask_;
};

//////////////////////////////////////////////////////////////////////////
// RsDepthStencilState
struct RsDepthStencilState
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsDepthStencilState );
	RsDepthStencilState( ReNoInit );
	RsDepthStencilState();

	BcBool DepthTestEnable_;
	BcBool DepthWriteEnable_;
	RsCompareMode DepthFunc_;
	BcBool StencilEnable_;
	BcU8 StencilRead_;
	BcU8 StencilWrite_;
	RsStencilFaceState StencilFront_;
	RsStencilFaceState StencilBack_;
};

//////////////////////////////////////////////////////////////////////////
// RsRasteriserState
struct RsRasteriserState
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsRasteriserState );
	RsRasteriserState( ReNoInit );
	RsRasteriserState();

	RsFillMode FillMode_;
	RsCullMode CullMode_;
	BcF32 DepthBias_;
	BcF32 SlopeScaledDepthBias_;
	BcBool DepthClipEnable_;
	BcBool ScissorEnable_;
	BcBool AntialiasedLineEnable_;
};

//////////////////////////////////////////////////////////////////////////
// RsRenderStateDesc
struct RsRenderStateDesc
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsRenderStateDesc );
	
	RsRenderStateDesc( ReNoInit );
	RsRenderStateDesc();
	RsRenderStateDesc( 
		const RsBlendState& BlendState, 
		const RsDepthStencilState& DepthStencilState, 
		const RsRasteriserState& RasteriserState );

	RsBlendState BlendState_;
	RsDepthStencilState DepthStencilState_;
	RsRasteriserState RasteriserState_;
};

//////////////////////////////////////////////////////////////////////////
// RsRenderState
class RsRenderState:
	public RsResource
{
public:
	RsRenderState( class RsContext* pContext, const RsRenderStateDesc& Desc );
	virtual ~RsRenderState();

	/**
	 * Get desc.
	 */
	 const RsRenderStateDesc& getDesc() const;

private:
	RsRenderStateDesc Desc_;
};


#endif

