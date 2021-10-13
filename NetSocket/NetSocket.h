#ifndef _NETSOCKET_H
#define _NETSOCKET_H

#include <winsock2.h>
#include <afxsock.h>

typedef void (* PFNSOCKCALLBACK)(int msg, int param, void *lpobj);

class NetSocket : public CAsyncSocket
{
public:
	NetSocket();
	virtual ~NetSocket();

	void SetCallback(PFNSOCKCALLBACK pfProc);
	int  GetLinkState();

	enum{
		STATE_CONNECTING     = 1,
		STATE_CONNECTED      = 2,
		STATE_DISCONNECTED   = 3,

		MSG_CONNECTED		 = 1,
		MSG_CONNECT_FAILED   = 2,
		MSG_DATA_RECEIVED    = 3,
		MSG_CLOSED           = 4,
	};

public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMysocket)
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CMysocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
	BYTE            m_state;
	PFNSOCKCALLBACK m_pfCallback;

	// Overrides
public:
	BOOL Connect(LPCTSTR lpszHostAddress, UINT nHostPort);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};

//////////////////////
#endif
