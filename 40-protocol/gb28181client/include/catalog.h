
#ifndef __GB_CATALOG_H__
#define __GB_CATALOG_H__

// exosip/osip include
#include <osip2/osip.h>

/// dec
struct TQueryCatalogItem;
struct CDialog;
class CDlgQueue;


/// 目录条目信息
class CGbCatalog
{
public:
	/// 构造函数
	CGbCatalog();

	/// 析构函数
	~CGbCatalog();

	/// 最大注册资源条目数量
	int create(uint32_t num = 1024);

    void destroy();

	/// 增加注册
	int add_res_item(TQueryCatalogItem &item, uint32_t parent_id, uint32_t ip, uint16_t port, uint64_t time);

	/// 增加媒体请求
	int add_rmt_invite_dialog(osip_message_t *msg, uint32_t rmt_tl_ip, uint16_t rmt_tl_port, uint64_t time,
		uint32_t &dev_id, uint32_t &item_id, uint32_t &dlg_id);

	/// 增加媒体请求应答
	int add_rmt_invite_dialog(uint32_t item_id, uint32_t dlg_id, char *to_tag);

	/// 增加媒体请求应答
	int add_rmt_invite_dialog(uint32_t item_id, uint32_t dlg_id, uint32_t transmitter_id, osip_message_t *msg);

	/// local invite media
    int add_local_invite_dialog(char *sip_id, uint32_t &item_id, uint32_t &dev_id, uint32_t &dlg_id);

    /// local invite media
    int add_local_invite_dialog(char *sip_id, uint32_t &item_id, uint32_t &dev_id, uint32_t &dlg_id,
                uint32_t ssrc, uint16_t port, uint8_t ptype);

	/// local invite media
    int add_local_invite_res_dialog(char *sip_id, char *call_id, char *local_tag, char *rmt_tag);

    /// delete local invite dialog
    int del_local_invite_dialog(uint32_t item_id, uint32_t dlg_id);

    /// delete local invite dialog
    int del_local_invite_dialog(char *sip_id, char **call_id, char **local_tag, char **rmt_tag,
                uint32_t ssrc, uint16_t port, uint8_t ptype);
    
	/// set local invite media param 
    int set_dialog_param(uint32_t dlg_id, char *rmt_tag, char *local_tag, char *call_id);

	/// 增加媒体请求应答
	bool find_rmt_ack_dialog(osip_message_t *msg, uint32_t &transmitter_id);

	/// 调度
	void shedule(uint64_t cur_time);

	// ev for callback
	enum event_type_t
	{
		EV_TYPE_DLG_OVERTIME = 1,            /// 会话超时
		EV_TYPE_DLG_RESEND = 1,            /// 会话重传
	};

	/// 事件回调
	typedef int(*FEvCb)(void *context, int type, void *buf, int len, uint64_t time);

	/// 设置事件回调
	void set_ev_cb(FEvCb cb, void *context);

	/// 记录所有会话
	void show_dlg(int(*log_handler)(const char* format, ...));

private:
	/// 资源条目节点信息
	class CNode
	{
	public:
		/// 构造函数
		CNode();

		/// 析构函数
		~CNode();

		/// 释放资源
		void free_resource();

		//// manscdp param
		char            sip_id[21];
		char            name[64];
		char            manufacturer[32];
		char            model[32];
		char            owner[32];
		char            civil_code[32];
		char            address[32];
		int             parental;
		int             safe_way;
		int             register_way;
		int             secrecy;
		int             status;

		bool            used;
		uint32_t        parent_dev_id;
		uint32_t        item_id;

		uint64_t        last_update_time;
		uint32_t        dev_ip;             /// 对端通信IP,网络序
		uint16_t        dev_port;           /// 对端通信端口,网络序

		CDialog         *local_caller;      /// 本地邀请对话,应该只有一个
		CDialog         *rmt_caller;        /// 本地邀请对话,可能有多个

		CNode           *next;

		/// 调度当前节点
		void shedule(uint64_t cur_time);

		/// 更新会话
		bool rmt_caller_update_dlg(osip_message_t *msg);
	};

	uint32_t                    max_item_num;    /// 最大注册设备数量
	uint32_t                    cur_item_num;    /// 当前注册设备数量
	CNode                       *catalog_item_queue;



	CDlgQueue                   *dlg_queue;
	uint32_t                    max_dialog_num;    /// 当前注册设备数量

												   /// 增加条目
	int _add(TQueryCatalogItem &item, uint32_t parent_id, uint32_t ip, uint16_t port, uint64_t time);

	/// 刷新条目信息
	int update(uint32_t item_id, TQueryCatalogItem &item, uint32_t parent_id, uint32_t ip, uint16_t port, uint64_t time);

	/// 条目是否存在
	bool item_exist(char *id, uint32_t &item_index);

};



#endif  // __GB_CATALOG_H__
