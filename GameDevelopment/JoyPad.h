/// <summary>
/// JoyPad�i�R���g���[���j�������N���X
/// </summary>
#pragma once

// DirectInput�̃o�[�W�������w��
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

	// ������
	bool Initialize(HWND window);
	// �X�V
	void Update();

private:

	// �W���C�p�b�h1���̏��
	struct JoyPadSet {

		JoyPadSet() {
			inputDevice = NULL;
		}

		// �f�o�C�X
		LPDIRECTINPUTDEVICE8 inputDevice;
		// ���͏��
		DIJOYSTATE2 joypad;
		// �O��̓��͏��
		DIJOYSTATE2 joypadOld;
	};

	// DirectInput�C���X�^���X
	LPDIRECTINPUT8 m_pDInput;

	// �W���C�p�b�h�z��
	std::vector<JoyPadSet> m_joyPadSet;
};