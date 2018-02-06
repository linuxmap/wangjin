#ifndef __REC_INFO_H__
#define __REC_INFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmlparser.h"
#include "gb.h"

class CRecordInfoList
{
public:
    /// recorder response item node
    class CNode
    {
    public:
        int id;
        void * ctx;
        int sn;
        int sum_num;
        int current_sum_num;
        GB::CGbRecordItem *item;
        CNode * next;

        GB::CGbRecordItem * last_item;

        /// construction & deconstruction
        CNode();
        ~CNode();

        void set_ctx_sn(void * ctx, int sn);

        void set_sum_num(int sum_num);

        int add_record_info_item(CRecordInfoItem *item);

        /// src的值复制给dst
        void value_copy_to(GB::CGbRecordItem *dst, GB::CGbRecordItem *src);

        void sort_by_start_time();

        void set_id(int in_id);
    };

    CRecordInfoList();
    ~CRecordInfoList();

    int insert_ctx_sn(void * ctx, int sn);

    /// delete specified query by ctx and sn
    void delete_ctx(int sn);

    int add_record_info(int sn, CRecordInfoItem *item, CNode **out_node = NULL);

    void printf_all();

    CNode *find_node_by_sn(int sn); 

    /*
     * find specified node by ctx and sn
     */
    CNode *find_node(void *ctx, int sn); 

    void sort_by_start_time( int sn );

    void set_id_sn( int id, int sn);
    
    void shedule(uint64_t time);

private:
    CNode *head;
    CNode *current;
    CNode *tail;

    int list_len;

    const static int MAX_NODE_COUNT;
};

#endif  /// __REC_INFO_H__
