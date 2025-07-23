#include "main.h"

void Begin(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	if (Blur::pBlurShaderX == nullptr || Blur::pBlurShaderY == nullptr)
	{
		pSAMP->getChat()->m_pD3DDevice->CreatePixelShader(reinterpret_cast<const DWORD*>(byteBlurX), &Blur::pBlurShaderX);
		pSAMP->getChat()->m_pD3DDevice->CreatePixelShader(reinterpret_cast<const DWORD*>(byteBlurY), &Blur::pBlurShaderY);
	}

	if (Blur::pTextureX == nullptr || Blur::pTextureY == nullptr)
	{
		pSAMP->getChat()->m_pD3DDevice->CreateTexture((int)ImGui::GetIO().DisplaySize.x / Blur::iDownSample, (int)ImGui::GetIO().DisplaySize.y / Blur::iDownSample, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &Blur::pTextureX, nullptr);
		pSAMP->getChat()->m_pD3DDevice->CreateTexture((int)ImGui::GetIO().DisplaySize.x / Blur::iDownSample, (int)ImGui::GetIO().DisplaySize.y / Blur::iDownSample, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &Blur::pTextureY, nullptr);
	}

	pSAMP->getChat()->m_pD3DDevice->GetRenderTarget(0, &Blur::pBackupSurface);

	IDirect3DSurface9* pBackBuffer;
	pSAMP->getChat()->m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

	IDirect3DSurface9* pSurface;
	Blur::pTextureX->GetSurfaceLevel(0, &pSurface);
	pSAMP->getChat()->m_pD3DDevice->StretchRect(pBackBuffer, nullptr, pSurface, nullptr, D3DTEXF_LINEAR);
	pBackBuffer->Release();
	pSurface->Release();

	pSAMP->getChat()->m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pSAMP->getChat()->m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	pSAMP->getChat()->m_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	const D3DMATRIX mProjection{ {{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (ImGui::GetIO().DisplaySize.x / Blur::iDownSample), 1.0f / (ImGui::GetIO().DisplaySize.y / Blur::iDownSample), 0.0f, 1.0f }} };
	pSAMP->getChat()->m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &mProjection);
}

void FirstPass(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	pSAMP->getChat()->m_pD3DDevice->SetPixelShader(Blur::pBlurShaderX);
	const float fParams[4] = { 1.0f / (ImGui::GetIO().DisplaySize.x / Blur::iDownSample) };
	pSAMP->getChat()->m_pD3DDevice->SetPixelShaderConstantF(0, fParams, 1);

	IDirect3DSurface9* pSurface;
	Blur::pTextureY->GetSurfaceLevel(0, &pSurface);
	pSAMP->getChat()->m_pD3DDevice->SetRenderTarget(0, pSurface);
	pSurface->Release();
}

void SecondPass(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	pSAMP->getChat()->m_pD3DDevice->SetPixelShader(Blur::pBlurShaderY);
	const float fParams[4] = { 1.0f / (ImGui::GetIO().DisplaySize.y / Blur::iDownSample) };
	pSAMP->getChat()->m_pD3DDevice->SetPixelShaderConstantF(0, fParams, 1);

	IDirect3DSurface9* pSurface;
	Blur::pTextureX->GetSurfaceLevel(0, &pSurface);
	pSAMP->getChat()->m_pD3DDevice->SetRenderTarget(0, pSurface);
	pSurface->Release();
}

void End(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	pSAMP->getChat()->m_pD3DDevice->SetRenderTarget(0, Blur::pBackupSurface);
	Blur::pBackupSurface->Release();

	pSAMP->getChat()->m_pD3DDevice->SetPixelShader(nullptr);
	pSAMP->getChat()->m_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
}

void Blur::AddBlur(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, float fRounding, ImDrawFlags DrawFlags, ImColor Color)
{
	pDrawList->AddCallback(&Begin, nullptr);
	for (int i = 0; i < 8; i++)
	{
		pDrawList->AddCallback(&FirstPass, nullptr);
		pDrawList->AddImage(pTextureX, { -1.0f, -1.0f }, { 1.0f, 1.0f });
		pDrawList->AddCallback(&SecondPass, nullptr);
		pDrawList->AddImage(pTextureY, { -1.0f, -1.0f }, { 1.0f, 1.0f });
	}
	pDrawList->AddCallback(&End, nullptr);
	pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

	pDrawList->AddImageRounded(pTextureX, vecMin, vecMax, { 1.0f / (ImGui::GetIO().DisplaySize.x / vecMin.x), 1.0f / (ImGui::GetIO().DisplaySize.y / vecMin.y) }, { 1.0f / (ImGui::GetIO().DisplaySize.x / vecMax.x), 1.0f / (ImGui::GetIO().DisplaySize.y / vecMax.y) }, Color, fRounding, DrawFlags);
}

void Blur::Reset()
{
	if (pTextureX != nullptr)
	{
		pTextureX->Release();
		pTextureX = nullptr;
	}

	if (pTextureY != nullptr)
	{
		pTextureY->Release();
		pTextureY = nullptr;
	}
}
