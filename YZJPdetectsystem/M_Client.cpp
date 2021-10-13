
#include "stdafx.h"
#include "M_Client.h"
#include <errno.h>

M_Client::M_Client()
{

}

void M_Client::Initial(const char* Addr, int Port, int Id)
{
	address = Addr;
	port = Port;
	id = Id;
}

int M_Client::Connect()
{
	int rlt = 0;

	//���ڼ�¼������Ϣ�����
	int iErrMsg;
	//����WinSock
	WSAData wsaData;
	iErrMsg = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (iErrMsg != NO_ERROR)
		//�д���
	{
		printf("failed with wsaStartup error : %d\n", iErrMsg);

		rlt = 1;
		return rlt;
	}

	//����Socket
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET)
		//����Socketʧ��
	{
		printf("socket failed with error : %d\n", WSAGetLastError());

		rlt = 2;
		return rlt;
	}

	//Ŀ�����������
	sockaddr_in sockaddrServer;
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_port = htons(port);
	sockaddrServer.sin_addr.s_addr = inet_addr(address);

	//����,sock��Ŀ�����������
	iErrMsg = connect(m_sock, (sockaddr*)&sockaddrServer, sizeof(sockaddrServer));


	if (iErrMsg < 0)
	{
		printf("connect failed with error : %d\n", iErrMsg);
		printf("Error: %d\n", errno);
		rlt = 3;
		return rlt;
	}

	return rlt;
}



//����ֵ��1��˵�����ʹ���
int M_Client::SendMsg(const char* msg, int len)
{
	int rlt = 0;

	int iErrMsg = 0;

	//������Ϣ��ָ��sock������Ϣ
	iErrMsg = send(m_sock, msg, len, 0);
	if (iErrMsg < 0)
		//����ʧ��
	{
		printf("send msg failed with error : %d\n", iErrMsg);

		rlt = 1;
		return rlt;
	}

	return rlt;

}

//����ֵ��1��˵�����մ���
int M_Client::RecvMsg(char* msg, int len)
{
	int rlt = 0;

	int iErrMsg = 0;

	//buffer�������
	memset(msg, 0, len);
	//������Ϣ��ָ��sock������Ϣ
	iErrMsg = recv(m_sock, msg, len, 0);
	if (iErrMsg < 0)
		//����ʧ��
	{
		printf("recv msg failed with error : %d\n", iErrMsg);

		rlt = 1;
		return rlt;
	}

	return rlt;

}

void M_Client::Close()
{
	closesocket(m_sock);
}

void M_Client::Modbus_recv_single(int Ref, int addr, int value)  //��һ���Ĵ�����ʹ�ù�����16���޸ĺ�Ϳ���д���
{
	unsigned char Temp_buf[15];
	RecvMsg((char*)Temp_buf, 15);

	//while (size > 0)//ʣ�ಿ�ִ���0
	//{
	//	int RecvSize = recv(sock, buffer, size, 0);
	//	if (SOCKET_ERROR == RecvSize)
	//		return false;
	//	size = size - RecvSize;
	//	buffer += RecvSize;
	//}
	//return true;


}

void M_Client::Modbus_sender_single(int Ref, int addr, int value)  //дһ���Ĵ�����ʹ�ù�����16���޸ĺ�Ϳ���д���
{
	unsigned char Temp_buf[20];
	Temp_buf[0] = Ref;
	Temp_buf[1] = 0;
	Temp_buf[2] = 0;
	Temp_buf[3] = 0;
	Temp_buf[4] = 0;//��ID��ʼ�������ֽ���
	Temp_buf[5] = 9;
	Temp_buf[6] = id;//�ӻ�ID
	Temp_buf[7] = 16;//0
	Temp_buf[8] = (addr - 1) / 256;//addr head //��ʼ�ĵ�ַ
	Temp_buf[9] = (addr - 1) % 256;
	Temp_buf[10] = 0;//number of addr   //��ַ�ĳ���
	Temp_buf[11] = 1;
	Temp_buf[12] = 2;//# of Bytes for values    //һ������byte��ֵ
	Temp_buf[13] = value / 256;//values           //�����ֵ��������ֻ��һ���Ĵ�������дһ��ֵ
	Temp_buf[14] = value % 256;
	SendMsg((char*)Temp_buf, 15);   //�����ķ�����15Ϊ���ĳ��ȣ������ǹ̶���
}

