﻿
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// prj include
#include "oscbb.h"
#include "common.h"
#include "gb.h"
#include "dialog.h"


void CDialog::free_resource()
{
    if (call_id)
    {
        osip_free(call_id);
        call_id    = NULL;
    }
    if (local_tag)
    {
        osip_free(local_tag);
        local_tag   = NULL;
    }
    if (remote_tag)
    {
        osip_free(remote_tag);
        remote_tag  = NULL;
    }

    item_id             = 0;
    transmitter_id      = 0;
    local_cseq          = 0;                        /**< last local cseq */
    remote_cseq         = 0;                        /**< last remote cseq*/

    ssrc                = 0;
    media_recv_port     = 0;
    ptype               = 0;

    invite_time         = 0;                        /// last invite time,for resend invite request
    start_time          = 0;                        /// the time when this request was happened
	rmt_tl_ip           = 0;
	rmt_tl_port         = 0;
    state               = EDIALOG_STATE_READY;
    next                = NULL;
}

/// 会话
CDialog::CDialog()
{
    call_id             = NULL;
    local_tag           = NULL;
    remote_tag          = NULL;

    dlg_id              = 0;
    item_id             = 0;
    transmitter_id      = 0;
    local_cseq          = 0;                        /**< last local cseq */
    remote_cseq         = 0;                        /**< last remote cseq*/

    ssrc                = 0;
    media_recv_port     = 0;
    ptype               = 0;

    invite_time         = 0;                        /// last invite time,for resend invite request
    start_time          = 0;                        /// the time when this request was happened
	rmt_tl_ip           = 0;
	rmt_tl_port         = 0;
    state               = EDIALOG_STATE_READY;
    next                = NULL;
}

CDialog::~CDialog()
{
    free_resource();
}

/// dialog queue
CDlgQueue::CDlgQueue()            /// construction function
{
    head        = NULL;
    free_head   = NULL;
    queue_len   = 0;
}

CDlgQueue::~CDlgQueue()           /// descontruction function
{
    if (NULL != head) 
    {
        delete [] head;
        head    = NULL;
    }

    free_head   = NULL;
    queue_len   = 0;
}

int CDlgQueue::create(uint32_t len)   /// create queue with len nodes
{
    queue_len   = len;

    CBB_SET_LIMIT(queue_len, 4, 2048);
    try
    {
        head = new CDialog [queue_len];
    }
    catch (...)
    {
        return -ENOMEM;
    }

    free_head   = head;
    uint32_t i = 0;
    for (; i < queue_len - 1; i++)
    {
        head[i].dlg_id  = i;
        head[i].next    = head + i + 1;
    }
    head[i].next    = NULL;
    head[i].dlg_id  = i;

    return 0;
}

CDialog *CDlgQueue::pop()   /// create queue with len nodes
{
    CDialog *ret = NULL;
    ret = free_head;
    if (NULL != free_head)
    {
        free_head   = free_head->next;
    }

    return ret;
}

int CDlgQueue::push(const uint32_t dlg_id)        /// give back one dlg
{
    head[dlg_id].free_resource();
    head[dlg_id].next   = free_head;
    free_head           = head + dlg_id;
    return 0;
}

int CDlgQueue::push(CDialog *dlg)        /// give back one dlg
{
    dlg->free_resource();
    head[dlg->dlg_id].next  = free_head;
    free_head               = dlg;
    return 0;
}


CDialog *CDlgQueue::get_dialog_by_id(uint32_t dlg_id)   /// 获取一个对话
{
    if (dlg_id < queue_len)
    {
        return head + dlg_id;
    }

    return NULL;
}

bool CDialog::invite_match_as_uas(osip_message_t *request)
{
  osip_generic_param_t *tag_param_remote;
  int i;
  char *tmp;

  i = osip_call_id_to_str (request->call_id, &tmp);
  if (i != 0)
    return false;

  if (0 != strcmp (call_id, tmp)) {
    osip_free (tmp);
    return false;
  }
  osip_free (tmp);

  i = osip_from_get_tag (request->from, &tag_param_remote);
  if (0 != i)
  {
      return false;
  }

  if (0 == strcmp (tag_param_remote->gvalue, remote_tag))
  {
      return true;
  }

  return false;
}

