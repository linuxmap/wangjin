#include "recinfo.h"

/// 2 exp
const int CRecordInfoList::MAX_NODE_COUNT = 16;

/// construction
CRecordInfoList::CRecordInfoList()
{
    head        = NULL;
    current     = NULL;
    tail        = NULL;
    list_len    = 0;
}

/// deconstruction
CRecordInfoList::~CRecordInfoList()
{
    current = NULL;
    tail = NULL;
    list_len = 0;

    CNode *it = head;
    while (it) {
        CNode *tmp = it->next;
        delete it;
        if (tmp == NULL)
            break;
        it = tmp;
    }

    head = NULL;
}

/// 在链表中找相同的sn；找到就返回；找不到就用头插的方式
int CRecordInfoList::insert_ctx_sn(void * ctx, int sn)
{
    if (list_len >= MAX_NODE_COUNT)
    {
        return GB_ERR_AGAIN;
    }

    /// if query exist
    if (find_node_by_sn(sn) != NULL)
    {
        return GB_SUCCESS;
    }

    CNode *node;
    try
    {
        node = new CNode;
    }
    catch ( ... )
    {
        return GB_ERR_NOMEM;
    }

    node->set_ctx_sn(ctx, sn);
    node->next  = head;
    head        = node;
    list_len++;                     /// 链表长度+1
    
    return GB_SUCCESS;
}

void CRecordInfoList::delete_ctx(int in_sn)
{
    /// if list empty
    if (NULL == head)
    {
        return;
    }

    /// if the first node match
    CNode *pre = head;
    if (head->sn == in_sn)
    {
        head    = head->next;
        delete pre;
        list_len--;
        return;
    }

    /// not the first
    CNode *tmp = head->next;
    while (tmp)
    {
        if (tmp->sn == in_sn)
        {
            pre->next = tmp->next;
            delete tmp;
            list_len--;
            return;
        }

        pre = tmp;
        tmp = tmp->next;
    }
}


void CRecordInfoList::printf_all()
{
    CNode *node = head;
    while( node )
    {
        printf("	:          %d\n",node->sn);
        GB::CGbRecordItem *item = node->item;
        while( item )
        {
            printf("address:==========%s\n",item->address);
            printf("device_id:========%s\n",item->device_id);
            printf("file_path:========%s\n",item->file_path);
            printf("name:=============%s\n",item->name);
            printf("secrecy:==========%d\n",item->secrecy);
#ifndef _MSC_VER
            printf("end_time:=========%ld\n",item->end_time);
            printf("start_time:=======%ld\n",item->start_time);
#else
            printf("end_time:=========%lld\n",item->end_time);
            printf("start_time:=======%lld\n",item->start_time);
#endif
            printf("type:=============%d\n",item->type);
            item = item->next;
        }
        node = node->next;
    }
    printf("---------------------------------\n\n\n");
}

int CRecordInfoList::add_record_info(int sn, CRecordInfoItem *item, CNode **out_node)
{
    CNode *node = find_node_by_sn(sn);
    if ( node == NULL )
    {
        return GB_ERR_ARG;
    }

    /// set out node
    if (out_node)
    {
        *out_node   = node;
    }

    return node->add_record_info_item(item);
}

void CRecordInfoList::sort_by_start_time( int sn )
{
    CNode *node = find_node_by_sn( sn );
    if (node == NULL)
        return;
    node->sort_by_start_time();
}

void CRecordInfoList::set_id_sn(int id, int sn) 
{
    CNode *node = find_node_by_sn(sn);
    if (node == NULL)
        return;
    node->set_id( id );
}

/*
 * find specified node by ctx and sn
 */
CRecordInfoList::CNode *CRecordInfoList::find_node(void *in_ctx, int in_sn)
{
    CNode *node = head;
    while(node)
    {
        if ( (node->sn == in_sn) && (in_ctx == node->ctx) )
        {
            return node;
        }
        node = node->next;
    }

	return NULL;
}