#if 1
int M_Client::Modbus_sender_multi2(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6)  //д20���Ĵ�����ʹ�ù�����16���޸ĺ�Ϳ���д���
{
	OutputDebugString("xxx----Modbus���Ϳ�ʼ \n");
	int iErrMsg = 0;

	unsigned char Temp_buf[53];

	WORD btDevNo = (WORD)((int)(addr * 1));///��ʼ��ַ

	OutputDebugString("xxx----Modbus����1 \n");
	WORD val0 = (WORD)((int)(value * 1));///����ֵ
	WORD val1 = (WORD)((int)(value1));///���1
	WORD val2 = (WORD)((int)(value2));///���2
	WORD val3 = (WORD)((int)(value3));///���3
	WORD val4 = (WORD)((int)(value4));///���4
	WORD val5 = (WORD)((int)(value5));///����1
	WORD val6 = (WORD)((int)(value6));///����2

	OutputDebugString("xxx----Modbus����2 \n");
	WORD val_tile = (WORD)(IsPaper1);//��ֽ ����ֵ==0��ʾ��ֽ
	WORD val_core = (WORD)(IsPaper2);//оֽ

	OutputDebugString("xxx----Modbus����3 \n");

	//����
	//����Ԫ��ʶ����һ��ÿ��ͨ��֮�󽫱�Ҫ���1������ͬ��ͨ�����ݱ���
	Temp_buf[0] = (BYTE)(Ref >> 8);//�������ʶ��Hi
	Temp_buf[1] = (BYTE)Ref;//�������ʶ��Lo
	//Э���ʶ����00 00ΪmodbusЭ��
	Temp_buf[2] = 0x00;//Э���ʶ��
	Temp_buf[3] = 0x00;
	//���ݳ��ȣ�����ָʾ���������ݵĳ��ȣ���λ�ֽ�;
	Temp_buf[4] = 0x00;//��ID��ʼ�������ֽ���
	Temp_buf[5] = (7 + 40);//7+2*�Ĵ�������

	//��Ԫ��ʶ���ӻ�ID�����Ա�ʶ�����ڴ����߻��������ϵ�Զ�̷���˵ĵ�ַ
	//	Temp_buf[6] = 0x01;//�ӻ���ַ
	Temp_buf[6] = id;//�ӻ���ַ��Ĭ��ֵ0��255
	////����Ϊ����ͷ

	//�����룬���Ĵ���0x10
	Temp_buf[7] = 0x10;//���ܱ�ʶ
		//��ʼ��ַ
	Temp_buf[8] = (BYTE)(btDevNo >> 8);//��ʼ��ַ
	Temp_buf[9] = (BYTE)(btDevNo);//


	Temp_buf[10] = 0;//д��Ĵ�������Hi
	Temp_buf[11] = 20; //д��Ĵ�������Lo
	Temp_buf[12] = 40; //д�����ݵ��ֽڸ��� 
							   ///>����
	Temp_buf[13] = (BYTE)(val0 >> 8);//����
	Temp_buf[14] = (BYTE)val0;
	Temp_buf[15] = (BYTE)(val1 >> 8);//��ֽ������λ��Hi
	Temp_buf[16] = (BYTE)val1;//��ֽ������λ��Lo
	Temp_buf[17] = (BYTE)(val2 >> 8);//��ֽ������λ��
	Temp_buf[18] = (BYTE)val2;
	Temp_buf[19] = (BYTE)(val5 >> 8);//��ֽ�ŷ�
	Temp_buf[20] = (BYTE)val5;
	Temp_buf[21] = (BYTE)(val_tile >> 8);// ����ֽ
	Temp_buf[22] = (BYTE)val_tile;// 
	Temp_buf[23] = 0x00;// 
	Temp_buf[24] = 0x00;// 
	Temp_buf[25] = 0x00;// 
	Temp_buf[26] = 0x00;// 
	Temp_buf[27] = 0x00;// 
	Temp_buf[28] = 0x00;// 
	Temp_buf[29] = 0x00;// 
	Temp_buf[30] = 0x00;// 
	Temp_buf[31] = 0x00;// 
	Temp_buf[32] = 0x00;// 
	Temp_buf[33] = (BYTE)(val3 >> 8);//оֽ������λ��Hi
	Temp_buf[34] = (BYTE)val3;//оֽ������λ��Lo
	Temp_buf[35] = (BYTE)(val4 >> 8); //оֽ������λ��Hi
	Temp_buf[36] = (BYTE)val4;// оֽ������λ��Lo
	Temp_buf[37] = (BYTE)(val6 >> 8); //оֽ�ŷ�Hi
	Temp_buf[38] = (BYTE)val6;// оֽ�ŷ�Lo
	Temp_buf[39] = (BYTE)(val_core >> 8);// ����ֽ
	Temp_buf[40] = (BYTE)val_core;// 
	Temp_buf[41] = 0x00;// 
	Temp_buf[42] = 0x00;// 
	Temp_buf[43] = 0x00;// 
	Temp_buf[44] = 0x00;// 
	Temp_buf[45] = 0x00;// 
	Temp_buf[46] = 0x00;// 
	Temp_buf[47] = 0x00;// 
	Temp_buf[48] = 0x00;// 
	Temp_buf[49] = 0x00;// 
	Temp_buf[50] = 0x00;// 
	Temp_buf[51] = 0x00;// 
	Temp_buf[52] = 0x00;//
#if 0

	Temp_buf[0] = Ref;
	Temp_buf[1] = 0;
	Temp_buf[2] = 0;
	Temp_buf[3] = 0;
	Temp_buf[4] = 0;//��ID��ʼ�������ֽ���
	Temp_buf[5] = 47;//7+2*�Ĵ�������
	Temp_buf[6] = id;//�ӻ�ID
	Temp_buf[7] = 16;//�������
	Temp_buf[8] = (addr) / 256;//addr head //��ʼ�ĵ�ַ
	Temp_buf[9] = (addr) % 256;

	Temp_buf[10] = 0;//number of addr //д��Ĵ�������Hi
	Temp_buf[11] = 20;//д��Ĵ�������Lo
	Temp_buf[12] = 40;//# of Bytes for values    //һ������byte��ֵ//д�����ݵ��ֽڸ��� 

	Temp_buf[13] = value / 256;//values //����
	Temp_buf[14] = value % 256;

	Temp_buf[15] = value1 / 256;//values1   //��ֽ������λ��
	Temp_buf[16] = value1 % 256;
	Temp_buf[17] = value2 / 256;//values1   //��ֽ������λ��
	Temp_buf[18] = value2 % 256;
	Temp_buf[19] = value5 / 256;//values1   //��ֽ�ŷ�
	Temp_buf[20] = value5 % 256;
	Temp_buf[21] = IsPaper1 / 256;//values1   // ����ֽ
	Temp_buf[22] = IsPaper1 % 256;

	Temp_buf[23] = 0;
	Temp_buf[24] = 0;
	Temp_buf[25] = 0;
	Temp_buf[26] = 0;
	Temp_buf[27] = 0;
	Temp_buf[28] = 0;
	Temp_buf[29] = 0;
	Temp_buf[30] = 0;
	Temp_buf[31] = 0;
	Temp_buf[32] = 0;

	Temp_buf[33] = value3 / 256;//values1   //оֽ������λ��
	Temp_buf[34] = abs(value3) % 256;
	Temp_buf[35] = value4 / 256;//values1   //оֽ������λ��
	Temp_buf[36] = value4 % 256;
	Temp_buf[37] = value6 / 256;//values1   //оֽ�ŷ�
	Temp_buf[38] = value6 % 256;
	Temp_buf[39] = IsPaper2 / 256;//values1   //����ֽ
	Temp_buf[40] = IsPaper2 % 256;

	Temp_buf[41] = 0;
	Temp_buf[42] = 0;
	Temp_buf[43] = 0;
	Temp_buf[44] = 0;
	Temp_buf[45] = 0;
	Temp_buf[46] = 0;
	Temp_buf[47] = 0;
	Temp_buf[48] = 0;
	Temp_buf[49] = 0;
	Temp_buf[50] = 0;
	Temp_buf[51] = 0;
	Temp_buf[52] = 0;
#endif
	
	iErrMsg = SendMsg((char*)Temp_buf, 53);   //�����ķ�����27Ϊ���ĳ��ȣ������ǹ̶���
	CString strmsg;
	strmsg.Format("xxx----Modbus������ɣ�iErr=%d\n", iErrMsg);
	OutputDebugString(strmsg);
	return iErrMsg;
}
#endif // 0
#if 0
int M_Client::Modbus_sender_multi2(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6)  //д20���Ĵ�����ʹ�ù�����16���޸ĺ�Ϳ���д���
{
	OutputDebugString("xxx----Modbus���Ϳ�ʼ \n");
	int iErrMsg = 0;

	unsigned char Temp_buf[31];
	Temp_buf[0] = Ref;
	Temp_buf[1] = 0;
	Temp_buf[2] = 0;
	Temp_buf[3] = 0;
	Temp_buf[4] = 0;//��ID��ʼ�������ֽ���
	Temp_buf[5] = 25;//7+2*�Ĵ�������
	Temp_buf[6] = id;//�ӻ�ID
	Temp_buf[7] = 16;//�������
	Temp_buf[8] = (addr) / 256;//addr head //��ʼ�ĵ�ַ
	Temp_buf[9] = (addr) % 256;

	Temp_buf[10] = 0;//number of addr //д��Ĵ�������Hi
	Temp_buf[11] = 9;//д��Ĵ�������Lo
	Temp_buf[12] = 18;//# of Bytes for values    //һ������byte��ֵ//д�����ݵ��ֽڸ��� 

	Temp_buf[13] = value / 256;//values //����
	Temp_buf[14] = value % 256;

	Temp_buf[15] = value1 / 256;//values1   //��ֽ������λ��
	Temp_buf[16] = value1 % 256;
	Temp_buf[17] = value2 / 256;//values1   //��ֽ������λ��
	Temp_buf[18] = value2 % 256;
	Temp_buf[19] = value5 / 256;//values1   //��ֽ�ŷ�
	Temp_buf[20] = value5 % 256;
	Temp_buf[21] = IsPaper1 / 256;//values1   // ����ֽ
	Temp_buf[22] = IsPaper1 % 256;
	Temp_buf[23] = value3 / 256;//values1   //оֽ������λ��
	Temp_buf[24] = value3 % 256;
	Temp_buf[25] = value4 / 256;//values1   //оֽ������λ��
	Temp_buf[26] = value4 % 256;
	Temp_buf[27] = value6 / 256;//values1   //оֽ�ŷ�
	Temp_buf[28] = value6 % 256;
	Temp_buf[29] = IsPaper2 / 256;//values1   //����ֽ
	Temp_buf[30] = IsPaper2 % 256;

	iErrMsg = SendMsg((char*)Temp_buf, 31);   //�����ķ�����27Ϊ���ĳ��ȣ������ǹ̶���
	CString strmsg;
	strmsg.Format("xxx----Modbus������ɣ�iErr=%d\n", iErrMsg);
	OutputDebugString(strmsg);
	return iErrMsg;


}
#endif

