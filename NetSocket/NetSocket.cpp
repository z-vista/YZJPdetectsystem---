// NetSocket.cpp : implementation file
#include "stdafx.h"
#include "NetSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NetSocket

NetSocket::NetSocket()
{
	m_state = STATE_DISCONNECTED;
	m_pfCallback = NULL;
}

NetSocket::~NetSocket()
{
}

void NetSocket::SetCallback(PFNSOCKCALLBACK pfProc)
{
	m_pfCallback = pfProc;
}

int NetSocket::GetLinkState()
{
	return m_state;
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(NetSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(NetSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// NetSocket member functions
BOOL NetSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
	if(!CAsyncSocket::Connect(lpszHostAddress, nHostPort))
	{
		int err = GetLastError();
		if(err != WSAEWOULDBLOCK)
		{
			TRACE("Connect to server error: %d.\n", GetLastError());
			return FALSE;
		}
	}
	m_state = STATE_CONNECTING;
	return TRUE;
}

void NetSocket::OnClose(int nErrorCode)
{
	if(m_pfCallback)
		m_pfCallback(MSG_CLOSED, nErrorCode, 0);
	else{
		TRACE("NetSocket callback not be set\n");
	}
	m_state = STATE_DISCONNECTED;
	CAsyncSocket::OnClose(nErrorCode);
}


void NetSocket::OnConnect(int nErrorCode)
{
	if(m_pfCallback)
	{
		if(nErrorCode == 0)
		{
			m_pfCallback(MSG_CONNECTED, 0, 0);
			m_state = STATE_CONNECTED;
		}
		else{
			m_pfCallback(MSG_CONNECT_FAILED, nErrorCode, 0);
			m_state = STATE_DISCONNECTED;
		}
	}
	else{
		TRACE("NetSocket callback not be set\n");
	}
	CAsyncSocket::OnConnect(nErrorCode);
}


void NetSocket::OnSend(int nErrorCode)
{
	CAsyncSocket::OnSend(nErrorCode);
}


void NetSocket::OnReceive(int nErrorCode)
{
	if(m_pfCallback)
		m_pfCallback(MSG_DATA_RECEIVED, nErrorCode, 0);
	else{
		TRACE("NetSocket callback not be set\n");
	}
	CAsyncSocket::OnReceive(nErrorCode);
}
