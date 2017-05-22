#include "JoyPad.h"
#include <assert.h>

// 十字キーの方向を定義する
enum DPadDirection
{
	DIRECTION_UP = 0,
	DIRECTION_UP_RIGHT = 4500,
	DIRECTION_RIGHT = 9000,
	DIRECTION_DOWN_RIGHT = 13500,
	DIRECTION_DOWN = 18000,
	DIRECTION_DOWN_LEFT = 22500,
	DIRECTION_LEFT = 27000,
	DIRECTION_UP_LEFT = 31500,

};

// 検出したジョイパッドの情報を格納するための構造体
struct SearchJoyPadSet
{
	// DirectInput
	LPDIRECTINPUT8 dinput;

	// 各コントローラごとのデバイス
	std::vector<LPDIRECTINPUTDEVICE8> joyPadDev;

	// コンストラクタ
	SearchJoyPadSet()
	{
		dinput = NULL;
	}
};

//// 検出した入力オブジェクト１個ごとに呼ばれるコールバック
//BOOL CALLBACK EnumObjectCallBack(LPCDIDEVICEOBJECTINSTANCE pInstance, void* pvRef)
//{
//
//
//	// 次を検索
//	return DIENUM_CONTINUE;
//}

// 検出したジョイパッドごとに呼ばれるコールバック
BOOL CALLBACK EnumJoyPadCallBack(LPCDIDEVICEINSTANCE lpddi, void* pvRef)
{
	// 結果
	HRESULT ret;

	SearchJoyPadSet* param = reinterpret_cast<SearchJoyPadSet*>(pvRef);

	// DirectInputDevice
	LPDIRECTINPUTDEVICE8 dev;

	// コントローラごとにデバイスを生成
	ret = param->dinput->CreateDevice(
		lpddi->guidInstance,
		&dev,
		NULL);

	// 失敗したら
	if (ret != DI_OK)
		goto NEXT;

	// データ形式を設定（拡張機能つきジョイスティック）
	ret = dev->SetDataFormat(&c_dfDIJoystick2);

	// 失敗したら
	if (ret != DI_OK)
		goto NEXT;

	// 配列に追加
	param->joyPadDev.push_back(dev);

NEXT:
	// 次を検索
	return DIENUM_CONTINUE;
}

JoyPad::JoyPad()
{
}

JoyPad::~JoyPad()
{
	// 配列にあるデバイスを開放
	std::vector<JoyPadSet>::iterator it;
	for (it = m_joyPadSet.begin(); it != m_joyPadSet.end(); it++)
	{
		SafeRelease(it->inputDevice);
	}

}

bool JoyPad::Initialize(HWND window)
{
	// 結果
	HRESULT ret;

	// DirectInputの作成
	ret = DirectInput8Create(
		GetModuleHandle(NULL),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pDInput,
		NULL);

	// 失敗したら
	if (ret != DI_OK)
		return false;

	// 受け渡し用パラメータ
	SearchJoyPadSet param;
	param.dinput = m_pDInput;

	// 利用可能なデバイスを列挙する
	ret = m_pDInput->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		EnumJoyPadCallBack,
		&param,
		DIEDFL_ATTACHEDONLY);

	// 失敗したら
	if (ret != DI_OK)
		return false;

	// デバイス配列を設定
	std::vector<LPDIRECTINPUTDEVICE8>::iterator it;
	for (it = param.joyPadDev.begin(); it != param.joyPadDev.end(); it++)
	{
		LPDIRECTINPUTDEVICE8 dev = *it;

		// アプリがアクティブ時はデバイスを排他アクセスに設定
		ret = dev->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

		// 失敗したら
		if (ret != DI_OK)
			continue;

		// アクセス権を要求
		ret = dev->Acquire();

		//// 失敗したら
		//if (ret != DI_OK)
		//	continue;

		// デバイス１個分の情報
		JoyPadSet initPad;
		initPad.inputDevice = dev;

		// 配列に追加する
		m_joyPadSet.push_back(initPad);

	}

	return true;
}

void JoyPad::Update()
{
	// 全てのデバイスについて処理する
	std::vector<JoyPadSet>::iterator it;
	for (it = m_joyPadSet.begin(); it != m_joyPadSet.end(); it++)
	{
		HRESULT ret;

		// 情報を更新する
		ret = it->inputDevice->Poll();
		if (ret != DI_OK)
		{
			// アクセス権を要求
			ret = it->inputDevice->Acquire();
			while (ret == DIERR_INPUTLOST)
			{
				// アクセス権を要求
				ret = it->inputDevice->Acquire();
			}
		}

		// 前フレームの入力情報をバックアップ
		it->joypadOld = it->joypad;
		// 入力情報を取得
		ret = it->inputDevice->GetDeviceState(sizeof(DIJOYSTATE2), &it->joypad);

		// 失敗したら
		if (ret != DI_OK)
			continue;

		// ボタン状態の更新
		for (int j = 0; j < 32; j++)
		{
			// 状態によって経過を監視
			if (it->joypad.rgbButtons[j])
			{
				it->m_buttonTime[j]++;
			}
			else {
				it->m_buttonTime[j] = 0;
			}
		}
	}
}

