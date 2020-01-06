#include "framework.h"
#include "Keyboard.h"

//키보드 객체 초기화
Keyboard* Keyboard::instance = NULL;

Keyboard * Keyboard::Get()
{
	//객체가 없으면 에러
	assert(instance != NULL);

	//가지고 있는 객체 반환
	return instance;
}

void Keyboard::Create()
{
	//객체를 이미 만들었으면 에러
	assert(instance == NULL);

	//키보드 객체 생성
	instance = new Keyboard();
}

void Keyboard::Delete()
{
	//객체 제거
	SafeDelete(instance);
}

void Keyboard::Update()
{
	//keyState의 값을 keyOldState에 복사(이전 입력값으로 변환)
	memcpy(keyOldState, keyState, sizeof(keyOldState));

	//keyState 초기화
	ZeroMemory(keyState, sizeof(keyState));
	//키맵 초기화
	ZeroMemory(keyMap, sizeof(keyMap));

	//현재 keyState를 입력 받음
	GetKeyboardState(keyState);

	for (DWORD i = 0; i < MAX_INPUT_KEY; i++)
	{
		//키가 눌렸는지 아닌지 판별
		byte key = keyState[i] & 0x80;
		//눌렸으면 1 아니면 0
		keyState[i] = key ? 1 : 0;

		int oldState = keyOldState[i];
		int state = keyState[i];

		//이전에 안눌렀고 지금 누른상태로
		if (oldState == 0 && state == 1)
			keyMap[i] = KEY_INPUT_STATUS_DOWN; //이전 0, 현재 1 - KeyDown
		//눌렀다가 뗀 상태
		else if (oldState == 1 && state == 0)
			keyMap[i] = KEY_INPUT_STATUS_UP; //이전 1, 현재 0 - KeyUp
		//계속 누르고 있는 상태
		else if (oldState == 1 && state == 1)
			keyMap[i] = KEY_INPUT_STATUS_PRESS; //이전 1, 현재 1 - KeyPress
		//그외, 계속 안누른 상태
		else
			keyMap[i] = KEY_INPUT_STATUS_NONE;
	}
}

Keyboard::Keyboard()
{
	//키보드 변수 전체 초기화
	ZeroMemory(keyState, sizeof(keyState));
	ZeroMemory(keyOldState, sizeof(keyOldState));
	ZeroMemory(keyMap, sizeof(keyMap));
}

Keyboard::~Keyboard()
{

}
