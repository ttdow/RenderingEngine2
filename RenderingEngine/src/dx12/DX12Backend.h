#pragma once

namespace Engine
{
	class DX12Backend
	{
	public:

		DX12Backend() = delete;
		DX12Backend(HINSTANCE hInstance, const HWND& hwnd);
		~DX12Backend();

		/**
		 * @brief Executes a full ray tracing rendering pass and presents the result to the screen.
		 * 
		 * This function resets the command list and allocator, updates the TLAS transforms,
		 * dispatches rays to a compute shader using DXR, copies the ray tracing output to
		 * the swapchain's back buffer, and then presents the frame.
		 * 
		 * Barriers are used throughout to ensure proper resource state transitions.
		 * 
		 * @throws std::runtime_error if command list or allocator cannot be reset or if the swapchain
		 * back buffer cannot be acquired.
		 */
		void Draw();

	private:

		static constexpr float quadVtx[18] =
		{
		  -1, 0, -1,
		  -1, 0,  1,
		   1, 0,  1,
		  -1, 0, -1,
		   1, 0, -1,
		   1, 0,  1
		};

		static constexpr float cubeVtx[24] =
		{
			-1, -1, -1, 1, -1, -1, -1, 1, -1, 1, 1, -1, -1, -1,  1, 1, -1,  1, -1, 1,  1, 1, 1,  1
		};

		static constexpr short cubeIdx[36] =
		{
			4, 6, 0, 2, 0, 6, 0, 1, 4, 5, 4, 1,
			0, 2, 1, 3, 1, 2, 1, 3, 5, 7, 5, 3,
			2, 6, 3, 7, 3, 6, 4, 5, 6, 7, 6, 5
		};

		static constexpr UINT NUM_INSTANCES = 3;
		static constexpr uint64_t NUM_SHADER_IDS = 3;
		uint64_t value;

		uint32_t frameIndex;
		HANDLE fenceEvent;
		uint64_t fenceValue;

		IDXGIFactory4* factory = nullptr;
		ID3D12Debug1* debugController = nullptr;

		ID3D12Device5* device = nullptr;
		ID3D12DebugDevice* debugDevice = nullptr;

		ID3D12CommandQueue* cmdQueue = nullptr;
		
		ID3D12Fence* fence = nullptr;
		IDXGISwapChain3* swapchain = nullptr;
		ID3D12DescriptorHeap* uavHeap = nullptr;
		ID3D12Resource* renderTarget = nullptr;

		ID3D12CommandAllocator* cmdAlloc = nullptr;
		ID3D12GraphicsCommandList4* cmdList = nullptr;

		ID3D12Resource* quadVB = nullptr;
		ID3D12Resource* cubeVB = nullptr;
		ID3D12Resource* cubeIB = nullptr;

		ID3D12Resource* quadBlas = nullptr;
		ID3D12Resource* cubeBlas = nullptr;

		ID3D12Resource* instances = nullptr;
		D3D12_RAYTRACING_INSTANCE_DESC* instanceData = nullptr;

		ID3D12Resource* tlas = nullptr;
		ID3D12Resource* tlasUpdateScratch = nullptr;

		ID3D12RootSignature* rootSignature = nullptr;

		ID3D12StateObject* pso = nullptr;
		ID3D12Resource* shaderIDs = nullptr;

		/**
		 * @brief Initializes the Direct3D 12 device and command infrastructure.
		 * 
		 * This function:
		 *  - Enables the D3D12 debug layer (in debug builds).
		 *  - Enumerates the DXGI adapters and selects the WARP adapter (software rasterizer).
		 *  - Creates the Direct3D 12 device with a minimum required feature level.
		 *  - Checks supported feature levels for diagnostics.
		 *  - Creates a command queue and synchronization fence.
		 * 
		 * @throws std::runtime_error if any step fails.
		 */
		void InitDevice();

		/**
		 * @brief Initializes the swapchain and UAV descriptor heap for the rendering surface.
		 * 
		 * This function:
		 *  - Creates a DXGI swapchain (via 'CreateSwapChainForHwnd') using the given window handle.
		 *  - Upgrades the swapchain to IDXGISwapChain3 for enhanced functionality.
		 *  - Initializes a descriptor heap for UAVs (used for ray tracing output).
		 *  - Releases the factory once no longer needed.
		 *  - Calls Resize() to create the initial render target based on the current window size.
		 * 
		 * @param hwnd Handle to the target window.
		 * @throws std::runtime_error if swapchain or descriptor heap creation fails.
		 */
		void InitSurfaces(const HWND& hwnd);

