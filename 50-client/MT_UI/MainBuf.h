#pragma once
#include <stdint.h>
#include <Windows.h>

struct CFrameNode
{
	uint8_t*                        mpDataBuf;			// 媒体数据指针
	uint32_t                        mnDataSize;			// 媒体数据大小
	uint32_t                        mnFrameID;			// 帧标识
	uint32_t                        mnTimeStamp;		// 时间戳
	uint32_t                        mnSSRC;				// 同步源
	union
	{
		struct
		{
			uint32_t	mbKeyFrame;			            // 是否关键帧
			uint16_t	mnWidth;			            // 视频宽度
			uint16_t	mnHeight;			            // 视频高度
		}mtVideoParam;
		uint8_t			mbyAudioMode;		            // 音频模式
	};
	uint8_t				            mbyStreamID;		// 帧ID
	uint8_t                         mbyMediaType;		// 媒体类型
	bool                            mbUsed;             // 可否使用,0：目前未使用，1：已经使用
	bool                            mbNeedFlip;         // 是否翻转，写的时候，如果翻转了，置为1：读操作的时候，判断，完毕后，置为0
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
	// 写入主缓存区
	BOOL WriteMain(const uint8_t* src,uint8_t** dst,uint32_t len);
	// 读取主缓存区
	BOOL ReadMain(const uint8_t* src,uint8_t* dst,uint32_t len);

	BOOL Write(CFrameNode& node);
	BOOL Read(CFrameNode* node);
private:
	uint8_t*                        mpDataBuf;
	uint32_t                        mnDataLen;
	uint32_t                        mnDataWirtePos;         // 读位置
	uint32_t                        mnDataReadPos;          // 写位置
	CRITICAL_SECTION				mcsData;                // 临界区
	CFrameNode                      mQueueFrame[10];
	uint8_t                         mNodeWritePos;          // 写的Node Pos
	uint8_t                         mNodeReadPos;           // 读的Node Pos
	uint8_t*                        mpPackBuf;
	uint32_t                        mWriteAfterReadCount;    // 写数据在读数据之后的统计
	uint8_t                         mnReadCon;    
};

