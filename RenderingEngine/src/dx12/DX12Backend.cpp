#include "pch.h"

#include "DX12Backend.h"

#pragma comment(lib, "user32") // Windows functions like DefWindowProcW, PostQuitMessage, etc.
#pragma comment(lib, "d3d12")  // Direct3D 12 graphics API.
#pragma comment(lib, "dxgi")   // DirectX Graphics Infrastructure, for handling adapters/swap chains.

namespace Engine
{
	constexpr DXGI_SAMPLE_DESC NO_AA = { .Count = 1, .Quality = 0 };

	constexpr D3D12_RESOURCE_DESC BASIC_BUFFER_DESC =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = 0, // Will be changed in copies.
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.SampleDesc = NO_AA,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
	};

	constexpr D3D12_HEAP_PROPERTIES UPLOAD_HEAP = { .Type = D3D12_HEAP_TYPE_UPLOAD };

	constexpr D3D12_HEAP_PROPERTIES DEFAULT_HEAP = { .Type = D3D12_HEAP_TYPE_DEFAULT };

	DX12Backend::DX12Backend(HINSTANCE hInstance, const HWND& hwnd)
	{
		value = 1;

		InitDevice();
		InitSurfaces(hwnd);
		InitCommand();
		InitMeshes();
		InitBLAS();
		InitScene();
		InitTLAS();
		InitRootSignature();
		InitPipeline();
	}

	DX12Backend::~DX12Backend()
	{

		WaitForGPU();

#if defined _DEBUG

		HRESULT hr = debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
		if (FAILED(hr))
		{
			std::cout << "ReportLiveDeviceObjects failed.\n";
		}
		else if (SUCCEEDED(hr))
		{
			std::cout << "ReportLiveDeviceObjects succeeded.\n";
		}
#endif

	}

	void DX12Backend::Draw()
	{
		// Reset command allocator (reclaims memory).
		HRESULT hr = cmdAlloc->Reset();
		if (FAILED(hr))
		{
			throw std::runtime_error("Command Allocator could not be reset!");
		}

		// Reset a command list and rebind to the allocator.
		hr = cmdList->Reset(cmdAlloc, nullptr);
		if (FAILED(hr))
		{
			throw std::runtime_error("Command List could not be reset!");
		}

		// Update the scene transforms and TLAS in-place.
		Update();

		// Bind the ray tracing pipeline.
		cmdList->SetPipelineState1(pso);

		// Bind the global root signature.
		cmdList->SetComputeRootSignature(rootSignature);

		// Bind descriptor heaps (UAV table required for output target).
		cmdList->SetDescriptorHeaps(1, &uavHeap);
		D3D12_GPU_DESCRIPTOR_HANDLE uavTable = uavHeap->GetGPUDescriptorHandleForHeapStart();
		cmdList->SetComputeRootDescriptorTable(0, uavTable);						// Bind UAV descriptor table.
		cmdList->SetComputeRootShaderResourceView(1, tlas->GetGPUVirtualAddress()); // Bind TLAS directly by GPU virtual address.

		// Get render target dimensions for dispatch size.
		D3D12_RESOURCE_DESC rtDesc = renderTarget->GetDesc();

		// Describe the ray tracing shader table layout.
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};

		D3D12_GPU_VIRTUAL_ADDRESS_RANGE rayGen = {};
		rayGen.StartAddress = shaderIDs->GetGPUVirtualAddress();
		rayGen.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		dispatchDesc.RayGenerationShaderRecord = rayGen;

		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE rayMiss = {};
		rayMiss.StartAddress = shaderIDs->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		rayMiss.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		dispatchDesc.MissShaderTable = rayMiss;

		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE rayHit = {};
		rayHit.StartAddress = shaderIDs->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		rayHit.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		dispatchDesc.HitGroupTable = rayHit;

		dispatchDesc.Width = static_cast<uint32_t>(rtDesc.Width);
		dispatchDesc.Height = rtDesc.Height;
		dispatchDesc.Depth = 1;

		// Issue the ray tracing dispatch call.
		cmdList->DispatchRays(&dispatchDesc);

		// Transition and copy render target to the swapchain's back buffer.
		ID3D12Resource* backBuffer;
		hr = swapchain->GetBuffer(swapchain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to get swapchain back buffer!");
		}

		// Helper lambda for resource state transitions.
		auto Barrier = [&](auto* resource, auto before, auto after)
			{
				D3D12_RESOURCE_TRANSITION_BARRIER transition = {};
				transition.pResource = resource;
				transition.StateBefore = before;
				transition.StateAfter = after;

				D3D12_RESOURCE_BARRIER rb = {};
				rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				rb.Transition = transition;

				cmdList->ResourceBarrier(1, &rb);
		};

		// Transition render target for copy-out (renderTarget -> backBuffer).
		Barrier(renderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		Barrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);

		// Copy final ray tracing result on the screen.
		cmdList->CopyResource(backBuffer, renderTarget);

		// Restore resource states.
		Barrier(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
		Barrier(renderTarget, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	
		// Done with back buffer references.
		backBuffer->Release();

		// Finalize the command list and submit it to the GPU.
		cmdList->Close();
		cmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&cmdList));

		// Wait for GPU to finish the frame (synchronous frame pacing).
		WaitForGPU();

		// Present the result to the screen.
		swapchain->Present(1, 0);
	}

	void DX12Backend::InitDevice()
	{
		HRESULT hr;
		uint32_t dxgiFactoryFlags = 0;

#if defined _DEBUG

		// Create a debug controller to track errors.
		ID3D12Debug* debug;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to get debug interface!");
		}

		// Get ID3D12Debug1 for GPU-side validation control.
		hr = debug->QueryInterface(IID_PPV_ARGS(&debugController));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to get debug1 interface!");
		}

		// Enable debug validation layers.
		debugController->EnableDebugLayer();				// Enable CPU-side validation.
		debugController->SetEnableGPUBasedValidation(true); // Enable GPU-side validation.

		// Add debug flag for factory.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; // Tell DXGI to enable debugging.

		debug->Release();
		debug = nullptr;