//---------------------------------------
// キーを押しているか関数
// [In] パッドID : ボタン ID
// [ret]バッドIDのボタンIDが押されている(true)
//		押されていない(false)
//---------------------------------------
bool JoyPad::buttonPush(int padId, int buttonId)
{
	assert(buttonId >= 0 && buttonId < 128);
	assert(m_joyPadSet.size() > padId);

	if (m_joyPadSet[padId].joypad.rgbButtons[buttonId])
	{
		return true;
	}
	return false;
}
//---------------------------------------
// キーが押された瞬間か関数
// [In] パッドID : ボタン ID
// [ret]バッドIDのボタンIDが押された瞬間(true)
//		押された瞬間ではない(false)
//---------------------------------------
bool JoyPad::buttonTrigger(int padId, int buttonId)
{
	assert(buttonId >= 0 && buttonId < 128);
	assert(m_joyPadSet.size() > padId);
	if (m_joyPadSet[padId].joypad.rgbButtons[buttonId] &&
		!m_joyPadSet[padId].joypadOld.rgbButtons[buttonId])
	{
		return true;
	}
	return false;
}
//---------------------------------------
// キーを離した瞬間か関数
// [In] パッドID : ボタン ID
// [ret]バッドIDのボタンIDを離した瞬間(true)
//		離した瞬間ではない(false)
//---------------------------------------
bool JoyPad::buttonRelease(int padId, int buttonId)
{
	assert(buttonId >= 0 && buttonId < 128);
	assert(m_joyPadSet.size() > padId);
	if (!m_joyPadSet[padId].joypad.rgbButtons[buttonId] &&
		m_joyPadSet[padId].joypadOld.rgbButtons[buttonId])
	{
		return true;
	}
	return false;
}
//---------------------------------------
// キーを押している時間
// [In] パッドID : ボタン ID
// [ret]経過時間
//---------------------------------------
int JoyPad::buttonPushTime(int padId, int buttonId)
{
	assert(buttonId >= 0 && buttonId < 128);
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].m_buttonTime[buttonId];
}
//---------------------------------------
// LXスティックの傾き
// [In] パッドID
// [ret]スティックの傾き
//---------------------------------------
LONG JoyPad::getStickLX(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lX;
}
//---------------------------------------
// LYスティックの傾き
// [In] パッドID
// [ret]スティックの傾き
//---------------------------------------
LONG JoyPad::getStickLY(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lY;
}
//---------------------------------------
// LZスティックの傾き
// [In] パッドID
// [ret]スティックの傾き
//---------------------------------------
LONG JoyPad::getStickLZ(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lZ;
}
//---------------------------------------
// LRXスティックの傾き
// [In] パッドID
// [ret]スティックの傾き
//---------------------------------------
LONG JoyPad::getStickLRX(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lRx;
}
//---------------------------------------
// LRYスティックの傾き
// [In] パッドID
// [ret]スティックの傾き
//---------------------------------------
LONG JoyPad::getStickLRY(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lRy;
}
//---------------------------------------
// LRZスティックの傾き
// [In] パッドID
// [ret]スティックの傾き
//---------------------------------------
LONG JoyPad::getStickLRZ(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lRz;
}
//---------------------------------------
// 方向キーの取得
// [In] パッドID : キーID
// [ret] 方向キー
//---------------------------------------
BYTE JoyPad::getDirectionKey(int padId)
{
	assert(m_joyPadSet.size() > padId);

	BYTE returnCode = -1;
	switch (m_joyPadSet[padId].joypad.rgdwPOV[0])
	{
	case DPadDirection::DIRECTION_UP:			returnCode = PAD_DIR_UP;					break;
	case DPadDirection::DIRECTION_UP_RIGHT:		returnCode = PAD_DIR_UP | PAD_DIR_RIGHT;	break;
	case DPadDirection::DIRECTION_RIGHT:		returnCode = PAD_DIR_RIGHT;					break;
	case DPadDirection::DIRECTION_DOWN_RIGHT:	returnCode = PAD_DIR_DOWN | PAD_DIR_RIGHT;	break;
	case DPadDirection::DIRECTION_DOWN:			returnCode = PAD_DIR_DOWN;					break;
	case DPadDirection::DIRECTION_DOWN_LEFT:	returnCode = PAD_DIR_DOWN | PAD_DIR_LEFT;	break;
	case DPadDirection::DIRECTION_LEFT:			returnCode = PAD_DIR_LEFT;					break;
	case DPadDirection::DIRECTION_UP_LEFT:		returnCode = PAD_DIR_UP | PAD_DIR_LEFT;		break;
	default:									returnCode = PAD_DIR_NONE;					break;
	}
	return returnCode;
}