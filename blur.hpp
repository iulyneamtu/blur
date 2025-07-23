#pragma once

namespace Blur
{
	void AddBlur(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, float fRounding, ImDrawFlags DrawFlags, ImColor Color);
	void Reset();

	inline IDirect3DSurface9* pBackupSurface{ nullptr };

	inline IDirect3DPixelShader9* pBlurShaderX{ nullptr };
	inline IDirect3DPixelShader9* pBlurShaderY{ nullptr };

	inline IDirect3DTexture9* pTextureX{ nullptr };
	inline IDirect3DTexture9* pTextureY{ nullptr };

	constexpr int iDownSample = 4;
}