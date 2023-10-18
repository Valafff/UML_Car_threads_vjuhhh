#include<iostream>
#include <conio.h>
#include <thread>
#include <Windows.h>

using namespace std;

#define Enter 13
#define Escape 27

#define MIN_TANK_VOLUME	 35
#define MAX_TANK_VOLUME	160


class Tank
{
	const int VOLUME;	//�������������� �������
	double fuel_level;	//��������� �������

public:
	int get_VOLUME()const
	{
		return VOLUME;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}
	void fill(double fuel)
	{
		if (fuel < 0)return;
		if (fuel + this->fuel_level > VOLUME)this->fuel_level = VOLUME;
		else this->fuel_level += fuel;
	}
	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0)fuel_level = 0;
		return fuel_level;
	}
	Tank(int volume) :VOLUME(
		volume < MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
		volume > MAX_TANK_VOLUME ? MAX_TANK_VOLUME :
		volume),
		fuel_level(0)
	{
		/*
		if (volume < 35)volume = 35;
		if (volume > 160)volume = 160;
		this->VOLUME = volume;
		*/
		cout << "Tank_Constructor:\t" << this << endl;
	}
	~Tank()
	{
		cout << "Tank_Destructor:\t" << this << endl;
	}

	void info()const
	{
		cout << "����� ����:\t" << get_VOLUME() << " �����.\n";
		cout << "������ �������:\t" << get_fuel_level() << " �����.\n";
	}
};

#define MIN_ENGINE_CONSUMPTION	4
#define MAX_ENGINE_CONSUMPTION	30