		// TODO.
		void InitCommand();

		/**
		 * @brief Initializes mesh vertex and index buffers for use in rendering.
		 * 
		 * This function creates GPU resources for vertex and index data, uploads the data
		 * to those buffers using the upload heap, and stores the resulting resource pointers.
		 * 
		 * It uses a helper lambda, MakeAndCopy, to avoid duplication across different mesh buffers.
		 * 
		 * The buffers are created in the upload heap ('D3D12_HEAP_TYPE_UPLOAD'), which allows
		 * CPU-writable, GPU-readable memory - ideal for small or static data uploads.
		 * 
		 * @throws std::runtime_error if buffer creation or mapping fails.
		 */
		void InitMeshes();

		/**
		 * @brief Initializes the Bottom-Level Acceleration Structures (BLAS) used for ray tracing.
		 * 
		 * This function builds individual BLAS objects for the quad and cube meshes.
		 * These BLAS objects are used as geometry inputs when constructing a top-level
		 * acceleration structure (TLAS) for ray tracing.
		 * 
		 * @throws std::runtime_error if either BLAS fails to build.
		 */
		void InitBLAS();

		/**
		 * @brief Initializes geometry instances that will be used to build the top-level acceleration structure (TLAS).
		 * 
		 * Allocates an upload buffer to hold an array of D3D12_RAYTRACING_INSTANCE_DESC structs, one per instance.
		 * Each descriptor defines the instance's transform, ID, mask, and reference to a BLAS.
		 * 
		 * This function also initializes instance IDs, masks, and BLAS pointers.
		 * Transform matrices are filled in by UpdateTransforms().
		 * 
		 * @throws std::runtime_error if the instance buffer cannot be created.
		 */
		void InitScene();

		/**
		 * @brief Initializes the Top-Level Acceleration Structure (TLAS) used for ray tracing.
		 * 
		 * This function builds the initial TLAS from the provided instance descriptors and allocates
		 * additional scratch memory required to perform future TLAS updates.
		 * 
		 * A special workaround is included to handle an issue in WARP where the reported update scratch
		 * size may be smaller than required.
		 * 
		 * @throws std::runtime_error if TLAS creation or scratch memory allocation fails.
		 */
		void InitTLAS();

		/**
		 * @brief Initializes the root signature for the ray tracing pipeline.
		 * 
		 * This root signature consists of:
		 *  - One UAV descriptor table (for writing outputs such as a raytraced image).
		 *  - One SRV root descriptor (for accessing the TLAS directly via a GPU virtual address).
		 * 
		 * The root signature is serialized and then used to create the runtime root signature object.
		 * 
		 * @throws std::runtime_error if serialization or creation fails.
		 */
		void InitRootSignature();

		/**
		 * @brief Initializes the ray tracing pipeline and shader table.
		 * 
		 * This function creates a D3D12 ray tracing pipeline using a shader library compiled to DXIL,
		 * defines a hit group, sets up payload and attribute size limits, and configures recursion depth.
		 * After the pipeline state object (PSO) is created, it allocates and fills a shader identifier table
		 * used to dispatch rays.
		 * 
		 * @throws std::runtime_error if any part of the pipeline or shader table fails to initialize.
		 */
		void InitPipeline();

		/**
		 * @brief Updates object transformations and rebuilds the Top-Level Acceleration Structure (TLAS) for the current frame.
		 * 
		 * This function is typically called once per frame. It first updates the instance transformation 
		 * matrices, then rebuilds the TLAS using the updated instance data. It uses the 
		 * D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGE_PREFORM_UPDATE flag to reuse the existing TLAS
		 * resource, and inserts a UAV barrier to ensure memory visibility.
		 */
		void Update();