/// 判断会话是否匹配
bool CDialog::ack_match_as_uas(osip_message_t *request)
{
  osip_generic_param_t *tag_param_remote;
  int i;
  char *tmp;

  /// step1: call-id match?
  i = osip_call_id_to_str (request->call_id, &tmp);
  if (i != 0)
  {
      return false;
  }

  if (0 != strcmp (call_id, tmp))
  {
      osip_free (tmp);
      return false;
  }
  osip_free (tmp);

  /// step2: rmt_tag match?
  i = osip_from_get_tag (request->from, &tag_param_remote);
  if (0 != i)
  {
      return false;
  }

  if (0 == strcmp (tag_param_remote->gvalue, remote_tag))
  {
      return true;
  }

  /// step3: to_tag_match?
  i = osip_from_get_tag (request->to, &tag_param_remote);
  if (0 != i)
  {
      return false;
  }

  if (0 == strcmp (tag_param_remote->gvalue, local_tag))
  {
      return true;
  }

  return false;
}

/// judge whether dialog match
bool CDialog::invite_res_match_as_uac(char *id, char *tag)
{
    if (call_id && id)
    {
        if (strcmp(call_id, id))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (local_tag && tag)
    {
        if (strcmp(local_tag, tag))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

/// 更新会话
bool CDialog::invite_update_as_uas(osip_message_t *in_msg, uint32_t ip, uint16_t port, uint64_t time)
{
    if (EDIALOG_STATE_INVITING != state)
    {
        return false;
    }

    /// get seq
    uint32_t this_seq = osip_atoi(in_msg->cseq->number);
    if (this_seq <= remote_cseq)
    {
        return true;
    }
    remote_cseq     = this_seq;

    /// get other param
    invite_time     = time;        /// last invite time,for resend invite request
    rmt_tl_ip       = ip;
    rmt_tl_port     = port;

    /// set state
    state           = EDIALOG_STATE_INVITING;

    return true;
}

/// set call-id
bool CDialog::set_call_id(char *id)
{
    if (call_id)
    {
        osip_free(call_id);
    }
    call_id = osip_strdup(id);
    return true;
}

/// add remote tag
bool CDialog::set_remote_tag(char *tag)
{
    if (remote_tag)
    {
        osip_free(remote_tag);
    }

    remote_tag  = osip_strdup(tag);

    return true;
}

/// 增加本地tag
bool CDialog::set_local_tag(char *tag)
{
    if (local_tag)
    {
        osip_free(local_tag);
    }

    local_tag   = osip_strdup(tag);

    return true;
}

/// judge dialog whether match
bool CDialog::dlg_get_param(char **id, char **l_tag, char **r_tag)
{
  if (id)
  {
      *id   = osip_strdup(call_id);
  }

  if (l_tag)
  {
      *l_tag    = osip_strdup(local_tag);
  }

  if (r_tag)
  {
      *r_tag    = osip_strdup(remote_tag);
  }

  return true;
}

/// 增加会话
bool CDialog::invite_add_as_uac(osip_message_t *in_msg, uint32_t ip, uint16_t port, uint64_t time, uint32_t it_id)
{
    item_id         = it_id;            /// GB item id

    /// get call_id
    int ret = osip_call_id_to_str (in_msg->call_id, &call_id);
    if (0 != ret)
    {
        return false;
    }

    /// get remote tag and local tag
    local_tag       = NULL;
    remote_tag      = NULL;
    osip_generic_param_t *tag;
    ret = osip_from_get_tag(in_msg->from, &tag);
    if (0 == ret)
    {
        remote_tag = osip_strdup(tag->gvalue);
    }

    /// get seq
    remote_cseq     = osip_atoi(in_msg->cseq->number);
    local_cseq      = 0;

    /// get other param
    invite_time     = start_time    = time;        /// last invite time,for resend invite request
    rmt_tl_ip       = ip;
    rmt_tl_port     = port;

    /// set state
    state           = EDIALOG_STATE_INVITING;
    next            = NULL;

    return true;
}


