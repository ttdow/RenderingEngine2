#include "pch.h"

#include "DX12DescriptorHeap.h"

namespace Engine
{
	DX12DescriptorHeap::DX12DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = 1;
		desc.Flags = (shaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(&heap)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create descriptor heap!");
		}

		D3D12_HEAP_PROPERTIES props = {};
		props.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = 1; // Size in bytes.
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ID3D12Resource* uploadBuffer = nullptr;
		device->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)
		);


	}

	DX12DescriptorHeap::~DX12DescriptorHeap()
	{
		if (heap != nullptr)
		{
			heap->Release();
			heap = nullptr;
		}
	}
}