#endif

		// Create the DXGI factory for adapter enumeration and swapchain management.
		hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create DXGI Factory!");
		}

		// Enumerate adapters to list available devices (software and hardware).
		IDXGIAdapter1* adapter = nullptr;
		uint32_t warpIndex = 0;
		uint32_t adapterIndex = 0;

		while (factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc = {};
			adapter->GetDesc1(&desc);

			// Convert wide string to narrow for output.
			std::string name = "";
			for (uint32_t i = 0; i < 128; i++)
			{
				char c = static_cast<char>(desc.Description[i]);
				name += c;

				if (c == '\0')
				{
					break;
				}
			}

			std::cout << "Adapter " << adapterIndex << ":\n";
			std::cout << "  Name: " << name << "\n";
			std::cout << "  Dedicated video memory: " << (desc.DedicatedVideoMemory / (1024 * 1024)) << "\n";
			std::cout << "  Flags: 0x" << std::hex << desc.Flags << std::dec << "\n";
			std::cout << "  Is software adapter: " << ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) ? "Yes" : "No") << "\n\n";

			// Save WARP adapter index once found for use later.
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				warpIndex = adapterIndex;
			}

			adapter->Release();
			adapterIndex++;
		}

		// TODO: Use the WARP adapter for now.
		hr = factory->EnumAdapters1(warpIndex, &adapter);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to enumerate WARP adapter!");
		}

		// Create the D3D12 device with a required feature level.
		D3D_FEATURE_LEVEL levelsToCheck[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0
		};

		D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels = {};
		featureLevels.NumFeatureLevels = _countof(levelsToCheck);
		featureLevels.pFeatureLevelsRequested = levelsToCheck;

		// Attempt to create the device at 12.1 (WARP supports this).
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create D3D12 device using WARP adapter with feature level 12.1!");
		}

		adapter->Release();

		// Query and print the highest feature level supported by the adapter.
		hr = device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels));
		if (SUCCEEDED(hr))
		{
			std::cout << "Highest supported feature level: " << featureLevels.MaxSupportedFeatureLevel << "\n";
		}

		// TODO: Is there a point to this?
		device->SetName(L"Hello Triangle Device");

