
#pragma once
#include <stdio.h>
#include<windows.h>
#pragma comment(lib, "Ws2_32.lib")

class M_Client
{
public:
	//Constructor
	M_Client();

	void Initial(const char* Addr, int Port, int Id);
	//����
	int Connect();
	//����TCP��
	int SendMsg(const char* msg, int len);
	//�ر�
	void Close();
	//����modbus��-���Ĵ���
	void Modbus_sender_single(int Ref, int addr, int value);
	//����modbus��-��Ĵ���
	int Modbus_sender_multi(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int IsPaper3, int IsPaper4, int value5, int value6);
	int Modbus_sender_multi2(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6);

	//����TCP��
	int RecvMsg(char* msg, int len);
	//����modbus��-���Ĵ���
	void Modbus_recv_single(int Ref, int addr, int value);

private:
	SOCKET m_sock;
	int port;
	const char* address;
	int id;
};