class Engine
{
	const double CONSUMPTION;
	double consumption_per_second;
	bool is_started;
public:
	double get_CONSUMPTION()const
	{
		return CONSUMPTION;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	//��������� ������� � �������
	void set_consumption_per_second(double consumption)
	{
		consumption_per_second = consumption * 1e-5;//3 * 10 � -5� �������
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started()const
	{
		return is_started;
	}
	Engine(double consumption) :
		CONSUMPTION
		(
			consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
			consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
			consumption
		)
	{
		set_consumption_per_second(CONSUMPTION);
		is_started = false;
		cout << "Engine_Constructor:\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine_Destructor:\t" << this << endl;
	}
	void info()const
	{
		cout << "������ �� 100 ��.:  " << CONSUMPTION << " �." << endl;
		cout << "������ �� 1 �������: " << consumption_per_second << " �." << endl;
		cout << "��������� " << (is_started ? "�������" : "����������") << endl;
	}
};

#define MAX_SPEED_LOW_LIMIT		120
#define MAX_SPEED_HIGHT_LIMIT	400
#define MIN_ACCELLERATION		 5
#define MAX_ACCELLERATION		 40

#define MAX_BACK_SPEED_LOW_LIMIT	0
#define MAX_BACK_SPEED_HIGHT_LIMIT	-30

class Car
{
	Engine engine;
	Tank tank;
	const int MAX_SPEED;
	const int MAX_BACK_SPEED;
	const int ACCELERATION;
	int speed;
	int friction_speed_down;
	bool driver_inside;

	bool drive_on = false;
	// ���� ������������ ����������� ���������� �������� � ������ ���������
	bool time_to_do = false;
	double default_consumption;


	struct // ��������� ��������� ������ ���� ������ threads
	{

		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread drive_thread;

	}threads;

public:
	Car(Engine engine, Tank tank, int max_speed = 250, int max_back_speed = -20, int accelleration = 15, int friction_speed_down = 1) :

		speed(0),
		//���������� ������� ����������� �����������
		tank(tank),		//Implicit copy constructor 
		engine(engine),	//Implicit copy constructor
		MAX_SPEED
		(
			max_speed < MAX_SPEED_LOW_LIMIT ? MAX_SPEED_LOW_LIMIT :
			max_speed > MAX_SPEED_HIGHT_LIMIT ? MAX_SPEED_HIGHT_LIMIT :
			max_speed
		),
		MAX_BACK_SPEED
		(
			max_back_speed > MAX_BACK_SPEED_LOW_LIMIT ? MAX_BACK_SPEED_LOW_LIMIT :
			max_back_speed < MAX_BACK_SPEED_HIGHT_LIMIT ? MAX_BACK_SPEED_HIGHT_LIMIT :
			max_back_speed
		),
		ACCELERATION
		(
			accelleration < MIN_ACCELLERATION ? MIN_ACCELLERATION :
			accelleration > MAX_ACCELLERATION ? MAX_ACCELLERATION :
			accelleration
		)
	{
		driver_inside = false;
		// ������ �� ��������� ���������� ����� ��������� ��� ���������� �������
		default_consumption = engine.get_CONSUMPTION();
		this->friction_speed_down = friction_speed_down;
		cout << "���� ������ ������! ������� Enter ����� ����� � ��" << endl;
	}

	~Car()
	{
		cout << "Car_Destructor:\t" << this << endl;
	}

	int get_speed() const
	{
		return speed;
	}

	void set_speed(int speed)
	{
		this->speed = speed;
	}

	void get_in()
	{
		driver_inside = true;
		//������� ����� ��� ������ Panel()
		threads.panel_thread = std::thread(&Car::Panel, this);
	}

	void get_out()
	{

		system("CLS");
		driver_inside = false;
		if (threads.panel_thread.joinable())
		{
			threads.panel_thread.join();
		}
		cout << "�� ����� �� ������. ��� �������� ���� ������� \"F\"" << endl;

	}

	void start()
	{
		//if (tank.get_fuel_level())
		{
			engine.start();
			threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}

	void stop()
	{
		engine.stop();
		if (threads.engine_idle_thread.joinable())
		{
			threads.engine_idle_thread.join();
		}
	}

	void start_drive()
	{
		threads.drive_thread = std::thread(&Car::drive, this);
	}

	void stop_drive()
	{
		if (threads.drive_thread.joinable())
		{
			threads.drive_thread.join();
			drive_on = false;
		}
	}


	void drive()
	{
		while (speed != 0 /*and engine.started()*/)
		{
			if (speed > 0)
			{
				speed -= friction_speed_down;
			}
			else if (speed < 0)
			{
				speed += friction_speed_down;
			}
			std::this_thread::sleep_for(1s);
		}
	}

	//�������� �������� �� ���������
	void is_started()
	{
		if (!drive_on and engine.started())
		{
			start_drive();
		}
		if (engine.started())
		{
			drive_on = true;
		}

	}


	void do_Acceleration(char input)
	{

		if (time_to_do and (input == 'w' or input == 'W'))
		{
			//engine.set_consumption_per_second(10000);
			if (speed < MAX_SPEED - ACCELERATION)
			{
				speed += ACCELERATION;
			}
			else
			{
				speed = MAX_SPEED;
			}

		}
		else if (time_to_do and (input == 's' or input == 'S'))
		{
			//engine.set_consumption_per_second(10000);
			if (speed > MAX_BACK_SPEED + ACCELERATION)
			{
				speed -= ACCELERATION;
			}
			else
			{
				speed = MAX_BACK_SPEED;
			}

		}
		time_to_do = false;
	}


#define CONSUMPTION_1_60 200
#define CONSUMPTION_61_100 140
#define CONSUMPTION_101_140 200
#define CONSUMPTION_141_200 250
#define CONSUMPTION_201_250 300

	void engine_idle()
	{

		while (engine.started() and tank.give_fuel(engine.get_consumption_per_second()))
		{
			std::this_thread::sleep_for(1s);
			//��������� ������� ������� � ����������� �� ��������
			 if (abs(speed)>1 and abs(speed) <=60)
			{
				engine.set_consumption_per_second(CONSUMPTION_1_60);
			}
			else if (abs(speed) > 60 and abs(speed) <= 100)
			{
				engine.set_consumption_per_second(CONSUMPTION_61_100);
			}
			else if (abs(speed) > 100 and abs(speed) <= 140)
			{
				engine.set_consumption_per_second(CONSUMPTION_101_140);
			}
			else if (abs(speed) > 140 and abs(speed) <= 200)
			{
				engine.set_consumption_per_second(CONSUMPTION_141_200);
			}
			else if (abs(speed) > 200 and abs(speed) <= 250)
			{
				engine.set_consumption_per_second(CONSUMPTION_201_250);
			}
			else
			{
				engine.set_consumption_per_second(default_consumption);
			}
			// ��� �������������� ����� ����� ��������� �������� � ������ control � ��� ������������� �������
			time_to_do = true;
		}
	}

	void Panel()
	{

		while (driver_inside)
		{
			system("cls");
			cout << "�� ���������� � ������" << endl;
			cout << "������� \"I\" ��� ������� ��������� ��� \"Enter\" ����� ����� �� ����" << endl;
			cout << "������� ������� � ���� :\t" << tank.get_fuel_level() << " �����.\n";
			cout << "������� ����������� �������\t" << (engine.started() ? engine.get_consumption_per_second() : NULL) << " �����/���." << endl;
			cout << "��������� " << (engine.started() ? "�������" : "����������") << endl;
			if (speed == 0)
			{
				cout << "���������� �����" << endl;
			}
			else if (speed > 0)
			{
				cout << "���������� �������� ������" << endl;
			}
			else
			{
				cout << "���������� �������� �����" << endl;
			}
			cout << "�������� ������\t" << speed << "\t��/�" << endl;
			// �������� ������� ������ ������� � ���������
			if (tank.get_fuel_level() < 5)
			{
				cout << "��������! ������ ������� �������!" << endl;
			}

			std::this_thread::sleep_for(1s);

		}
	}

	void info()const
	{
		engine.info();
		tank.info();
		cout << "������������ ��������:    \t" << MAX_SPEED << " ��/�\n";
		cout << "���������:\t" << ACCELERATION << " ��/�\n";
		cout << "������� ��������:\t\t\t" << speed << " ��/�\n";
	}

	//�������� ����� ���������� �������
	void control()
	{
		char key;
		do
		{
			key = 0;
			// ������� _kbhit() ������� ������� �������, ���������� true ��� ������� ����� �������
			if (_kbhit())
			{
				key = _getch(); //������� ������� ������� ���������� SS�II ��� 
			}

			//�������� �������� ������
			if (speed == 0)
			{
				stop_drive();
			}

			switch (key)
			{
				//���� ����� � ����
			case Enter:
				if (driver_inside and !drive_on)
				{
					//cout << "&*^*&^*&^*�� ����� �� ������. ��� �������� ���� ������� \"F\"" << endl;
					// ������������� ����� �������� Panel()
					get_out();
				}
				else if (driver_inside and drive_on)
				{
					cout << "� ��� ����������� ���� \"����������\" :-( " << endl;
				}
				else
				{
					//cout << "�� ����� � ������" << endl;
					//��������� ����� � ������� Panel()
					get_in();
				}
				break;
				//�������� ����
			case 'F'://Fill
			case 'f':

				if (driver_inside)
				{
					cout << "��� �������� ���� ������� �� ������! � ��� ��� ����������������!";
					//Sleep(100000);
					break;
				}
				cout << "������� ����� ������� ";
				double fuel;
				cin >> fuel;
				tank.fill(fuel);
				cout << "��� ��������� ��\t" << tank.get_fuel_level() << "\t ������" << endl;
				break;
				//���������
			case 'I':
			case 'i':
				if (driver_inside and !drive_on)
				{
					engine.started() ? stop() : start();
					if (tank.get_fuel_level() == 0)
					{
						cout << "��� ����!" << endl;
					}
					break;
				}
				else if (driver_inside and drive_on)
				{
					cout << "\"�������� ���� �� ����� ��������� �� ����� �������� ���������� ������������� �����������!\" ������������� ���������� ���. 111" << endl;
				}
				// ����� ������
			case 'W':
			case 'w':
			{
				is_started();
				if (engine.started())
				{
					do_Acceleration(key);
				}
				break;
			}
			//����� �����, ���������
			case 'S':
			case 's':
			{
				is_started();
				if (engine.started())
				{
					do_Acceleration(key);
				}
				else 
				{
					speed -= ACCELERATION;
					if (speed < 0)
					{
						speed = 0;
					}
					Sleep(1000);
				}
				break;
			}
			//�����
			case Escape:

				speed = 0;
				stop_drive();
				stop();
				get_out();
			}

			if (tank.get_fuel_level() == 0)
			{

				stop();
			}

		} while (key != Escape);

		cout << "����!" << endl;
	}
};

//Deep copy - ��������� �����������
//Shallow copy - ������������� �����������

//#define TANK_CHECK
//��������� #define (����������) ������� ����������������
//#define ENGINE_CHECK

void main()
{
	setlocale(LC_ALL, "");

#ifdef TANK_CHECK		//���������� ������ ����� ����
	//���� ���������� 'TANK_CHECK', �� ������������� ��� ����� ����� �����������:
	Tank tank(20);
	tank.info();
	while (true)
	{
		double fuel;
		cout << "������� ����� �������: "; cin >> fuel;
		tank.fill(fuel);
		tank.info();
	}
#endif // TANK_CHECK	//���������� ����� ����� ����

#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif // ENGINE_CHECK


	// � ����������� ���������� 2 ��������� - ��������� ��� ������ �  ����� ����. ���� ��� ����
	Car car(30, 60);
	//car.info();
	car.control();
}