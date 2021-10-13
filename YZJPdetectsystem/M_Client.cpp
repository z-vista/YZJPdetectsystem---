
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

	//用于记录错误信息并输出
	int iErrMsg;
	//启动WinSock
	WSAData wsaData;
	iErrMsg = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (iErrMsg != NO_ERROR)
		//有错误
	{
		printf("failed with wsaStartup error : %d\n", iErrMsg);

		rlt = 1;
		return rlt;
	}

	//创建Socket
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET)
		//创建Socket失败
	{
		printf("socket failed with error : %d\n", WSAGetLastError());

		rlt = 2;
		return rlt;
	}

	//目标服务器数据
	sockaddr_in sockaddrServer;
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_port = htons(port);
	sockaddrServer.sin_addr.s_addr = inet_addr(address);

	//连接,sock与目标服务器连接
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



//返回值是1，说明发送错误
int M_Client::SendMsg(const char* msg, int len)
{
	int rlt = 0;

	int iErrMsg = 0;

	//发送消息，指定sock发送消息
	iErrMsg = send(m_sock, msg, len, 0);
	if (iErrMsg < 0)
		//发送失败
	{
		printf("send msg failed with error : %d\n", iErrMsg);

		rlt = 1;
		return rlt;
	}

	return rlt;

}

