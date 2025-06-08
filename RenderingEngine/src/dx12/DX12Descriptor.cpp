#include "pch.h"

#include "DX12Descriptor.h"

namespace Engine
{
	DX12Descriptor::DX12Descriptor(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible)
	{
		// Create a GPU-visible descriptor heap to hold SRVs/UAVs/CBVs.
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = numDescriptors;					
		desc.Flags = (shaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = device->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(&heap)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create descriptor heap!");
		}
	}

	DX12Descriptor::~DX12Descriptor()
	{
		if (heap)
		{
			heap->Release();
			heap = nullptr;
		}
	}
}