//����ֵ��0-������ȷ��1-���ʹ���
int M_Client::Modbus_sender_multi(int Ref, int addr, int value,int value1,int value2, int value3,int value4,int IsPaper1, int IsPaper2, int IsPaper3, int IsPaper4, int value5,int value6)  //д20���Ĵ�����ʹ�ù�����16���޸ĺ�Ϳ���д���
{
	//unsigned char Temp_buf[25];
	////����
	////����Ԫ��ʶ����һ��ÿ��ͨ��֮�󽫱�Ҫ���1������ͬ��ͨ�����ݱ���
	//Temp_buf[0] = value / 256;//�������ʶ��Hi
	//Temp_buf[1] = value % 256;//�������ʶ��Lo
	////Э���ʶ����00 00ΪmodbusЭ��
	//Temp_buf[2] = 00;
	//Temp_buf[3] = 00;
	////���ݳ��ȣ�����ָʾ���������ݵĳ��ȣ���λ�ֽ�;
	//Temp_buf[4] = 00;//��ID��ʼ�������ֽ���
	//Temp_buf[5] = (7+6);//7+2*�Ĵ�������
	////��Ԫ��ʶ���ӻ�ID�����Ա�ʶ�����ڴ����߻��������ϵ�Զ�̷���˵ĵ�ַ
	//Temp_buf[6] = id;//�ӻ�ID
	////����Ϊ����ͷ
	////�����룬���Ĵ���0x10
	//Temp_buf[7] = 16;//�������
	////��ʼ��ַ
	//Temp_buf[8] = (addr - 1) / 256;//addr head //��ʼ�ĵ�ַ
	//Temp_buf[9] = (addr - 1) % 256;
	////д�Ĵ�������
	//Temp_buf[10] = 0;//number of addr   //��ַ�ĳ���
	//Temp_buf[11] = 3;//д��Ĵ�������Lo
	////д�ֽڵĸ���
	//Temp_buf[12] = 6;//# of Bytes for values    //һ������byte��ֵ 2*�Ĵ�������
	////д�������
	//Temp_buf[13] = value / 256;//values           //�����ֵ��������ֻ��һ���Ĵ�������дһ��ֵ
	//Temp_buf[14] = value % 256;
	//Temp_buf[15] = value1 / 256;//values           //�����ֵ��������ֻ��һ���Ĵ�������дһ��ֵ
	//Temp_buf[16] = value1 % 256;
	//Temp_buf[17] = value2 / 256;//values           //�����ֵ��������ֻ��һ���Ĵ�������дһ��ֵ
	//Temp_buf[18] = value2 % 256;
	//SendMsg((char*)Temp_buf, 19);   //�����ķ�����19Ϊ���ĳ��ȣ������ǹ̶���



	OutputDebugString("xxx----Modbus���Ϳ�ʼ \n");
	int iErrMsg = 0;
	unsigned char Temp_buf[53];
	if (1)
	{
		WORD btDevNo = (WORD)((int)(addr * 1));///��ʼ��ַ

		OutputDebugString("xxx----Modbus����1 \n");
		WORD val0 = (WORD)((int)(value * 1));///����ֵ
		WORD val1 = (WORD)((int)(value1 * 10));///���1
		WORD val2 = (WORD)((int)(value2 * 10));///���2
		WORD val3 = (WORD)((int)(value3 * 10));///���3
		WORD val4 = (WORD)((int)(value4 * 10));///���4
		WORD val5 = (WORD)((int)(value5 * 10));///����1
		WORD val6 = (WORD)((int)(value6 * 10));///����2

		OutputDebugString("xxx----Modbus����2 \n");
		WORD val_tile = (WORD)((IsPaper1 == 1 && IsPaper2 == 1) ? 1 : 0);//��ֽ ����ֵ==0��ʾ��ֽ
		WORD val_core = (WORD)((IsPaper3 == 1 && IsPaper4 == 1) ? 1 : 0);//оֽ

		OutputDebugString("xxx----Modbus����3 \n");
		
		//����
		//����Ԫ��ʶ����һ��ÿ��ͨ��֮�󽫱�Ҫ���1������ͬ��ͨ�����ݱ���
		Temp_buf[0] = (BYTE)(val0 >> 8);//�������ʶ��Hi
		Temp_buf[1] = (BYTE)val0;//�������ʶ��Lo
		//Э���ʶ����00 00ΪmodbusЭ��
		Temp_buf[2] = 0x00;//Э���ʶ��
		Temp_buf[3] = 0x00;
		//���ݳ��ȣ�����ָʾ���������ݵĳ��ȣ���λ�ֽ�;
		Temp_buf[4] = 0x00;//��ID��ʼ�������ֽ���
		Temp_buf[5] = (7 + 40);//7+2*�Ĵ�������

		//��Ԫ��ʶ���ӻ�ID�����Ա�ʶ�����ڴ����߻��������ϵ�Զ�̷���˵ĵ�ַ
		//	Temp_buf[6] = 0x01;//�ӻ���ַ
		Temp_buf[6] = id;//�ӻ���ַ��Ĭ��ֵ0��255
		////����Ϊ����ͷ

		//�����룬���Ĵ���0x10
		Temp_buf[7] = 0x10;//���ܱ�ʶ
			//��ʼ��ַ
		Temp_buf[8] = (BYTE)(btDevNo >> 8);//��ʼ��ַ
		Temp_buf[9] = (BYTE)(btDevNo);//


		Temp_buf[10] = 0;//д��Ĵ�������Hi
		Temp_buf[11] = 20; //д��Ĵ�������Lo
		Temp_buf[12] = 40; //д�����ݵ��ֽڸ��� 
								   ///>����
		Temp_buf[13] = (BYTE)(val0 >> 8);//����
		Temp_buf[14] = (BYTE)val0;
		Temp_buf[15] = (BYTE)(val1 >> 8);//��ֽ������λ��Hi
		Temp_buf[16] = (BYTE)val1;//��ֽ������λ��Lo
		Temp_buf[17] = (BYTE)(val2 >> 8);//��ֽ������λ��
		Temp_buf[18] = (BYTE)val2;
		Temp_buf[19] = (BYTE)(val5 >> 8);//��ֽ�ŷ�
		Temp_buf[20] = (BYTE)val5;
		Temp_buf[21] = (BYTE)(val_tile >> 8);// ����ֽ
		Temp_buf[22] = (BYTE)val_tile;// 
		Temp_buf[23] = 0x00;// 
		Temp_buf[24] = 0x00;// 
		Temp_buf[25] = 0x00;// 
		Temp_buf[26] = 0x00;// 
		Temp_buf[27] = 0x00;// 
		Temp_buf[28] = 0x00;// 
		Temp_buf[29] = 0x00;// 
		Temp_buf[30] = 0x00;// 
		Temp_buf[31] = 0x00;// 
		Temp_buf[32] = 0x00;// 
		Temp_buf[33] = (BYTE)(val3 >> 8);//оֽ������λ��Hi
		Temp_buf[34] = (BYTE)val3;//оֽ������λ��Lo
		Temp_buf[35] = (BYTE)(val4 >> 8); //оֽ������λ��Hi
		Temp_buf[36] = (BYTE)val4;// оֽ������λ��Lo
		Temp_buf[37] = (BYTE)(val6 >> 8); //оֽ�ŷ�Hi
		Temp_buf[38] = (BYTE)val6;// оֽ�ŷ�Lo
		Temp_buf[39] = (BYTE)(val_core >> 8);// ����ֽ
		Temp_buf[40] = (BYTE)val_core;// 
		Temp_buf[41] = 0x00;// 
		Temp_buf[42] = 0x00;// 
		Temp_buf[43] = 0x00;// 
		Temp_buf[44] = 0x00;// 
		Temp_buf[45] = 0x00;// 
		Temp_buf[46] = 0x00;// 
		Temp_buf[47] = 0x00;// 
		Temp_buf[48] = 0x00;// 
		Temp_buf[49] = 0x00;// 
		Temp_buf[50] = 0x00;// 
		Temp_buf[51] = 0x00;// 
		Temp_buf[52] = 0x00;//

	}
	///ƴ�ӱ���
	else
	{
		int val0 = (value * 1);///����ֵ
		int val1 = ((int)(value1 * 10));///���1
		int val2 = ((int)(value2 * 10));///���2
		int val3 = ((int)(value3 * 10));///���3
		int val4 = ((int)(value4 * 10));///���4
		int val5 = ((int)(value5 * 10));///����1
		int val6 = ((int)(value6 * 10));///����2

		OutputDebugString("xxx----Modbus����2 \n");
		int val_tile = ((IsPaper1 == 1 && IsPaper2 == 1) ? 1 : 0);//��ֽ ����ֵ==0��ʾ��ֽ
		int val_core = ((IsPaper3 == 1 && IsPaper4 == 1) ? 1 : 0);//оֽ

		OutputDebugString("xxx----Modbus����3 \n");
		Temp_buf[0] = Ref;
		Temp_buf[1] = 0;
		Temp_buf[2] = 0;
		Temp_buf[3] = 0;
		Temp_buf[4] = 0;//��ID��ʼ�������ֽ���
		Temp_buf[5] = 47;
		Temp_buf[6] = id;//�ӻ�ID
		Temp_buf[7] = 16;//0
		Temp_buf[8] = (addr - 1) / 256;//addr head //��ʼ�ĵ�ַ
		Temp_buf[9] = (addr - 1) % 256;
		Temp_buf[10] = 0;//number of addr   //��ַ�ĳ���
		Temp_buf[11] = 20;
		Temp_buf[12] = 40;//# of Bytes for values    //һ������byte��ֵ
		OutputDebugString("xxx----Modbus����4 \n");

		///>����
		Temp_buf[13] = val0 / 256;//����
		Temp_buf[14] = val0 % 256;
		Temp_buf[15] = val1 / 256; //��ֽ������λ��Hi
		Temp_buf[16] = val1 % 256;//��ֽ������λ��Lo
		Temp_buf[17] = val2 / 256;//��ֽ������λ��
		Temp_buf[18] = val2 % 256;
		Temp_buf[19] = val5 / 256;//��ֽ�ŷ�
		Temp_buf[20] = val5 % 256;
		Temp_buf[21] = val_tile / 256;// ����ֽ
		Temp_buf[22] = val_tile % 256;// 
		Temp_buf[23] = 0;// 
		Temp_buf[24] = 0;// 
		Temp_buf[25] = 0;// 
		Temp_buf[26] = 0;// 
		Temp_buf[27] = 0;// 
		Temp_buf[28] = 0;// 
		Temp_buf[29] = 0;// 
		Temp_buf[30] = 0;// 
		Temp_buf[31] = 0;// 
		Temp_buf[32] = 0;// 
		OutputDebugString("xxx----Modbus����5 \n");
		Temp_buf[33] = val3 / 256; //оֽ������λ��Hi
		Temp_buf[34] = val3 % 256;//оֽ������λ��Lo
		Temp_buf[35] = val4 / 256;  //оֽ������λ��Hi
		Temp_buf[36] = val4 % 256;// оֽ������λ��Lo
		Temp_buf[37] = val6 / 256; //оֽ�ŷ�Hi
		Temp_buf[38] = val6 % 256;// оֽ�ŷ�Lo
		Temp_buf[39] = val_core / 256;// ����ֽ
		Temp_buf[40] = val_core % 256;// 
		Temp_buf[41] = 0;// 
		Temp_buf[42] = 0;// 
		Temp_buf[43] = 0;// 
		Temp_buf[44] = 0;// 
		Temp_buf[45] = 0;// 
		Temp_buf[46] = 0;// 
		Temp_buf[47] = 0;// 
		Temp_buf[48] = 0;// 
		Temp_buf[49] = 0;// 
		Temp_buf[50] = 0;// 
		Temp_buf[51] = 0;// 
		Temp_buf[52] = 0;//
	}

	OutputDebugString("xxx----Modbus����6 \n");
	iErrMsg=SendMsg((char*)Temp_buf, 53);   //�����ķ�����27Ϊ���ĳ��ȣ������ǹ̶���
	OutputDebugString("xxx----Modbus�������\n");
	return iErrMsg;

}
