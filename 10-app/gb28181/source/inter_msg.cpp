

#if 0
/// deal income msg
void CAdpGbParam::deal_msg()
{
    int ret = 0;

    // check msg len 
    if (recv_len < (int)sizeof(recv_msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gb adp,invalid msg len:%d\n", recv_len);
        return;
    }

    // check data len
    memcpy(&recv_msg, buf, sizeof(recv_msg));
    if (recv_msg.var_len + (int)sizeof(recv_msg) != recv_len)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mc,invalid msg len:%d, var len:%d\n", recv_len, recv_msg.var_len);
        return;
    }

    recv_msg.var    = buf + sizeof(recv_msg);

    CLog::log(CLog::CLOG_LEVEL_API, "[GB]deal income msg, msg_type:%d\n", recv_msg.type);
    switch (recv_msg.type)
    {
        case MN_GB_INVITE_MEDIA:
            {
                MN::CMediaInvite invite_param;
                if (recv_msg.var_len != sizeof(invite_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&invite_param, recv_msg.var, sizeof(invite_param));
                    ret = GB::gb_server_invite(handle, invite_param);
                }
            }
            break;

        case MN_GB_LOCAL_RES_CATALOG:
            {
                MN::CRmtQueryCatalog query_catalog_param;
                if ( (recv_msg.var_len < (int)sizeof(query_catalog_param))
                     || (((recv_msg.var_len - sizeof(query_catalog_param)) % sizeof(MN::CCatalogItem)) != 0) )
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&query_catalog_param, recv_msg.var, sizeof(query_catalog_param));
                    ret = GB::gb_server_reponse_catalog(handle, query_catalog_param, (uint8_t *)recv_msg.var + sizeof(CRmtQueryCatalog),
                                recv_msg.var_len - sizeof(query_catalog_param));
                }
            }
            break;

        case MN_GB_RMT_INVITE_MEDIA:
            {
                MN::CMediaInvite invite_param;
                if (recv_msg.var_len != sizeof(invite_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&invite_param, recv_msg.var, sizeof(invite_param));
                    ret = GB::gb_server_rmt_invite(handle, invite_param);
                }
            }
            break;

        default:
            break;
    }

    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gb adp deal msg failed:%d\n", ret);
    }
}
#endif
