#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <d3dcompiler.h>
#include <directxtk/DDSTextureLoader.h>

#include "Structure.h"
#include "Chunk.h"
#include "DXUtils.h"
#include "Graphics.h"
#include "Utils.h"

using namespace Microsoft::WRL;
using namespace DirectX;


class DXUtils {
public:
	template <typename V>
	static bool CreateVertexBuffer(ComPtr<ID3D11Buffer>& vertexBuffer, std::vector<V>& vertices)
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth = UINT(sizeof(V) * vertices.size());
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.StructureByteStride = sizeof(V);

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = vertices.data();

		HRESULT ret = Graphics::device->CreateBuffer(&desc, &data, vertexBuffer.GetAddressOf());
		if (FAILED(ret))
			return false;

		return true;
	}


	static bool CreateIndexBuffer(ComPtr<ID3D11Buffer>& indexBuffer, std::vector<uint32_t>& indices)
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.StructureByteStride = sizeof(uint32_t);

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = indices.data();

		HRESULT ret = Graphics::device->CreateBuffer(&desc, &data, indexBuffer.GetAddressOf());
		if (FAILED(ret))
			return false;

		return true;
	}

	template <typename ConstantData>
	static bool CreateConstantBuffer(
		ComPtr<ID3D11Buffer>& constantBuffer, ConstantData& constantData)
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth = sizeof(ConstantData);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = &constantData;

		HRESULT ret = Graphics::device->CreateBuffer(&desc, &data, constantBuffer.GetAddressOf());
		if (FAILED(ret))
			return false;

		return true;
	}


	template <typename ConstantData>
	static void UpdateConstantBuffer(
		ComPtr<ID3D11Buffer>& constantBuffer, ConstantData& constantData)
	{
		D3D11_MAPPED_SUBRESOURCE ms;

		Graphics::context->Map(constantBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &constantData, sizeof(ConstantData));
		Graphics::context->Unmap(constantBuffer.Get(), NULL);
	}


	static bool CreateVertexShaderAndInputLayout(const std::wstring& filename,
		ComPtr<ID3D11VertexShader>& vs, ComPtr<ID3D11InputLayout>& il,
		std::vector<D3D11_INPUT_ELEMENT_DESC>& elementDesc)
	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ComPtr<ID3DBlob> shaderBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT ret = D3DCompileFromFile(
			filename.c_str(), 0, 0, "main", "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
		if (FAILED(ret)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (shaderBlob)
				shaderBlob->Release();

			return false;
		}

		ret = Graphics::device->CreateVertexShader(
			shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, vs.GetAddressOf());
		if (FAILED(ret))
			return false;

		ret = Graphics::device->CreateInputLayout(elementDesc.data(), (UINT)elementDesc.size(),
			shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), il.GetAddressOf());
		if (FAILED(ret))
			return false;
		return true;
	}


	static bool CreatePixelShader(const std::wstring& filename, ComPtr<ID3D11PixelShader>& ps)
	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ComPtr<ID3DBlob> shaderBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT ret = D3DCompileFromFile(
			filename.c_str(), 0, 0, "main", "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
		if (FAILED(ret)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (shaderBlob)
				shaderBlob->Release();

			return false;
		}

		ret = Graphics::device->CreatePixelShader(
			shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, ps.GetAddressOf());
		if (FAILED(ret))
			return false;

		return true;
	}


	static void UpdateViewport(
		D3D11_VIEWPORT& viewport, int topLeftX, int topLeftY, int width, int height)
	{
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.TopLeftX = (FLOAT)topLeftX;
		viewport.TopLeftY = (FLOAT)topLeftY;
		viewport.Width = (FLOAT)width;
		viewport.Height = (FLOAT)height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
	}

	static bool CreateTextureBuffer(ComPtr<ID3D11Texture2D>& buffer, UINT width, UINT height,
		bool isMSAA, DXGI_FORMAT format, UINT bindFlags)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = desc.ArraySize = 1;
		desc.Format = format;
		if (isMSAA) {
			UINT qualityLevel = 0;
			HRESULT ret = Graphics::device->CheckMultisampleQualityLevels(format, 4, &qualityLevel);
			if (FAILED(ret)) {
				std::cout << "failed check MSSA" << std::endl;
				return false;
			}

			desc.SampleDesc.Count = 4;
			desc.SampleDesc.Quality = qualityLevel - 1;
		}
		else {
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
		}

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bindFlags;
		if (bindFlags == 72)
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		HRESULT ret = Graphics::device->CreateTexture2D(&desc, nullptr, buffer.GetAddressOf());
		if (FAILED(ret))
			return false;

		return true;
	}

	static ComPtr<ID3D11Texture2D> CreateStagingTexture(UINT width, UINT height,
		std::vector<uint8_t>& image, UINT mipLevels = 1, UINT arraySize = 1,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, size_t pixelSize = sizeof(uint8_t) * 4)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mipLevels;
		desc.ArraySize = arraySize;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

		ComPtr<ID3D11Texture2D> stagingTexture;
		HRESULT ret = Graphics::device->CreateTexture2D(&desc, NULL, stagingTexture.GetAddressOf());
		if (FAILED(ret))
			return nullptr;

		D3D11_MAPPED_SUBRESOURCE ms;
		Graphics::context->Map(stagingTexture.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);
		uint8_t* pData = (uint8_t*)ms.pData;
		for (UINT h = 0; h < UINT(height); h++) { // ������ �� �پ� ����
			memcpy(
				&pData[h * ms.RowPitch], &image[(size_t)h * width * pixelSize], width * pixelSize);
		}
		Graphics::context->Unmap(stagingTexture.Get(), NULL);

		return stagingTexture;
	}

	static bool CreateTextureArrayFromAtlasFile(ComPtr<ID3D11Texture2D>& texture,
		ComPtr<ID3D11ShaderResourceView>& srv, std::string filename,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, size_t pixelSize = 4, UINT tileSizeW = 128,
		UINT tileSizeH = 128, UINT tileCountW = 16, UINT tileCountH = 16)
	{
		// Read Atlas image
		int width, height, channel = 4;
		std::vector<uint8_t> imageData;
		Utils::ReadImage(filename, imageData, width, height);


		// Convert image data to tile image data array
		std::vector<std::vector<uint8_t>> imageArray;

		UINT rowPitch = tileSizeW * channel * tileCountW;
		UINT sliceRowPitch = tileSizeH * rowPitch;

		for (UINT y = 0; y < tileCountH; ++y) {
			UINT sliceHeight = sliceRowPitch * y;
			for (UINT x = 0; x < tileCountW; ++x) {
				UINT sliceWidth = tileSizeW * channel * x;

				std::vector<uint8_t> tileData;
				for (UINT h = 0; h < tileSizeH; ++h) {
					for (UINT w = 0; w < tileSizeW; ++w) {
						tileData.push_back(
							imageData[sliceHeight + sliceWidth + h * rowPitch + channel * w]);
						tileData.push_back(
							imageData[sliceHeight + sliceWidth + h * rowPitch + channel * w + 1]);
						tileData.push_back(
							imageData[sliceHeight + sliceWidth + h * rowPitch + channel * w + 2]);
						tileData.push_back(
							imageData[sliceHeight + sliceWidth + h * rowPitch + channel * w + 3]);
					}
				}

				imageArray.push_back(tileData);
			}
		}


		// Create TextureArray without initData
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = tileSizeW;
		desc.Height = tileSizeH;
		desc.MipLevels = 0;
		desc.ArraySize = tileCountW * tileCountH;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT; // ������¡ �ؽ���κ��� ����
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // �Ӹ� ���
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;						// �Ӹ� ���

		HRESULT ret = Graphics::device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
		if (FAILED(ret)) {
			return false;
		}
		texture->GetDesc(&desc);

		// Create StagingTexture, Copy to Origin Texture
		for (UINT i = 0; i < desc.ArraySize; ++i) {
			auto& image = imageArray[i];

			ComPtr<ID3D11Texture2D> tempStagingTexture =
				CreateStagingTexture(tileSizeW, tileSizeH, image, 1, 1, format, pixelSize);
			if (!tempStagingTexture)
				return false;

			UINT subresourceIndex =
				D3D11CalcSubresource(0, i, desc.MipLevels); // MipSlice + ArraySlice * MipLevels

			Graphics::context->CopySubresourceRegion(
				texture.Get(), subresourceIndex, 0, 0, 0, tempStagingTexture.Get(), 0, nullptr);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = desc.ArraySize;

		// Generate Mips
		ret =
			Graphics::device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
		if (FAILED(ret))
			return false;

		Graphics::context->GenerateMips(srv.Get());

		return true;
	}

	static bool CreateTexture2DFromFile(ComPtr<ID3D11Texture2D>& texture,
		ComPtr<ID3D11ShaderResourceView>& srv, std::string filename,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, size_t pixelSize = 4)
	{
		int width, height, channel = 4;
		std::vector<uint8_t> image;
		Utils::ReadImage(filename, image, width, height);

		ComPtr<ID3D11Texture2D> stagingTexture =
			CreateStagingTexture(width, height, image, 0, 1, format, pixelSize);

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 0; // �Ӹ� ���� �ִ�
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT; // ������¡ �ؽ���κ��� ���� ����
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // �Ӹ� ���
		desc.CPUAccessFlags = 0;

		HRESULT ret = Graphics::device->CreateTexture2D(&desc, NULL, texture.GetAddressOf());
		if (FAILED(ret))
			return false;

		Graphics::context->CopySubresourceRegion(
			texture.Get(), 0, 0, 0, 0, stagingTexture.Get(), 0, NULL);

		ret = Graphics::device->CreateShaderResourceView(texture.Get(), 0, srv.GetAddressOf());
		if (FAILED(ret))
			return false;

		Graphics::context->GenerateMips(srv.Get());

		return true;
	}

	static bool CreateDDSTextureFromFile(
		ComPtr<ID3D11ShaderResourceView>& srv, const std::wstring& filename, bool isCubemap)
	{
		ComPtr<ID3D11Texture2D> texture;

		UINT miscFlags = 0;
		if (isCubemap) {
			miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		HRESULT ret = CreateDDSTextureFromFileEx(Graphics::device.Get(), filename.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, miscFlags,
			DDS_LOADER_FLAGS(DDS_LOADER_DEFAULT), (ID3D11Resource**)texture.GetAddressOf(),
			srv.GetAddressOf(), nullptr);
		if (FAILED(ret))
			return false;

		return true;
	}
};
