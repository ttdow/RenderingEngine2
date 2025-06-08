#pragma once

#include <cstdint>
#include <d3d12.h>

namespace Engine
{
	class DX12Descriptor
	{
	public:

		ID3D12DescriptorHeap* heap = nullptr;

		DX12Descriptor(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible);
		~DX12Descriptor();

	private:


	};
}