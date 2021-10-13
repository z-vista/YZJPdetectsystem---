//--------------------------------------------------------------------------------
/**
\file     Fps.h
\brief    CFps��������ļ�
*/
//----------------------------------------------------------------------------------

#ifndef FPS_H
#define FPS_H

#include "StopWatch.h"

//--------------------------------------------------------------------------------
/**
\file     CFps�����ڼ���֡�ʡ�
			
		  CFps���̰߳�ȫ�ġ�
*/
//----------------------------------------------------------------------------------

class CFps
{
public:
	///���캯��
	CFps(void);

	///��������
	~CFps(void);	

    ///��ȡ���һ�ε�֡��
    double GetFps(void);

    ///��ȡ�ۻ�����֡��
    inline size_t GetTotalFrameCount(void); 

	///����֡��
	void IncreaseFrameNum(void);

    ///����֡��
    void UpdateFps(void);

    ///����ʱ���ָ�Ϊ��ʼ״̬
    inline void Reset(void);

private:
	size_t    m_nFrameCount;        ///< ���ϴμ�����Ͽ�ʼ�ۻ���֡��

    double    m_dBeginTime;         ///< ��һ֮֡ǰ��һ֡��ʱ�䣨��ʼΪ0��
    double    m_dEndTime;           ///< ���һ֡��ʱ��

	double    m_dFps;               ///< ���һ�ε�֡��(֡/��)
	size_t    m_nTotalFrameCount;   ///< �ۻ���֡��


	CStopWatch m_objStopWatch;            ///< ��ʱ��
};

//----------------------------------------------------------------------------------
/**
\brief  ��ȡ�ۻ�����֡��
\param  pFrame  ��ǰ֡ͼ��
*/
//----------------------------------------------------------------------------------
inline size_t CFps::GetTotalFrameCount(void) 
{
    return m_nTotalFrameCount;
} 



//----------------------------------------------------------------------------------
/**
\brief  ����ʱ���ָ�Ϊ��ʼ״̬
*/
//----------------------------------------------------------------------------------

inline void CFps::Reset(void)
{
    m_nFrameCount       = 0;
    m_dBeginTime        = 0;
    m_dEndTime          = 0;
    m_nTotalFrameCount  = 0;
    m_dFps              = 0;
}




#endif //FPS_H