#pragma once

namespace Engine
{
	class DX12DescriptorHeap
	{
	public:

		ID3D12DescriptorHeap* heap = nullptr;

		DX12DescriptorHeap() = delete;
		DX12DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisisble);
		~DX12DescriptorHeap();

	private:

	};
}