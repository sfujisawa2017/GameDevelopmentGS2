#include "JoyPad.h"
#include <assert.h>

// �\���L�[�̕������`����
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

// ���o�����W���C�p�b�h�̏����i�[���邽�߂̍\����
struct SearchJoyPadSet
{
	// DirectInput
	LPDIRECTINPUT8 dinput;

	// �e�R���g���[�����Ƃ̃f�o�C�X
	std::vector<LPDIRECTINPUTDEVICE8> joyPadDev;

	// �R���X�g���N�^
	SearchJoyPadSet()
	{
		dinput = NULL;
	}
};

//// ���o�������̓I�u�W�F�N�g�P���ƂɌĂ΂��R�[���o�b�N
//BOOL CALLBACK EnumObjectCallBack(LPCDIDEVICEOBJECTINSTANCE pInstance, void* pvRef)
//{
//
//
//	// ��������
//	return DIENUM_CONTINUE;
//}

// ���o�����W���C�p�b�h���ƂɌĂ΂��R�[���o�b�N
BOOL CALLBACK EnumJoyPadCallBack(LPCDIDEVICEINSTANCE lpddi, void* pvRef)
{
	// ����
	HRESULT ret;

	SearchJoyPadSet* param = reinterpret_cast<SearchJoyPadSet*>(pvRef);

	// DirectInputDevice
	LPDIRECTINPUTDEVICE8 dev;

	// �R���g���[�����ƂɃf�o�C�X�𐶐�
	ret = param->dinput->CreateDevice(
		lpddi->guidInstance,
		&dev,
		NULL);

	// ���s������
	if (ret != DI_OK)
		goto NEXT;

	// �f�[�^�`����ݒ�i�g���@�\���W���C�X�e�B�b�N�j
	ret = dev->SetDataFormat(&c_dfDIJoystick2);

	// ���s������
	if (ret != DI_OK)
		goto NEXT;

	// �z��ɒǉ�
	param->joyPadDev.push_back(dev);

NEXT:
	// ��������
	return DIENUM_CONTINUE;
}

JoyPad::JoyPad()
{
}

JoyPad::~JoyPad()
{
	// �z��ɂ���f�o�C�X���J��
	std::vector<JoyPadSet>::iterator it;
	for (it = m_joyPadSet.begin(); it != m_joyPadSet.end(); it++)
	{
		SafeRelease(it->inputDevice);
	}

}

bool JoyPad::Initialize(HWND window)
{
	// ����
	HRESULT ret;

	// DirectInput�̍쐬
	ret = DirectInput8Create(
		GetModuleHandle(NULL),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pDInput,
		NULL);

	// ���s������
	if (ret != DI_OK)
		return false;

	// �󂯓n���p�p�����[�^
	SearchJoyPadSet param;
	param.dinput = m_pDInput;

	// ���p�\�ȃf�o�C�X��񋓂���
	ret = m_pDInput->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		EnumJoyPadCallBack,
		&param,
		DIEDFL_ATTACHEDONLY);

	// ���s������
	if (ret != DI_OK)
		return false;

	// �f�o�C�X�z���ݒ�
	std::vector<LPDIRECTINPUTDEVICE8>::iterator it;
	for (it = param.joyPadDev.begin(); it != param.joyPadDev.end(); it++)
	{
		LPDIRECTINPUTDEVICE8 dev = *it;

		// �A�v�����A�N�e�B�u���̓f�o�C�X��r���A�N�Z�X�ɐݒ�
		ret = dev->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

		// ���s������
		if (ret != DI_OK)
			continue;

		// �A�N�Z�X����v��
		ret = dev->Acquire();

		//// ���s������
		//if (ret != DI_OK)
		//	continue;

		// �f�o�C�X�P���̏��
		JoyPadSet initPad;
		initPad.inputDevice = dev;

		// �z��ɒǉ�����
		m_joyPadSet.push_back(initPad);

	}

	return true;
}

