#include "framework.h"
#include "Keyboard.h"

//Ű���� ��ü �ʱ�ȭ
Keyboard* Keyboard::instance = NULL;

Keyboard * Keyboard::Get()
{
	//��ü�� ������ ����
	assert(instance != NULL);

	//������ �ִ� ��ü ��ȯ
	return instance;
}

void Keyboard::Create()
{
	//��ü�� �̹� ��������� ����
	assert(instance == NULL);

	//Ű���� ��ü ����
	instance = new Keyboard();
}

void Keyboard::Delete()
{
	//��ü ����
	SafeDelete(instance);
}

void Keyboard::Update()
{
	//keyState�� ���� keyOldState�� ����(���� �Է°����� ��ȯ)
	memcpy(keyOldState, keyState, sizeof(keyOldState));

	//keyState �ʱ�ȭ
	ZeroMemory(keyState, sizeof(keyState));
	//Ű�� �ʱ�ȭ
	ZeroMemory(keyMap, sizeof(keyMap));

	//���� keyState�� �Է� ����
	GetKeyboardState(keyState);

	for (DWORD i = 0; i < MAX_INPUT_KEY; i++)
	{
		//Ű�� ���ȴ��� �ƴ��� �Ǻ�
		byte key = keyState[i] & 0x80;
		//�������� 1 �ƴϸ� 0
		keyState[i] = key ? 1 : 0;

		int oldState = keyOldState[i];
		int state = keyState[i];

		//������ �ȴ����� ���� �������·�
		if (oldState == 0 && state == 1)
			keyMap[i] = KEY_INPUT_STATUS_DOWN; //���� 0, ���� 1 - KeyDown
		//�����ٰ� �� ����
		else if (oldState == 1 && state == 0)
			keyMap[i] = KEY_INPUT_STATUS_UP; //���� 1, ���� 0 - KeyUp
		//��� ������ �ִ� ����
		else if (oldState == 1 && state == 1)
			keyMap[i] = KEY_INPUT_STATUS_PRESS; //���� 1, ���� 1 - KeyPress
		//�׿�, ��� �ȴ��� ����
		else
			keyMap[i] = KEY_INPUT_STATUS_NONE;
	}
}

Keyboard::Keyboard()
{
	//Ű���� ���� ��ü �ʱ�ȭ
	ZeroMemory(keyState, sizeof(keyState));
	ZeroMemory(keyOldState, sizeof(keyOldState));
	ZeroMemory(keyMap, sizeof(keyMap));
}

Keyboard::~Keyboard()
{

}
