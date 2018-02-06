#pragma once
#include <stdint.h>
#include <Windows.h>

struct CFrameNode
{
	uint8_t*                        mpDataBuf;			// ý������ָ��
	uint32_t                        mnDataSize;			// ý�����ݴ�С
	uint32_t                        mnFrameID;			// ֡��ʶ
	uint32_t                        mnTimeStamp;		// ʱ���
	uint32_t                        mnSSRC;				// ͬ��Դ
	union
	{
		struct
		{
			uint32_t	mbKeyFrame;			            // �Ƿ�ؼ�֡
			uint16_t	mnWidth;			            // ��Ƶ���
			uint16_t	mnHeight;			            // ��Ƶ�߶�
		}mtVideoParam;
		uint8_t			mbyAudioMode;		            // ��Ƶģʽ
	};
	uint8_t				            mbyStreamID;		// ֡ID
	uint8_t                         mbyMediaType;		// ý������
	bool                            mbUsed;             // �ɷ�ʹ��,0��Ŀǰδʹ�ã�1���Ѿ�ʹ��
	bool                            mbNeedFlip;         // �Ƿ�ת��д��ʱ�������ת�ˣ���Ϊ1����������ʱ���жϣ���Ϻ���Ϊ0
};

class CMainBuf
{
public:
	CMainBuf()
	{
		Init();
	}

	~CMainBuf()
	{
		Uninit();
	}
private:
	BOOL Init();
	void Uninit();
	BOOL ValidArea(uint32_t pos1,uint32_t pos2);
public:
	// д����������
	BOOL WriteMain(const uint8_t* src,uint8_t** dst,uint32_t len);
	// ��ȡ��������
	BOOL ReadMain(const uint8_t* src,uint8_t* dst,uint32_t len);

	BOOL Write(CFrameNode& node);
	BOOL Read(CFrameNode* node);
private:
	uint8_t*                        mpDataBuf;
	uint32_t                        mnDataLen;
	uint32_t                        mnDataWirtePos;         // ��λ��
	uint32_t                        mnDataReadPos;          // дλ��
	CRITICAL_SECTION				mcsData;                // �ٽ���
	CFrameNode                      mQueueFrame[10];
	uint8_t                         mNodeWritePos;          // д��Node Pos
	uint8_t                         mNodeReadPos;           // ����Node Pos
	uint8_t*                        mpPackBuf;
	uint32_t                        mWriteAfterReadCount;    // д�����ڶ�����֮���ͳ��
	uint8_t                         mnReadCon;    
};