		/**
		 * @brief Blocks the GPU until the GPU has completed all submitted work.
		 * 
		 * This functions ensures synchronization between the CPU and GPU by signaling a fence
		 * and waiting until the GPU has processed all commands up to the current fence value.
		 * 
		 * This is accomplished by calling ID3D12Fence::SetEventOnCompletion with a null handle,
		 * which causes the function to block until the GPU reaches the specified fence value.
		 * No Windows event or additional synchronization primitive is required.
		 * 
		 * This is typically used to flush the GPU pipeline or ensure that submitted GPU work
		 * is complete before proceeding with resource updates or frame teardown.
		 * 
		 * @throws std::runtime_error if signalling or waiting on the fence fails.
		 */
		void WaitForGPU();

		/**
		 * @brief Resizes the swapchain and recreates the render target UAV to match the new window size.
		 * 
		 * This function:
		 *  - Queries the client area of the window for updated width and height.
		 *  - Waits for GPU completion before resizing.
		 *  - Resizes the swapchain's internal buffers.
		 *  - Releases and recreates the render target texture.
		 *  - Rebinds the new render target to a UAV descriptor in the heap.
		 * 
		 * @param hwnd Handle to the window to query for size.
		 * @throws std::runtime_error if the render target creation fails.
		 */
		void Resize(const HWND& hwnd);

		/**
		 * @brief Builds a ray tracing acceleration structure (BLAS or TLAS) from the given input geometry.
		 *
		 * This function calculates required buffer sizes using GetRaytracingAccelerationStructurePrebuildInfo,
		 * allocates GPU buffers for the result and scratch data, builds the acceleration structure,
		 * and executes the build on the GPU using a command list.
		 * 
		 * After execution, it waits for the GPU to complete the build and returns a pointer to the 
		 * resulting acceleration structure buffer.
		 * 
		 * @param inputs Structure describing the geometry and type of acceleration structure to build.
		 * @param updateScratchSize (Optional) Out parameter that receives the scratch size needed for AS updates.
		 * @return Pointer to the created GPU resource containing the built acceleration structure.
		 * 
		 * @throws std::runtime_error if resource creation or command execution fails.
		 */
		ID3D12Resource* MakeAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs, UINT64* updateScratchSize = nullptr);
		
		/**
		 * @brief Creates a Bottom-Level Acceleration Structure (BLAS) from vertex and optional index buffers.
		 * 
		 * This function constructs a D3D12 geometry descriptor using vertex and optional index buffers,
		 * and builds a bottom-level acceleration structure from it using ray tacing APIs.
		 * 
		 * @param vertexBuffer A pointer to a GPU resource containing vertex data.
		 * @param vertexFloats The number of floats in the vertex buffer (typically vertex count * 3).
		 * @param indexBuffer (Optional) A pointer to a GPU resource containing index data.
		 * @param indices (Optional) The number of indices in the index buffer.
		 * @return A pointer to the created BLAS resource, or nullptr on failure.
		 */
		ID3D12Resource* MakeBLAS(ID3D12Resource* vertexBuffer, UINT vertexFloats, ID3D12Resource* indexBuffer = nullptr, UINT indices = 0);

		/**
		 * @brief Creates a Top-Level Acceleration Structure (TLAS) from a set of geometry instances.
		 * 
		 * The TLAS is built using an input resource containing D3D12_RAYTRACING_INSTANCE_DESC entries,
		 * which reference BLAS structures and include world transforms.
		 * 
		 * This function requests the TLAS by updateable by using the corresponding build flag,
		 * and returns the resulting acceleration structure GPU resource.
		 * 
		 * @param instances A GPU resource containing instance descriptors (in upload heap).
		 * @param numInstances The number of instances in the buffer.
		 * @param updateScratchSize (Optional) Output parameter to receive scratch size for updates.
		 * @return A pointer to the TLAS resource.
		 * 
		 * @throws std::runtime_error if the acceleration structure cannot be created.
		 */
		ID3D12Resource* MakeTLAS(ID3D12Resource* instances, UINT numInstances, UINT64* updateScratchSize = nullptr);

		/**
		 * @brief Updates the world transform matrices for each instance in the scene.
		 * 
		 * Converts world-space matrices to the required 3x4 row-major format (DirectX::XMFLOAT3X4),
		 * and writes them directly into the mapped instance descriptor buffer.
		 * 
		 * This function uses animated or static transformations to demonstrate dynamic instance movement.
		 */
		void UpdateTransforms();
	};
}