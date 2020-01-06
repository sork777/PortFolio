#include "stdafx.h"
#include "ThreadDemo.h"

void ThreadDemo::Initialize()
{
	//Basic();
	//Mutex_RW();
	render2D = new Render2D();
	render2D->GetTransform()->Scale(D3D::Width(), D3D::Height(), 1);
	render2D->GetTransform()->Position(D3D::Width()*0.5f, D3D::Height()*0.5f, 0);

	srv = NULL;
	Time();
}

void ThreadDemo::Update()
{
	if (Keyboard::Get()->Down(VK_SPACE))
	{
		t = new thread([&] 
		{
			SafeRelease(srv);

			D3DX11_IMAGE_LOAD_INFO desc;
			desc.Width = 10000;
			desc.Height = 3197;
			desc.MipLevels = 8;
			//m.lock();
			Check(D3DX11CreateShaderResourceViewFromFile
			(
				D3D::GetDevice(), L"../../_Textures/heic.png", &desc, NULL, &srv, NULL
			));
			//m.unlock();
		});
	}
	
}

void ThreadDemo::Render()
{
	if (srv != NULL)
	{
		if (t != NULL)
		{
			t->join();
			SafeDelete(t);
		}
		render2D->SRV(srv);
		render2D->Update();
		render2D->Render();
	}
}

void ThreadDemo::Func()
{
	for (int i = 0; i < 100; i++)
		printf("Func:%d\n", i);
	printf("--Func 종료\n");
}

void ThreadDemo::Func2()
{
	for (UINT i = 0; i < 10; i++)
	{
		//if (i % 100==0)
			printf("Func2:%d\n", i);
	}
	printf("--Func2 종료\n");
}

void ThreadDemo::WhileFunc()
{
	int i = 0;
	while (true)
	{

		if (Keyboard::Get()->Down(VK_SPACE))
			break;
		/* Sleep : 인터럽트를 강제로 만드는 api */
		Sleep(1000);

		printf("Sleep : %d\n",++i);
	}
	printf("while 종료\n");
}

void ThreadDemo::Basic()
{
	Func();
	Func2();
}

void ThreadDemo::MultiThread()
{
	//function<void()> f = bind(&ThreadDemo::Func,this);
	//thread t(f);
	thread t(bind(&ThreadDemo::Func, this));
	thread t2(bind(&ThreadDemo::Func2, this));

	//먼저 쓴놈이 종료되기전까지 뒤엣것이 안나옴
	t.join();
	printf("T 종료\n");
	t2.join();
	printf("T2 종료\n");
	//thread(bind(&ThreadDemo::Func));
}

void ThreadDemo::MultiThreadLamda()
{
	/* 
	[=] : 대입
	[&] : 참조
	*/
	thread t([=](int a,int b) {
		for (int i = a; i < b; i++)
			printf("T : %d\n", i);
	},1,100);

	t.join();

}

void ThreadDemo::Detach()
{
	thread t([] {
		for (int i = 0; i < 1000; i++)
			printf("T : %d\n", i);
	});
	thread t2([] {
		for (int i = 0; i < 10; i++)
			printf("T2 : %d\n", i);
	});

	/* join과 달리 실행중에 만나면 그냥 통과함 */
	t.detach();
	printf("T 종료\n");
	t2.detach();
	printf("T2 종료\n");
}

void ThreadDemo::Mutex()
{
	int val = 0;
	/* 임계 : 자물쇠*/
	mutex m;

	/* 전역변수 : t기준으로val과 m? 을 참조로 */	
	thread t([&] {
		for (int i = 0; i < 1000000; i++)
		{
			if (i % 5 == 0)
			{
				m.lock();
				{
					val++;
					printf("T : %d - 5 %d\n", i,val);
				}
				m.unlock();
			}
		}
	});

	thread t2([&] {
		for (int i = 0; i < 1000000; i++)
		{
			if (i % 10 == 0)
			{
				m.lock();
				{
					val++;
					printf("T2 : %d - 10 %d\n", i, val);
				}
				m.unlock();
			}
		}
	});

	t.join();
	t2.join();
}

void ThreadDemo::Mutex_RW()
{
	int val = 0;
	/* 임계 : 자물쇠*/
	mutex m;

	/* 전역변수 : t기준으로val과 m? 을 참조로 */
	thread t([&] {
		while(true)
		{
			Sleep(2);
			m.lock();
			{
				val++;
			}
			m.unlock();
		}
	});

	thread t2([&] {
		while (true)
		{
			Sleep(100);
			/* 접근만 할때는 값 변화가 없으므로 락 안함 */
			printf("%d\n",val);
		}
	});

	t.join();
	t2.join();
}

void ThreadDemo::Time()
{
	 timer.Start([]()
	{
		printf("Timer\n");
	}, 2000, 2);


	timer2.Start([]()
	{
		printf("Timer2\n");
	}, 3000, 3);


	timer3.Start([]()
	{
		printf("Timer3\n");
	}, 3000, 0);


	timer4.Start([&]()
	{
		timer3.Stop();

		printf("StopInterval\n");
	}, 12000, 1);
}
