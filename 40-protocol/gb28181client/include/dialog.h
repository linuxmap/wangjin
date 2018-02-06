#ifndef __DIALOG_H__
#define __DIALOG_H__


// exosip/osip include
#include <osip2/osip.h>


/**
* Structure for referencing a dialog.
* @struct osip_dialog
*/

/// fsm
enum EDialogState
{
	EDIALOG_STATE_READY,
	EDIALOG_STATE_INVITING,
	EDIALOG_STATE_INVITING_RESPONSED,
	EDIALOG_STATE_INVITED,
	EDIALOG_STATE_BYING,
};

/// one dialog referenced by call_id,local_tag and remote tag
struct CDialog
{
	uint32_t        dlg_id;             /// 会话ID
	uint32_t        item_id;            /// GB item id
	uint32_t        transmitter_id;            /// GB item id

	char            *call_id;
	char            *local_tag;
	char            *remote_tag;

	uint32_t        local_cseq;         /**< last local cseq */
	uint32_t        remote_cseq;        /**< last remote cseq*/

	uint64_t        invite_time;        /// last invite time,for resend invite request
	uint64_t        start_time;         /// the time when this request was happened

    uint32_t        ssrc;
    uint16_t        media_recv_port;
    uint8_t         ptype;

	uint32_t        rmt_tl_ip;
	uint16_t        rmt_tl_port;
	EDialogState    state;

	CDialog         *next;

	/// 是否匹配
	bool            invite_match_as_uas(osip_message_t *in_msg);

	/// 更新会话
	bool            invite_update_as_uas(osip_message_t *in_msg, uint32_t rmt_tl_ip, uint16_t rmt_tl_port, uint64_t time);

	/// 增加会话
	bool            invite_add_as_uac(osip_message_t *in_msg, uint32_t rmt_tl_ip, uint16_t rmt_tl_port, uint64_t time, uint32_t it_id);

	/// 判断会话是否匹配
	bool            ack_match_as_uas(osip_message_t *in_msg);

    /// judge whether dialog match
    bool            invite_res_match_as_uac(char *call_id, char *local_tag);

	/// 增加本地tag
	bool            set_local_tag(char *tag);

    /// set call-id
    bool            set_call_id(char *call_id);

    /// add remote tag
    bool            set_remote_tag(char *tag);

    /// judge dialog whether match
    bool            dlg_get_param(char **id, char **l_tag, char **r_tag);

    /// free current node resource,exclude @dlg_id
    void            free_resource();

	/// 构造
	CDialog();
	~CDialog();
};

/// dialog queue
class CDlgQueue
{
public:
	CDlgQueue();            /// construction function
	~CDlgQueue();           /// descontruction function

	int create(uint32_t len);   /// create queue with len nodes

	CDialog *pop();    /// add or update one dlg

	int push(const uint32_t dlg_id);        /// give back one dlg

	int push(CDialog *dlg);        /// give back one dlg

	CDialog *get_dialog_by_id(uint32_t dlg_id);   /// 获取一个对话

												  /// 记录所有会话
	void show_all(int(*log_handler)(const char* format, ...));

private:
	CDialog     *head;              /// head for all node
	CDialog     *free_head;         /// free node

	uint32_t    queue_len;
};


#endif  /// __DIALOG_H__
