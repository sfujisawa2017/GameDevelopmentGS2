/// <summary>
/// JoyPad（コントローラ）を扱うクラス
/// </summary>
#pragma once

// DirectInputのバージョンを指定
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <vector>

class JoyPad
{
public:
	JoyPad();

	~JoyPad();

	bool Initialize(HWND window);

private:
	// DirectInputインスタンス
	LPDIRECTINPUT8 m_pDInput;
};