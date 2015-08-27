/**************************************************************************
*
* File:		RsContextD3D12.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXTD3D12_H__
#define __RSCONTEXTD3D12_H__

#include "System/Renderer/RsContext.h"
#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/D3D12/RsPipelineStateCacheD3D12.h"
#include "System/Renderer/D3D12/RsDescriptorHeapCacheD3D12.h"
#include "System/Renderer/D3D12/RsLinearHeapAllocatorD3D12.h"

#include "Base/BcMisc.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// RsContextD3D12
class RsContextD3D12:
	public RsContext
{
public:
	RsContextD3D12( OsClient* pClient, RsContextD3D12* pParent );
	virtual ~RsContextD3D12();
	
	virtual OsClient* getClient() const override;
	virtual const RsFeatures& getFeatures() const override;

	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const override;
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const override;

	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	void beginFrame( BcU32 Width, BcU32 Height ) override;
	void endFrame() override;
	void takeScreenshot() override;
	void setViewport( const class RsViewport& Viewport ) override;
	void setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height ) override;

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( class RsRenderState* RenderState );
	void setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState );
	void setTexture( BcU32 SlotIdx, class RsTexture* pTexture, BcBool Force = BcFalse );
	void setProgram( class RsProgram* Program );
	void setIndexBuffer( class RsBuffer* IndexBuffer );
	void setVertexBuffer( 
		BcU32 StreamIdx, 
		class RsBuffer* VertexBuffer,
		BcU32 Stride );
	void setUniformBuffer( 
		BcU32 Handle, 
		class RsBuffer* UniformBuffer );
	void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );
	void setFrameBuffer( class RsFrameBuffer* FrameBuffer );
	void clear(
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil );

	void drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices );
	void drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset );

	bool createRenderState(
		RsRenderState* RenderState );
	bool destroyRenderState(
		RsRenderState* RenderState );
	bool createSamplerState(
		RsSamplerState* SamplerState );
	bool destroySamplerState(
		RsSamplerState* SamplerState );

	bool createFrameBuffer( 
		class RsFrameBuffer* FrameBuffer );
	bool destroyFrameBuffer( 
		class RsFrameBuffer* FrameBuffer );

	bool createBuffer( 
		RsBuffer* Buffer );
	bool destroyBuffer( 
		RsBuffer* Buffer );
	bool updateBuffer( 
		RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc );

	bool createTexture( 
		class RsTexture* Texture );
	bool destroyTexture( 
		class RsTexture* Texture );
	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc );

	bool createShader(
		class RsShader* Shader );
	bool destroyShader(
		class RsShader* Shader );

	bool createProgram(
		class RsProgram* Program );
	bool destroyProgram(
		class RsProgram* Program );
	
	bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) override;

	void flushState();

	/**
	 * Flush out command list, and call a function post execute.
	 */
	void flushCommandList( std::function< void() > PostExecute );

	/**
	 * Create default root signature.
	 */
	void createDefaultRootSignature();

	/**
	 * Create default pipeline state.
	 */
	void createDefaultPSO();

	/**
	 * Create command list data.
	 */
	void createCommandListData( size_t NoofBuffers );

	/**
	 * Recreate backbuffers.
	 */
	void recreateBackBuffers( BcU32 Width, BcU32 Height );

	/**
	 * Get current command list.
	 */
	ID3D12GraphicsCommandList* getCurrentCommandList();

	/**
	 * Get current upload allocator.
	 */
	class RsLinearHeapAllocatorD3D12* getCurrentUploadAllocator();


protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

private:
	RsContextD3D12* Parent_;
	OsClient* Client_;

	BcU32 InsideBeginEnd_;
	BcU32 Width_;
	BcU32 Height_;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	ComPtr< IDXGIFactory4 > Factory_;
	ComPtr< IDXGIAdapter > Adapter_;
	ComPtr< IDXGISwapChain3 > SwapChain_;
	ComPtr< ID3D12Device > Device_;
	D3D_FEATURE_LEVEL FeatureLevel_;

	RsFeatures Features_;

	/// Command queue.
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc_;
	ComPtr< ID3D12CommandQueue > CommandQueue_;

	/// CommandListData
	/// TODO: Factor this stuff out into a new interface to allow for concurrency.
	struct CommandListData
	{
		CommandListData():
			CommandAllocator_(),
			CommandList_(),
			UploadAllocator_(),
			CompleteFence_(),
			CompletionValue_()
		{
		}

		CommandListData( CommandListData&& Other )
		{
			std::swap( CommandAllocator_, Other.CommandAllocator_ );
			std::swap( CommandList_, Other.CommandList_ );
			std::swap( UploadAllocator_, Other.UploadAllocator_ );
			std::swap( CompleteFence_, Other.CompleteFence_ );
			std::swap( CompletionValue_, Other.CompletionValue_ );
		}

		CommandListData& operator == ( CommandListData&& Other )
		{
			std::swap( CommandAllocator_, Other.CommandAllocator_ );
			std::swap( CommandList_, Other.CommandList_ );
			std::swap( UploadAllocator_, Other.UploadAllocator_ );
			std::swap( CompleteFence_, Other.CompleteFence_ );
			std::swap( CompletionValue_, Other.CompletionValue_ );
		}

		/// Command allocator.
		ComPtr< ID3D12CommandAllocator > CommandAllocator_;

		/// Graphics command list.
		ComPtr< ID3D12GraphicsCommandList > CommandList_;

		// Memory management.
		// TODO: To save memory overall, perhaps have one global one which is reset on frame 0?
		std::unique_ptr< class RsLinearHeapAllocatorD3D12 > UploadAllocator_;

		// Completion fence.
		ComPtr< ID3D12Fence > CompleteFence_;

		// Completion value to wait for.
		BcU64 CompletionValue_;
	};

	// Per frame.
	std::vector< CommandListData > CommandListDatas_;
	int CurrentCommandListData_;
	HANDLE WaitOnCommandListEvent_;

	/// Presenting.
	BcU64 FrameCounter_;
	BcU64 FlushCounter_;
	BcU32 NumSwapBuffers_;
	BcU32 CurrentSwapBuffer_;
	
	/// Graphics pipeline state management.
	ComPtr< ID3D12RootSignature > DefaultRootSignature_;
	ComPtr< ID3D12PipelineState > DefaultPSO_;
	std::unique_ptr< RsPipelineStateCacheD3D12 > PSOCache_;
	RsGraphicsPipelineStateDescD3D12 GraphicsPSODesc_;
	std::array< D3D12_VIEWPORT, MAX_RENDER_TARGETS > Viewports_;
	std::array< D3D12_RECT, MAX_RENDER_TARGETS > ScissorRects_;
	class RsFrameBuffer* FrameBuffer_;

	// Buffer views.
	std::array< D3D12_VERTEX_BUFFER_VIEW, MAX_VERTEX_STREAMS > VertexBufferViews_;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView_;
	
	// Descriptor heaps.
	std::unique_ptr< RsDescriptorHeapCacheD3D12 > DHCache_;
	RsDescriptorHeapSamplerStateDescArrayD3D12 SamplerStateDescs_;
	RsDescriptorHeapShaderResourceDescArrayD3D12 ShaderResourceDescs_;

	/// Backbuffer.
	std::vector< class RsFrameBuffer* > BackBufferFB_; // TODO: Need multiple.
	std::vector< class RsTexture* > BackBufferRT_;	// TODO: Need multiple.
	class RsTexture* BackBufferDS_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;


};

#endif