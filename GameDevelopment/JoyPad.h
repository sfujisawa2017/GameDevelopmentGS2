/// <summary>
/// JoyPad（コントローラ）を扱うクラス
/// </summary>
#pragma once

// DirectInputのバージョンを指定
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <vector>

template<typename T>
void SafeRelease(T*& p)
{
	if (p)
	{
		p->Release();
		p = NULL;
	}
}

class JoyPad
{
public:
	JoyPad();

	~JoyPad();

	// 初期化
	bool Initialize(HWND window);
	// 更新
	void Update();

private:

	// ジョイパッド1個分の情報
	struct JoyPadSet {

		JoyPadSet() {
			inputDevice = NULL;
		}

		// デバイス
		LPDIRECTINPUTDEVICE8 inputDevice;
		// 入力情報
		DIJOYSTATE2 joypad;
		// 前回の入力情報
		DIJOYSTATE2 joypadOld;
	};

	// DirectInputインスタンス
	LPDIRECTINPUT8 m_pDInput;

	// ジョイパッド配列
	std::vector<JoyPadSet> m_joyPadSet;
};