CRecordInfoList::CNode * CRecordInfoList::find_node_by_sn(int in_sn)
{
    CNode *node = head;
    while(node)
    {
        if(node->sn == in_sn)
        {
            return node;
        }
        node = node->next;
    }

	return NULL;
}

CRecordInfoList::CNode::CNode()
{
    id              = -1;
    ctx             = NULL;
    sn              = 0;
    sum_num         = 0;
    current_sum_num = 0;
    item            = NULL;
    next            = NULL;
    last_item       = NULL;

}

CRecordInfoList::CNode::~CNode()
{
    id = -1;
    ctx = NULL;
    sn = 0;
    sum_num = 0;
    current_sum_num = 0;
    next = NULL;

    last_item = NULL;

    GB::CGbRecordItem *it = this->item;
    while ( it ) {
        GB::CGbRecordItem *tmp = it->next;
        delete it;
        if( tmp == NULL )
          break;
        it = tmp;
    }

    item = NULL;
}

void CRecordInfoList::CNode::set_ctx_sn(void *in_ctx, int in_sn)
{
    ctx = in_ctx;
    sn  = in_sn;
}

void CRecordInfoList::CNode::set_sum_num(int sum_num)
{
    this->sum_num = sum_num;
}

int CRecordInfoList::CNode::add_record_info_item(CRecordInfoItem *rec_item)
{
    CRecordInfoItem * tmp = rec_item;
    GB::CGbRecordItem * it;
    while (tmp)
    {
        it = (GB::CGbRecordItem *)malloc(sizeof(GB::CGbRecordItem));
        if (NULL == it)
        {
            return GB_ERR_NOMEM;
        }
        memset(it, 0, sizeof(GB::CGbRecordItem));

        strncpy(it->address, tmp->address, GBSIP_GENERAL_ITEM_LEN);
        strncpy(it->device_id, tmp->device_id, GBSIP_USERNAME_LEN);
        strncpy(it->file_path, tmp->file_path, GBSIP_GENERAL_ITEM_LEN);
        strncpy(it->name, tmp->name, GBSIP_GENERAL_ITEM_LEN);
        it->start_time  = tmp->start_time;
        it->end_time    = tmp->end_time;
        it->secrecy     = tmp->secrecy;
        it->type        = tmp->type;
        it->next        = NULL;
        current_sum_num++;

        if (this->item == NULL)
        {
            this->item = it;
            last_item = it;
        }
        else
        {
            last_item->next = it;
            last_item = it;
        }
        tmp = tmp->next;
    }

    return GB_SUCCESS;
}


void CRecordInfoList::CNode::value_copy_to(GB::CGbRecordItem *a, GB::CGbRecordItem *b) //b的值复制给a
{
    strncpy(a->address, b->address, MN_GENERAL_ITEM_LEN);
    strncpy(a->device_id, b->device_id, MN_DEV_ID_LEN);
    strncpy(a->file_path, b->file_path, MN_GENERAL_ITEM_LEN);
    strncpy(a->name, b->name, MN_GENERAL_ITEM_LEN);
    a->start_time = b->start_time;
    a->end_time = b->end_time;
    a->secrecy = b->secrecy;
    a->type = b->type;
}

void CRecordInfoList::CNode::sort_by_start_time()
{
    GB::CGbRecordItem *p = item;
    if( p == NULL )
      return;

    GB::CGbRecordItem *q;
    GB::CGbRecordItem tmp;
    for( ; p != NULL; p = p->next )
    {
        for( q = p->next; q != NULL; q = q->next )
        {
            if( p->start_time > q->start_time )
            {
                value_copy_to(&tmp, p);
                value_copy_to(p, q);
                value_copy_to(q, &tmp);
            }
        }
    }
}

void CRecordInfoList::CNode::set_id(int in_id)
{
    id = in_id;
}