//返回值是1，说明接收错误
int M_Client::RecvMsg(char* msg, int len)
{
	int rlt = 0;

	int iErrMsg = 0;

	//buffer必须清空
	memset(msg, 0, len);
	//发送消息，指定sock发送消息
	iErrMsg = recv(m_sock, msg, len, 0);
	if (iErrMsg < 0)
		//发送失败
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

void M_Client::Modbus_recv_single(int Ref, int addr, int value)  //读一个寄存器，使用功能码16，修改后就可以写多个
{
	unsigned char Temp_buf[15];
	RecvMsg((char*)Temp_buf, 15);

	//while (size > 0)//剩余部分大于0
	//{
	//	int RecvSize = recv(sock, buffer, size, 0);
	//	if (SOCKET_ERROR == RecvSize)
	//		return false;
	//	size = size - RecvSize;
	//	buffer += RecvSize;
	//}
	//return true;


}

void M_Client::Modbus_sender_single(int Ref, int addr, int value)  //写一个寄存器，使用功能码16，修改后就可以写多个
{
	unsigned char Temp_buf[20];
	Temp_buf[0] = Ref;
	Temp_buf[1] = 0;
	Temp_buf[2] = 0;
	Temp_buf[3] = 0;
	Temp_buf[4] = 0;//从ID开始到最后的字节数
	Temp_buf[5] = 9;
	Temp_buf[6] = id;//从机ID
	Temp_buf[7] = 16;//0
	Temp_buf[8] = (addr - 1) / 256;//addr head //开始的地址
	Temp_buf[9] = (addr - 1) % 256;
	Temp_buf[10] = 0;//number of addr   //地址的长度
	Temp_buf[11] = 1;
	Temp_buf[12] = 2;//# of Bytes for values    //一共多少byte的值
	Temp_buf[13] = value / 256;//values           //具体的值，这里我只改一个寄存器，就写一个值
	Temp_buf[14] = value % 256;
	SendMsg((char*)Temp_buf, 15);   //将报文发出，15为报文长度，这里是固定的
}

#if 1
int M_Client::Modbus_sender_multi2(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6)  //写20个寄存器，使用功能码16，修改后就可以写多个
{
	OutputDebugString("xxx----Modbus发送开始 \n");
	int iErrMsg = 0;

	unsigned char Temp_buf[53];

	WORD btDevNo = (WORD)((int)(addr * 1));///起始地址

	OutputDebugString("xxx----Modbus发送1 \n");
	WORD val0 = (WORD)((int)(value * 1));///心跳值
	WORD val1 = (WORD)((int)(value1));///相机1
	WORD val2 = (WORD)((int)(value2));///相机2
	WORD val3 = (WORD)((int)(value3));///相机3
	WORD val4 = (WORD)((int)(value4));///相机4
	WORD val5 = (WORD)((int)(value5));///幅宽1
	WORD val6 = (WORD)((int)(value6));///幅宽2

	OutputDebugString("xxx----Modbus发送2 \n");
	WORD val_tile = (WORD)(IsPaper1);//瓦纸 返回值==0表示无纸
	WORD val_core = (WORD)(IsPaper2);//芯纸

	OutputDebugString("xxx----Modbus发送3 \n");

	//报文
	//事务元标识符，一般每次通信之后将被要求加1以区别不同的通信数据报文
	Temp_buf[0] = (BYTE)(Ref >> 8);//事务处理标识符Hi
	Temp_buf[1] = (BYTE)Ref;//事务处理标识符Lo
	//协议标识符，00 00为modbus协议
	Temp_buf[2] = 0x00;//协议标识符
	Temp_buf[3] = 0x00;
	//数据长度，用来指示接下来数据的长度，单位字节;
	Temp_buf[4] = 0x00;//从ID开始到最后的字节数
	Temp_buf[5] = (7 + 40);//7+2*寄存器数量

	//单元标识符从机ID，用以标识连接在串行线或者网络上的远程服务端的地址
	//	Temp_buf[6] = 0x01;//从机地址
	Temp_buf[6] = id;//从机地址，默认值0或255
	////以上为报文头

	//功能码，读寄存器0x10
	Temp_buf[7] = 0x10;//功能标识
		//起始地址
	Temp_buf[8] = (BYTE)(btDevNo >> 8);//起始地址
	Temp_buf[9] = (BYTE)(btDevNo);//


	Temp_buf[10] = 0;//写入寄存器数量Hi
	Temp_buf[11] = 20; //写入寄存器数量Lo
	Temp_buf[12] = 40; //写入数据的字节个数 
							   ///>数据
	Temp_buf[13] = (BYTE)(val0 >> 8);//心跳
	Temp_buf[14] = (BYTE)val0;
	Temp_buf[15] = (BYTE)(val1 >> 8);//瓦纸操作侧位置Hi
	Temp_buf[16] = (BYTE)val1;//瓦纸操作侧位置Lo
	Temp_buf[17] = (BYTE)(val2 >> 8);//瓦纸驱动侧位置
	Temp_buf[18] = (BYTE)val2;
	Temp_buf[19] = (BYTE)(val5 >> 8);//瓦纸门幅
	Temp_buf[20] = (BYTE)val5;
	Temp_buf[21] = (BYTE)(val_tile >> 8);// 有无纸
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
	Temp_buf[33] = (BYTE)(val3 >> 8);//芯纸操作侧位置Hi
	Temp_buf[34] = (BYTE)val3;//芯纸操作侧位置Lo
	Temp_buf[35] = (BYTE)(val4 >> 8); //芯纸驱动侧位置Hi
	Temp_buf[36] = (BYTE)val4;// 芯纸驱动侧位置Lo
	Temp_buf[37] = (BYTE)(val6 >> 8); //芯纸门幅Hi
	Temp_buf[38] = (BYTE)val6;// 芯纸门幅Lo
	Temp_buf[39] = (BYTE)(val_core >> 8);// 有无纸
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
	Temp_buf[4] = 0;//从ID开始到最后的字节数
	Temp_buf[5] = 47;//7+2*寄存器数量
	Temp_buf[6] = id;//从机ID
	Temp_buf[7] = 16;//命令代码
	Temp_buf[8] = (addr) / 256;//addr head //开始的地址
	Temp_buf[9] = (addr) % 256;

	Temp_buf[10] = 0;//number of addr //写入寄存器数量Hi
	Temp_buf[11] = 20;//写入寄存器数量Lo
	Temp_buf[12] = 40;//# of Bytes for values    //一共多少byte的值//写入数据的字节个数 

	Temp_buf[13] = value / 256;//values //心跳
	Temp_buf[14] = value % 256;

	Temp_buf[15] = value1 / 256;//values1   //瓦纸操作侧位置
	Temp_buf[16] = value1 % 256;
	Temp_buf[17] = value2 / 256;//values1   //瓦纸驱动侧位置
	Temp_buf[18] = value2 % 256;
	Temp_buf[19] = value5 / 256;//values1   //瓦纸门幅
	Temp_buf[20] = value5 % 256;
	Temp_buf[21] = IsPaper1 / 256;//values1   // 有无纸
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

	Temp_buf[33] = value3 / 256;//values1   //芯纸操作侧位置
	Temp_buf[34] = abs(value3) % 256;
	Temp_buf[35] = value4 / 256;//values1   //芯纸驱动侧位置
	Temp_buf[36] = value4 % 256;
	Temp_buf[37] = value6 / 256;//values1   //芯纸门幅
	Temp_buf[38] = value6 % 256;
	Temp_buf[39] = IsPaper2 / 256;//values1   //有无纸
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
	
	iErrMsg = SendMsg((char*)Temp_buf, 53);   //将报文发出，27为报文长度，这里是固定的
	CString strmsg;
	strmsg.Format("xxx----Modbus发送完成，iErr=%d\n", iErrMsg);
	OutputDebugString(strmsg);
	return iErrMsg;
}
#endif // 0
#if 0
int M_Client::Modbus_sender_multi2(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6)  //写20个寄存器，使用功能码16，修改后就可以写多个
{
	OutputDebugString("xxx----Modbus发送开始 \n");
	int iErrMsg = 0;

	unsigned char Temp_buf[31];
	Temp_buf[0] = Ref;
	Temp_buf[1] = 0;
	Temp_buf[2] = 0;
	Temp_buf[3] = 0;
	Temp_buf[4] = 0;//从ID开始到最后的字节数
	Temp_buf[5] = 25;//7+2*寄存器数量
	Temp_buf[6] = id;//从机ID
	Temp_buf[7] = 16;//命令代码
	Temp_buf[8] = (addr) / 256;//addr head //开始的地址
	Temp_buf[9] = (addr) % 256;

	Temp_buf[10] = 0;//number of addr //写入寄存器数量Hi
	Temp_buf[11] = 9;//写入寄存器数量Lo
	Temp_buf[12] = 18;//# of Bytes for values    //一共多少byte的值//写入数据的字节个数 

	Temp_buf[13] = value / 256;//values //心跳
	Temp_buf[14] = value % 256;

	Temp_buf[15] = value1 / 256;//values1   //瓦纸操作侧位置
	Temp_buf[16] = value1 % 256;
	Temp_buf[17] = value2 / 256;//values1   //瓦纸驱动侧位置
	Temp_buf[18] = value2 % 256;
	Temp_buf[19] = value5 / 256;//values1   //瓦纸门幅
	Temp_buf[20] = value5 % 256;
	Temp_buf[21] = IsPaper1 / 256;//values1   // 有无纸
	Temp_buf[22] = IsPaper1 % 256;
	Temp_buf[23] = value3 / 256;//values1   //芯纸操作侧位置
	Temp_buf[24] = value3 % 256;
	Temp_buf[25] = value4 / 256;//values1   //芯纸驱动侧位置
	Temp_buf[26] = value4 % 256;
	Temp_buf[27] = value6 / 256;//values1   //芯纸门幅
	Temp_buf[28] = value6 % 256;
	Temp_buf[29] = IsPaper2 / 256;//values1   //有无纸
	Temp_buf[30] = IsPaper2 % 256;

	iErrMsg = SendMsg((char*)Temp_buf, 31);   //将报文发出，27为报文长度，这里是固定的
	CString strmsg;
	strmsg.Format("xxx----Modbus发送完成，iErr=%d\n", iErrMsg);
	OutputDebugString(strmsg);
	return iErrMsg;


}
#endif

//返回值：0-发送正确。1-发送错误
int M_Client::Modbus_sender_multi(int Ref, int addr, int value,int value1,int value2, int value3,int value4,int IsPaper1, int IsPaper2, int IsPaper3, int IsPaper4, int value5,int value6)  //写20个寄存器，使用功能码16，修改后就可以写多个
{
	//unsigned char Temp_buf[25];
	////报文
	////事务元标识符，一般每次通信之后将被要求加1以区别不同的通信数据报文
	//Temp_buf[0] = value / 256;//事务处理标识符Hi
	//Temp_buf[1] = value % 256;//事务处理标识符Lo
	////协议标识符，00 00为modbus协议
	//Temp_buf[2] = 00;
	//Temp_buf[3] = 00;
	////数据长度，用来指示接下来数据的长度，单位字节;
	//Temp_buf[4] = 00;//从ID开始到最后的字节数
	//Temp_buf[5] = (7+6);//7+2*寄存器数量
	////单元标识符从机ID，用以标识连接在串行线或者网络上的远程服务端的地址
	//Temp_buf[6] = id;//从机ID
	////以上为报文头
	////功能码，读寄存器0x10
	//Temp_buf[7] = 16;//命令代码
	////起始地址
	//Temp_buf[8] = (addr - 1) / 256;//addr head //开始的地址
	//Temp_buf[9] = (addr - 1) % 256;
	////写寄存器数量
	//Temp_buf[10] = 0;//number of addr   //地址的长度
	//Temp_buf[11] = 3;//写入寄存器数量Lo
	////写字节的个数
	//Temp_buf[12] = 6;//# of Bytes for values    //一共多少byte的值 2*寄存器数量
	////写入的数据
	//Temp_buf[13] = value / 256;//values           //具体的值，这里我只改一个寄存器，就写一个值
	//Temp_buf[14] = value % 256;
	//Temp_buf[15] = value1 / 256;//values           //具体的值，这里我只改一个寄存器，就写一个值
	//Temp_buf[16] = value1 % 256;
	//Temp_buf[17] = value2 / 256;//values           //具体的值，这里我只改一个寄存器，就写一个值
	//Temp_buf[18] = value2 % 256;
	//SendMsg((char*)Temp_buf, 19);   //将报文发出，19为报文长度，这里是固定的



	OutputDebugString("xxx----Modbus发送开始 \n");
	int iErrMsg = 0;
	unsigned char Temp_buf[53];
	if (1)
	{
		WORD btDevNo = (WORD)((int)(addr * 1));///起始地址

		OutputDebugString("xxx----Modbus发送1 \n");
		WORD val0 = (WORD)((int)(value * 1));///心跳值
		WORD val1 = (WORD)((int)(value1 * 10));///相机1
		WORD val2 = (WORD)((int)(value2 * 10));///相机2
		WORD val3 = (WORD)((int)(value3 * 10));///相机3
		WORD val4 = (WORD)((int)(value4 * 10));///相机4
		WORD val5 = (WORD)((int)(value5 * 10));///幅宽1
		WORD val6 = (WORD)((int)(value6 * 10));///幅宽2

		OutputDebugString("xxx----Modbus发送2 \n");
		WORD val_tile = (WORD)((IsPaper1 == 1 && IsPaper2 == 1) ? 1 : 0);//瓦纸 返回值==0表示无纸
		WORD val_core = (WORD)((IsPaper3 == 1 && IsPaper4 == 1) ? 1 : 0);//芯纸

		OutputDebugString("xxx----Modbus发送3 \n");
		
		//报文
		//事务元标识符，一般每次通信之后将被要求加1以区别不同的通信数据报文
		Temp_buf[0] = (BYTE)(val0 >> 8);//事务处理标识符Hi
		Temp_buf[1] = (BYTE)val0;//事务处理标识符Lo
		//协议标识符，00 00为modbus协议
		Temp_buf[2] = 0x00;//协议标识符
		Temp_buf[3] = 0x00;
		//数据长度，用来指示接下来数据的长度，单位字节;
		Temp_buf[4] = 0x00;//从ID开始到最后的字节数
		Temp_buf[5] = (7 + 40);//7+2*寄存器数量

		//单元标识符从机ID，用以标识连接在串行线或者网络上的远程服务端的地址
		//	Temp_buf[6] = 0x01;//从机地址
		Temp_buf[6] = id;//从机地址，默认值0或255
		////以上为报文头

		//功能码，读寄存器0x10
		Temp_buf[7] = 0x10;//功能标识
			//起始地址
		Temp_buf[8] = (BYTE)(btDevNo >> 8);//起始地址
		Temp_buf[9] = (BYTE)(btDevNo);//


		Temp_buf[10] = 0;//写入寄存器数量Hi
		Temp_buf[11] = 20; //写入寄存器数量Lo
		Temp_buf[12] = 40; //写入数据的字节个数 
								   ///>数据
		Temp_buf[13] = (BYTE)(val0 >> 8);//心跳
		Temp_buf[14] = (BYTE)val0;
		Temp_buf[15] = (BYTE)(val1 >> 8);//瓦纸操作侧位置Hi
		Temp_buf[16] = (BYTE)val1;//瓦纸操作侧位置Lo
		Temp_buf[17] = (BYTE)(val2 >> 8);//瓦纸驱动侧位置
		Temp_buf[18] = (BYTE)val2;
		Temp_buf[19] = (BYTE)(val5 >> 8);//瓦纸门幅
		Temp_buf[20] = (BYTE)val5;
		Temp_buf[21] = (BYTE)(val_tile >> 8);// 有无纸
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
		Temp_buf[33] = (BYTE)(val3 >> 8);//芯纸操作侧位置Hi
		Temp_buf[34] = (BYTE)val3;//芯纸操作侧位置Lo
		Temp_buf[35] = (BYTE)(val4 >> 8); //芯纸驱动侧位置Hi
		Temp_buf[36] = (BYTE)val4;// 芯纸驱动侧位置Lo
		Temp_buf[37] = (BYTE)(val6 >> 8); //芯纸门幅Hi
		Temp_buf[38] = (BYTE)val6;// 芯纸门幅Lo
		Temp_buf[39] = (BYTE)(val_core >> 8);// 有无纸
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
	///拼接报文
	else
	{
		int val0 = (value * 1);///心跳值
		int val1 = ((int)(value1 * 10));///相机1
		int val2 = ((int)(value2 * 10));///相机2
		int val3 = ((int)(value3 * 10));///相机3
		int val4 = ((int)(value4 * 10));///相机4
		int val5 = ((int)(value5 * 10));///幅宽1
		int val6 = ((int)(value6 * 10));///幅宽2

		OutputDebugString("xxx----Modbus发送2 \n");
		int val_tile = ((IsPaper1 == 1 && IsPaper2 == 1) ? 1 : 0);//瓦纸 返回值==0表示无纸
		int val_core = ((IsPaper3 == 1 && IsPaper4 == 1) ? 1 : 0);//芯纸

		OutputDebugString("xxx----Modbus发送3 \n");
		Temp_buf[0] = Ref;
		Temp_buf[1] = 0;
		Temp_buf[2] = 0;
		Temp_buf[3] = 0;
		Temp_buf[4] = 0;//从ID开始到最后的字节数
		Temp_buf[5] = 47;
		Temp_buf[6] = id;//从机ID
		Temp_buf[7] = 16;//0
		Temp_buf[8] = (addr - 1) / 256;//addr head //开始的地址
		Temp_buf[9] = (addr - 1) % 256;
		Temp_buf[10] = 0;//number of addr   //地址的长度
		Temp_buf[11] = 20;
		Temp_buf[12] = 40;//# of Bytes for values    //一共多少byte的值
		OutputDebugString("xxx----Modbus发送4 \n");

		///>数据
		Temp_buf[13] = val0 / 256;//心跳
		Temp_buf[14] = val0 % 256;
		Temp_buf[15] = val1 / 256; //瓦纸操作侧位置Hi
		Temp_buf[16] = val1 % 256;//瓦纸操作侧位置Lo
		Temp_buf[17] = val2 / 256;//瓦纸驱动侧位置
		Temp_buf[18] = val2 % 256;
		Temp_buf[19] = val5 / 256;//瓦纸门幅
		Temp_buf[20] = val5 % 256;
		Temp_buf[21] = val_tile / 256;// 有无纸
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
		OutputDebugString("xxx----Modbus发送5 \n");
		Temp_buf[33] = val3 / 256; //芯纸操作侧位置Hi
		Temp_buf[34] = val3 % 256;//芯纸操作侧位置Lo
		Temp_buf[35] = val4 / 256;  //芯纸驱动侧位置Hi
		Temp_buf[36] = val4 % 256;// 芯纸驱动侧位置Lo
		Temp_buf[37] = val6 / 256; //芯纸门幅Hi
		Temp_buf[38] = val6 % 256;// 芯纸门幅Lo
		Temp_buf[39] = val_core / 256;// 有无纸
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

	OutputDebugString("xxx----Modbus发送6 \n");
	iErrMsg=SendMsg((char*)Temp_buf, 53);   //将报文发出，27为报文长度，这里是固定的
	OutputDebugString("xxx----Modbus发送完成\n");
	return iErrMsg;

}