#if defined _DEBUG

		// Query for ID3D12DebugDevice for detailed GPU diagnostics in debug builds.
		device->QueryInterface(&debugDevice);

#endif

		// Create the primary command queue for submitting work to the GPU.
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Supports draw, dispatch, copy.

		hr = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create D3D12 command queue!");
		}

		// Create a CPU-GPU fence for synchronization.
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create D3D12 synchronization fence!");
		}
	}

	void DX12Backend::InitSurfaces(const HWND& hwnd)
	{
		// Describe a double-buffered swapchain for the window.
		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		   // RGBA 8-bit format.
		scDesc.SampleDesc = NO_AA;						   // No multisampling.
		scDesc.BufferCount = 2;							   // Double buffering.
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Recommended for modern presentation.

		// Create the swapchain using DXGI 1.1 interface.
		IDXGISwapChain1* swapchain1;
		HRESULT hr = factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &scDesc, nullptr, nullptr, &swapchain1);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create swapchain!");
		}

		// Upgrade the swapchain to version 3 for advanced usage (e.g., frame indexing).
		swapchain1->QueryInterface(&swapchain);
		swapchain1->Release();
		
		// DXGI factory is no longer needed after swapchain creation.
		factory->Release();

		// TODO: Add a new descriptor for texture.
		
		// Create a GPU-visible descriptor heap to hold SRVs/UAVs/CBVs
		D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
		uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		uavHeapDesc.NumDescriptors = 2;								   // One for output UAV, one spare.
		uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Needed for shader access.

		hr = device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&uavHeap));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create descriptor heap!");
		}

		// Initialize render target with the current window size.
		Resize(hwnd);
	}

	void DX12Backend::InitCommand()
	{
		// A command allocator is a chunk of memory used to store the actual command data 
		// recorded by a command list.
		HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create command allocator!");
		}

		// A command list is where you actually record the GPU commands.
		hr = device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmdList));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create command list!");
		}
	}

	void DX12Backend::InitMeshes()
	{
		// Helper lambda to create a buffer, upload data to it, and return the resource pointer.
		auto MakeAndCopy = [&](auto& data) 
		{
			// Copy base buffer description and set buffer size to match the data.
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			desc.Width = sizeof(data);
			desc.Height = 1;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1;
			desc.SampleDesc = NO_AA;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			ID3D12Resource* res;

			// Create a committed resource in the UPLOAD heap so we can map and copy data.
			HRESULT hr = device->CreateCommittedResource(
				&UPLOAD_HEAP,				 // Heap type = CPU-writeable memory.
				D3D12_HEAP_FLAG_NONE,		 // No special heap flags.
				&desc,						 // Resource description (buffer size and layout).
				D3D12_RESOURCE_STATE_COMMON, // Initial resource state (not used for upload).
				nullptr,					 // No optimized clear value.
				IID_PPV_ARGS(&res)			 // Output resource pointer.
			);

			if (FAILED(hr))
			{
				throw std::runtime_error("Failed to create mesh buffer!");
			}

			// Map the buffer into CPU-visible memory.
			void* ptr;
			res->Map(0, nullptr, &ptr);

			// Copy the mesh data into the GPU buffer.
			memcpy(ptr, data, sizeof(data));

			// Unmap the buffer (flushes changes, readies GPU access).
			res->Unmap(0, nullptr);

			// Return the GPU resource.
			return res;
		};

		// Create and upload mesh data for various objects.
		quadVB = MakeAndCopy(quadVtx); // Vertex buffer for quad.
		cubeVB = MakeAndCopy(cubeVtx); // Vertex buffer for cube.
		cubeIB = MakeAndCopy(cubeIdx); // Index buffer for cube.
	}

	void DX12Backend::InitBLAS()
	{
		// Build a BLAS from the quad's vertex buffer only (no index buffer).
		quadBlas = MakeBLAS(quadVB, std::size(quadVtx));
		if (!quadBlas)
		{
			throw std::runtime_error("Failed to create quad BLAS!");
		}

		// Build a BLAS from the cube's vertex and index buffers.
		cubeBlas = MakeBLAS(cubeVB, std::size(cubeVtx), cubeIB, std::size(cubeIdx));
		if (!cubeBlas)
		{
			throw std::runtime_error("Failed to create cube BLAS!");
		}
	}

	void DX12Backend::InitScene()
	{
		// Define the instance descriptor buffer's size.
		auto instancesDesc = BASIC_BUFFER_DESC;
		instancesDesc.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * NUM_INSTANCES;

		// Create the upload heap buffer to hold instance descriptors (CPU writable).
		HRESULT hr = device->CreateCommittedResource(
			&UPLOAD_HEAP, 
			D3D12_HEAP_FLAG_NONE, 
			&instancesDesc, 
			D3D12_RESOURCE_STATE_COMMON, 
			nullptr, 
			IID_PPV_ARGS(&instances)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create geometry instances!");
		}

		// Map the buffer to get a CPU pointer to the instance descriptor array.
		instances->Map(0, nullptr, reinterpret_cast<void**>(&instanceData));

		// Fill in each instance with its ID, visibility mask, and BLAS pointer.
		for (UINT i = 0; i < NUM_INSTANCES; i++)
		{
			instanceData[i].InstanceID = i;
			instanceData[i].InstanceMask = 1;
			instanceData[i].AccelerationStructure = (i ? quadBlas : cubeBlas)->GetGPUVirtualAddress();
		}

		// Set initial transform matrices for each instance.
		UpdateTransforms();
	}

	void DX12Backend::InitTLAS()
	{
		UINT64 updateScratchSize;

		// Build the TLAS from the mapped instance descriptors.
		tlas = MakeTLAS(instances, NUM_INSTANCES, &updateScratchSize);
		if (!tlas)
		{
			throw std::runtime_error("Failed to make TLAS!");
		}

		// Create a scratch buffer that can be reused for future TLAS updates.
		auto desc = BASIC_BUFFER_DESC;

		// WARP bug workaround: use 8 if the required size was reported as less.
		desc.Width = std::max(updateScratchSize, 8ULL);
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		// Allocate the scratch buffer in GPU-local memory.
		HRESULT hr = device->CreateCommittedResource(
			&DEFAULT_HEAP, 
			D3D12_HEAP_FLAG_NONE, 
			&desc, 
			D3D12_RESOURCE_STATE_COMMON, 
			nullptr, 
			IID_PPV_ARGS(&tlasUpdateScratch)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create TLAS scratch memory!");
		}
	}

	void DX12Backend::InitRootSignature()
	{
		// Define a descriptor range for UAV (Unordered Access View).
		D3D12_DESCRIPTOR_RANGE uavRange = {};
		uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		uavRange.NumDescriptors = 1;	 // Just 1 UAV bound (e.g., output texture).
		uavRange.BaseShaderRegister = 0; // Corresponds to u0 in HLSL.
		uavRange.RegisterSpace = 0;		 // Default register space.
		uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Container for root parameters (UAV table and TLAS SRV).
		std::vector<D3D12_ROOT_PARAMETER> params;
		params.reserve(2);

		// Describe the root parameter for the UAV descriptor table.
		D3D12_ROOT_DESCRIPTOR_TABLE descTable = {};
		descTable.NumDescriptorRanges = 1;
		descTable.pDescriptorRanges = &uavRange;

		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // Make visible to all shader stages.
		param.DescriptorTable = descTable;
		params.push_back(param);

		// Describe the root parameter for the TLAS SRV (bind via GPU virtual address).
		D3D12_ROOT_DESCRIPTOR rootDesc = {};
		rootDesc.ShaderRegister = 0; // Corresponds to t0 (SRV register 0).
		rootDesc.RegisterSpace = 0;

		param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		param.Descriptor = rootDesc;
		params.push_back(param);

		// Create the root signature description.
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = static_cast<UINT>(params.size());
		desc.pParameters = params.data();
		desc.NumStaticSamplers = 0; // No samplers bound.
		desc.pStaticSamplers = nullptr;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		// Serialize the root signature to a blob.
		ID3DBlob* blob;
		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to serialize root signature!");
		}

		// Create the root signature object from the serialized blob.
		hr = device->CreateRootSignature(
			0, 
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create root signature!");
		}

		// Release the serialization blob.
		blob->Release();
	}

	void DX12Backend::InitPipeline()
	{
		// Load compiled shader bytecode (DXIL library).
		D3D12_SHADER_BYTECODE bytecode = {};
		bytecode.BytecodeLength = std::size(compiledShader);
		bytecode.pShaderBytecode = compiledShader;

		D3D12_DXIL_LIBRARY_DESC lib = {};
		lib.DXILLibrary = bytecode;
		lib.NumExports = 0; // Export all shaders in the library implicitly.
		lib.pExports = nullptr;

		// Defines a hit group: bundles closest hit, any hit, and intersection shaders.
		D3D12_HIT_GROUP_DESC hitGroup = {};
		hitGroup.HitGroupExport = L"HitGroup";			 // Name used in shader table.
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hitGroup.ClosestHitShaderImport = L"ClosestHit"; // Name of closest hit function in the DXIL library.
		hitGroup.AnyHitShaderImport = NULL;				 // Unused.
		hitGroup.IntersectionShaderImport = NULL;		 // Unused.

		// Define ray payload size (data passed between shaders) and attribute size (e.g., barycentrics).
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
		shaderConfig.MaxPayloadSizeInBytes = 20;  // Float3, bool, bool 
		shaderConfig.MaxAttributeSizeInBytes = 8; // Barycentric coords (float2).

		// Set the global root signature (used by all shaders in the pipeline).
		D3D12_GLOBAL_ROOT_SIGNATURE globalSig = {};
		globalSig.pGlobalRootSignature = rootSignature;

		// Configure the max trace recursion depth (how many levels of ray tracing allowed).
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = 3;

		// Create a list of subobjects to define the pipeline.
		D3D12_STATE_SUBOBJECT subobjects[] =
		{
			{.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &lib},
			{.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &hitGroup},
			{.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &shaderConfig},
			{.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &globalSig},
			{.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &pipelineConfig}
		};

		// Describe the full pipeline object.
		D3D12_STATE_OBJECT_DESC desc = {};
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		desc.NumSubobjects = std::size(subobjects);
		desc.pSubobjects = subobjects;

		// Create the PSO (Pipeline State Object).
		HRESULT hr = device->CreateStateObject(&desc, IID_PPV_ARGS(&pso));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create pipeline state object!");
		}

		// Allocate a small upload heap to store shader identifiers.
		auto idDesc = BASIC_BUFFER_DESC;
		idDesc.Width = NUM_SHADER_IDS * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // 64-byte alignment.

		hr = device->CreateCommittedResource(
			&UPLOAD_HEAP, 
			D3D12_HEAP_FLAG_NONE, 
			&idDesc, 
			D3D12_RESOURCE_STATE_COMMON, 
			nullptr, 
			IID_PPV_ARGS(&shaderIDs)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create shader IDs!");
		}

		// Query pipeline for shader identifier mapping.
		ID3D12StateObjectProperties* props;
		pso->QueryInterface(&props);

		// Get a mapped pointer to write shader identifiers.
		void* data;
		auto WriteId = [&](const wchar_t* name) 
		{
			void* id = props->GetShaderIdentifier(name); // Get 32-byte shader identifier.
			memcpy(data, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			data = static_cast<char*>(data) + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		};

		// Map the shader ID buffer and populate with identifiers for shaders.
		shaderIDs->Map(0, nullptr, &data);
		WriteId(L"RayGeneration"); // t0, u0 bindings assumed in HLSL.
		WriteId(L"Miss");
		WriteId(L"HitGroup");	   // Composite name for the hit group.
		shaderIDs->Unmap(0, nullptr);

		props->Release();
	}

	void DX12Backend::Update()
	{
		// Recalculate instance transformations based on time (animate the scene).
		UpdateTransforms();

		// Prepare inputs for updating the TLAS using the new transforms.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE; // Reuse TLAS memory.
		inputs.NumDescs = NUM_INSTANCES;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.InstanceDescs = instances->GetGPUVirtualAddress(); // Points to the updated instance data.

		// Prepare the full build description for the GPU command.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
		desc.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();
		desc.Inputs = inputs;
		desc.SourceAccelerationStructureData = tlas->GetGPUVirtualAddress();
		desc.ScratchAccelerationStructureData = tlasUpdateScratch->GetGPUVirtualAddress();

		// Issue TLAS update command to the command list.
		cmdList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

		// Insert a UAV barrier to ensure the TLAS is visible to subsequent shaders.
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV = { .pResource = tlas };

		cmdList->ResourceBarrier(1, &barrier);
	}

	void DX12Backend::WaitForGPU()
	{
		// Signal the fence with the current value to mark the GPU's progress.
		HRESULT hr = cmdQueue->Signal(fence, value);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to signal synchronization fence!");
		}

		// SetEventOnCompletion with nullptr blocks this call on the CPU until
		// the GPU reaches the specified fence value.
		hr = fence->SetEventOnCompletion(value++, nullptr);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to update synchronization fence!");
		}
	}

	void DX12Backend::Resize(const HWND& hwnd)
	{
		if (!swapchain) [[unlikely]]
		{
			return;
		}

		// Query the window's current client area dimensions.
		RECT rect;
		GetClientRect(hwnd, &rect);
		auto width = std::max<UINT>(rect.right - rect.left, 1);
		auto height = std::max<UINT>(rect.bottom - rect.top, 1);

		// Wait until the GPU is done with current frame before resizing.
		WaitForGPU();

		// Resize swapchain buffers; width/height == 0 means use passed dimensions.
		swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

		// Release the old render target, if one exists.
		if (renderTarget) [[likely]]
		{
			renderTarget->Release();
		}

		// Describe the new render target texture to match the new resolution.
		D3D12_RESOURCE_DESC rtDesc = {};
		rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rtDesc.Width = width;
		rtDesc.Height = height;
		rtDesc.DepthOrArraySize = 1;
		rtDesc.MipLevels = 1;
		rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtDesc.SampleDesc = NO_AA;
		rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Needed for UAV access.

		// Create the GPU resource (backed in the default heap).
		HRESULT hr = device->CreateCommittedResource(
			&DEFAULT_HEAP, 
			D3D12_HEAP_FLAG_NONE, 
			&rtDesc, 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
			nullptr, 
			IID_PPV_ARGS(&renderTarget)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create render target!");
		}

		// Describe the UAV view for this render target.
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Bind UAV to the descriptor heap at slot 0.
		device->CreateUnorderedAccessView(
			renderTarget,								  // The actual resource (e.g., a texture on the GPU).
			nullptr,									  // No counter resource (used only for structured buffers).
			&uavDesc,									  // Descriptor for the UAV.
			uavHeap->GetCPUDescriptorHandleForHeapStart() // Where to place the descriptor.
		);
	}

	ID3D12Resource* DX12Backend::MakeAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs, UINT64* updateScratchSize)
	{
		// Helper lambda to create a default heap buffer of given size and state.
		auto MakeBuffer = [&](UINT64 size, auto initialState) 
		{
			D3D12_RESOURCE_DESC desc = BASIC_BUFFER_DESC;
			desc.Width = size;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Req'd for AS buffers.
			
			ID3D12Resource* buffer;

			// Allocate committed GPU memory for AS.
			HRESULT result = device->CreateCommittedResource(
				&DEFAULT_HEAP, 
				D3D12_HEAP_FLAG_NONE, 
				&desc, 
				initialState, 
				nullptr, 
				IID_PPV_ARGS(&buffer)
			);

			if (FAILED(result))
			{
				throw std::runtime_error("Failed to create acceleration structure buffer!");
			}

			return buffer;
		};

		// Query required buffer sizes (scratch + result) for the AS build.
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
		device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

		// Optionally store the scratch size needed for AS updates (if a pointer is provided).
		if (updateScratchSize)
		{
			*updateScratchSize = prebuildInfo.UpdateScratchDataSizeInBytes;
		}
		
		// Create the scratch buffer (temporary memory used during AS build process).
		auto* scratch = MakeBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_COMMON);

		// Create the actual AS result buffer (stores the final acceleration structure).
		auto* as = MakeBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

		// Fill in build description structure for the GPU.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
		buildDesc.DestAccelerationStructureData = as->GetGPUVirtualAddress();		  // Target GPU memory.
		buildDesc.Inputs = inputs;													  // Geometry info.
		buildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress(); // Temp build memory.

		// Reset and record GPU commands to build the acceleration structure.
		cmdAlloc->Reset();
		cmdList->Reset(cmdAlloc, nullptr);
		cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
		cmdList->Close();

		// Submit the command list to the GPU for execution.
		cmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&cmdList));

		// Wait until the GPU has finished building the structure.
		WaitForGPU();

		// Clean up temorary scratch buffer resource.
		scratch->Release();

		// Return the finalized acceleration structure resource.
		return as;
	}

	ID3D12Resource* DX12Backend::MakeBLAS(ID3D12Resource* vertexBuffer, UINT vertexFloats, ID3D12Resource* indexBuffer, UINT indices)
	{
		// Describe the vertex buffer layout for the triangle geometry.
		D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE vertexBufferDesc = {};
		vertexBufferDesc.StartAddress = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferDesc.StrideInBytes = sizeof(float) * 3;

		// Set up triangle-specific geometry description.
		D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC trianglesDesc = {};
		trianglesDesc.Transform3x4 = 0; // No additional transform.
		trianglesDesc.IndexFormat = indexBuffer ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_UNKNOWN;
		trianglesDesc.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		trianglesDesc.IndexCount = indices;
		trianglesDesc.VertexCount = vertexFloats / 3;
		trianglesDesc.IndexBuffer = indexBuffer ? indexBuffer->GetGPUVirtualAddress() : 0;
		trianglesDesc.VertexBuffer = vertexBufferDesc;

		// Wrap the triangle description in a geometry description.
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometryDesc.Triangles = trianglesDesc;
		
		// Specify that we're building a BLAS with a single geometry.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = 1;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.pGeometryDescs = &geometryDesc;

		// Delegate to a helper function to create the actual acceleration structure.
		ID3D12Resource* blas = MakeAccelerationStructure(inputs);
		if (!blas)
		{
			throw std::runtime_error("Failed to create bottom-level acceleration structure!");
		}

		return blas;
	}

	ID3D12Resource* DX12Backend::MakeTLAS(ID3D12Resource* instances, UINT numInstances, UINT64* updateScratchSize)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		inputs.NumDescs = numInstances;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY; // Instance descriptors are in a flat array.
		inputs.InstanceDescs = instances->GetGPUVirtualAddress(); // Pointer to the instance descriptor buffer.

		// Use the same general AS build function used for BLAS to build the TLAS.
		ID3D12Resource* tlas = MakeAccelerationStructure(inputs, updateScratchSize);
		if (!tlas)
		{
			throw std::runtime_error("Failed to create the top-level acceleration structure!");
		}

		return tlas;
	}

	void DX12Backend::UpdateTransforms()
	{
		// Lambda to write a DirectX::XMMATRIX into a D3D12-compatible 3x4 float array.
		auto Set = [&](int idx, DirectX::XMMATRIX mx) 
		{
			auto* ptr = reinterpret_cast<DirectX::XMFLOAT3X4*>(&instanceData[idx].Transform);
			DirectX::XMStoreFloat3x4(ptr, mx);
		};

		auto time = static_cast<float>(GetTickCount64()) / 1000;

		// Instance 0: Rotating cube.
		auto cube = DirectX::XMMatrixRotationRollPitchYaw(time / 2, time / 3, time / 5);
		cube *= DirectX::XMMatrixTranslation(-1.5, 2, 2);
		Set(0, cube);

		// Instance 1: "Mirror" object with rotation animation.
		auto mirror = DirectX::XMMatrixRotationX(-1.8f);
		mirror *= DirectX::XMMatrixRotationY(DirectX::XMScalarSinEst(time) / 8 + 1);
		mirror *= DirectX::XMMatrixTranslation(2, 2, 2);
		Set(1, mirror);

		// Instance 2: Large floor plane, statically scaled.
		auto floor = DirectX::XMMatrixScaling(5, 5, 5);
		floor *= DirectX::XMMatrixTranslation(0, 0, 2);
		Set(2, floor);
	}
}