void JoyPad::Update()
{
	// �S�Ẵf�o�C�X�ɂ��ď�������
	std::vector<JoyPadSet>::iterator it;
	for (it = m_joyPadSet.begin(); it != m_joyPadSet.end(); it++)
	{
		HRESULT ret;

		// �����X�V����
		ret = it->inputDevice->Poll();
		if (ret != DI_OK)
		{
			// �A�N�Z�X����v��
			ret = it->inputDevice->Acquire();
			while (ret == DIERR_INPUTLOST)
			{
				// �A�N�Z�X����v��
				ret = it->inputDevice->Acquire();
			}
		}

		// �O�t���[���̓��͏����o�b�N�A�b�v
		it->joypadOld = it->joypad;
		// ���͏����擾
		ret = it->inputDevice->GetDeviceState(sizeof(DIJOYSTATE2), &it->joypad);

		// ���s������
		if (ret != DI_OK)
			continue;

		// �{�^����Ԃ̍X�V
		for (int j = 0; j < 32; j++)
		{
			// ��Ԃɂ���Čo�߂��Ď�
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
// �L�[�������Ă��邩�֐�
// [In] �p�b�hID : �{�^�� ID
// [ret]�o�b�hID�̃{�^��ID��������Ă���(true)
//		������Ă��Ȃ�(false)
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
// �L�[�������ꂽ�u�Ԃ��֐�
// [In] �p�b�hID : �{�^�� ID
// [ret]�o�b�hID�̃{�^��ID�������ꂽ�u��(true)
//		�����ꂽ�u�Ԃł͂Ȃ�(false)
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
// �L�[�𗣂����u�Ԃ��֐�
// [In] �p�b�hID : �{�^�� ID
// [ret]�o�b�hID�̃{�^��ID�𗣂����u��(true)
//		�������u�Ԃł͂Ȃ�(false)
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
// �L�[�������Ă��鎞��
// [In] �p�b�hID : �{�^�� ID
// [ret]�o�ߎ���
//---------------------------------------
int JoyPad::buttonPushTime(int padId, int buttonId)
{
	assert(buttonId >= 0 && buttonId < 128);
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].m_buttonTime[buttonId];
}
//---------------------------------------
// LX�X�e�B�b�N�̌X��
// [In] �p�b�hID
// [ret]�X�e�B�b�N�̌X��
//---------------------------------------
LONG JoyPad::getStickLX(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lX;
}
//---------------------------------------
// LY�X�e�B�b�N�̌X��
// [In] �p�b�hID
// [ret]�X�e�B�b�N�̌X��
//---------------------------------------
LONG JoyPad::getStickLY(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lY;
}
//---------------------------------------
// LZ�X�e�B�b�N�̌X��
// [In] �p�b�hID
// [ret]�X�e�B�b�N�̌X��
//---------------------------------------
LONG JoyPad::getStickLZ(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lZ;
}
//---------------------------------------
// LRX�X�e�B�b�N�̌X��
// [In] �p�b�hID
// [ret]�X�e�B�b�N�̌X��
//---------------------------------------
LONG JoyPad::getStickLRX(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lRx;
}
//---------------------------------------
// LRY�X�e�B�b�N�̌X��
// [In] �p�b�hID
// [ret]�X�e�B�b�N�̌X��
//---------------------------------------
LONG JoyPad::getStickLRY(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lRy;
}
//---------------------------------------
// LRZ�X�e�B�b�N�̌X��
// [In] �p�b�hID
// [ret]�X�e�B�b�N�̌X��
//---------------------------------------
LONG JoyPad::getStickLRZ(int padId)
{
	assert(m_joyPadSet.size() > padId);
	return m_joyPadSet[padId].joypad.lRz;
}
//---------------------------------------
// �����L�[�̎擾
// [In] �p�b�hID : �L�[ID
// [ret] �����L�[
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