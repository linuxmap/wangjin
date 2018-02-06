
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#include "oscbb.h"

#include "common.h"

#include "reg.h"

#define GBSIP_REG_UNAUTH_MAX_NUM    1024
#define GBSIP_REG_UNAUTH_MIN_NUM    32 

/// 构造函数
CGbUnauthReg::CGbUnauthReg()
{
    queue       = NULL;
    queue_len   = 0;
}

/// 析构函数
CGbUnauthReg::~CGbUnauthReg()
{
    delete [] queue;
    queue       = NULL;
    queue_len   = 0;
}

/// 创建,参数为队列长度
int CGbUnauthReg::create(uint32_t len)
{
    if ( (0 != queue_len) || (NULL != queue) )
    {
        return -EBUSY;
    }

    /// 需要对入参进行区间限制,防止无限制大的数据
    CBB_SET_LIMIT(len, GBSIP_REG_UNAUTH_MAX_NUM, GBSIP_REG_UNAUTH_MIN_NUM);

    try
    {
        queue   = new CNode[len];
    }
    catch (...)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "reg queue create failed, queue len:%u\n", len);
        queue   = NULL;
        return -ENOMEM;
    }

    queue_len   = len;

    return 0;
}


/// 增加消息到注册队列,timeout为超时时间,单位:ms
int CGbUnauthReg::add(osip_message_t *msg, uint64_t current_time, uint32_t timeout_ms)
{
    int ret;

    for (uint32_t index = 0; index < queue_len; index++)
    {
        if (NULL == queue[index].msg)
        {
            ret = osip_message_clone(msg, &(queue[index].msg));
            if (OSIP_SUCCESS != ret)
            {
                CLog::log(CLog::CLOG_LEVEL_GB_TASK, "reg queue add failed, maybe no mem,ret:%d\n", ret);
                queue[index].msg    = NULL;
                return -ENOMEM;
            }

            queue[index].birth_time = current_time;
            queue[index].timeout    = timeout_ms;

            return 0;
        }
    }

    return -EAGAIN;
}

namespace
{
#include <osipparser2/osip_md5.h>
#define HASHLEN 16
typedef char HASH[HASHLEN];

#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

void CvtHex(HASH Bin, HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
          Hex[i * 2] = (j + '0');
        else
          Hex[i * 2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
          Hex[i * 2 + 1] = (j + '0');
        else
          Hex[i * 2 + 1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void digest_calc_ha1(const char *pszAlg,
                    const char *pszUserName, unsigned int username_len,
                    const char *pszRealm, unsigned int realm_len,
                    const char *pszPassword, unsigned int passwd_len,
                    const char *pszNonce, const char *pszCNonce, HASHHEX SessionKey)
{
    osip_MD5_CTX Md5Ctx;
    HASH HA1;
    HASHHEX HA1Hex;

    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszUserName, username_len);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszRealm, realm_len);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszPassword, passwd_len);
    osip_MD5Final ((unsigned char *) HA1, &Md5Ctx);
    if ((pszAlg != NULL) && osip_strcasecmp (pszAlg, "md5-sess") == 0) {
        CvtHex(HA1, HA1Hex);
        osip_MD5Init (&Md5Ctx);
        osip_MD5Update(&Md5Ctx, (unsigned char *)HA1Hex, HASHHEXLEN);
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, (unsigned int) strlen (pszCNonce));
        osip_MD5Final ((unsigned char *) HA1, &Md5Ctx);
    }
    CvtHex (HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void digest_calc_response(HASHHEX HA1,     /* H(A1) */
                    const char *pszNonce, unsigned int nonce_len,    /* nonce from server */
                    const char *pszNonceCount,       /* 8 hex digits */
                    const char *pszCNonce,   /* client nonce */
                    const char *pszQop,      /* qop-value: "", "auth", "auth-int" */
                    int Aka, /* Calculating AKAv1-MD5 response */
                    const char *pszMethod,   /* method from the request */
                    const char *pszDigestUri, unsigned int uri_len,        /* requested URL */
                    HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
                    HASHHEX Response /* request-digest or response-digest */ )
{
    osip_MD5_CTX Md5Ctx;
    HASH HA2;
    HASH RespHash;
    HASHHEX HA2Hex;

    /* calculate H(A2) */
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszMethod, (unsigned int) strlen (pszMethod));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszDigestUri, uri_len);

    if (pszQop == NULL) {
        goto auth_withoutqop;
    }
    else if (0 == osip_strcasecmp (pszQop, "auth-int")) {
        goto auth_withauth_int;
    }
    else if (0 == osip_strcasecmp (pszQop, "auth")) {
        goto auth_withauth;
    }

auth_withoutqop:
    osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
    CvtHex (HA2, HA2Hex);

    /* calculate response */
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHHEXLEN);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, nonce_len);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);

    goto end;

auth_withauth_int:

    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HEntity, HASHHEXLEN);

auth_withauth:
    osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
    CvtHex (HA2, HA2Hex);

    /* calculate response */
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHHEXLEN);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    if (Aka == 0) {
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonceCount, (unsigned int) strlen (pszNonceCount));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, (unsigned int) strlen (pszCNonce));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszQop, (unsigned int) strlen (pszQop));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    }
end:
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA2Hex, HASHHEXLEN);
    osip_MD5Final ((unsigned char *) RespHash, &Md5Ctx);
    CvtHex (RespHash, Response);
}
}

/// 检查是否匹配，若存在并匹配返回成功
bool CGbUnauthReg::match(osip_message_t *msg)
{
    return true;
}


/// 检查认证信息
bool CGbUnauthReg::check_alg(osip_message_t *msg)
{
    /// 数字认证核心代码,需要注意的是剥离osip库解析后带来的引号
    osip_authorization_t *auth = (osip_authorization_t *)msg->authorizations.node->element;
    
    HASHHEX  ha1;
    HASHHEX  response;

#define key_value_len(key)  \
    char *key = auth->key;  \
    if ('"' == key[0])    \
    {                       \
        key++;   \
    }   \
    unsigned int v_len_##key = strlen(key);   \
    if ( ('"' == key[v_len_##key - 1]) && (v_len_##key > 1) )    \
    {                               \
        v_len_##key--;         \
    }

    key_value_len(username);
    key_value_len(realm);

    digest_calc_ha1("md5", username, v_len_username, realm, v_len_realm, "123456", strlen("123456"), auth->nonce, NULL, ha1);

    key_value_len(nonce);
    key_value_len(uri);
    digest_calc_response(ha1, nonce, v_len_nonce, NULL, NULL, NULL, 0, msg->sip_method, uri, v_len_uri, NULL, response);

    char *rmt_response = auth->response;
    if ('"' == rmt_response[0])
    {
        rmt_response++;
    }

    /// 认证成功,返回正确的结果
    if (0 == strncasecmp(response, rmt_response, 32))
    {
        return true;
    }

    return false;
}

/// 检查是否存在该消息登记
bool CGbUnauthReg::check_exist(osip_message_t *msg, uint32_t &out_index)
{
    for (uint32_t index = 0; index < queue_len; index++)
    {
        if (NULL != queue[index].msg)
        {
            if ( (OSIP_SUCCESS == osip_from_tag_match(msg->from, queue[index].msg->from))
                    && (OSIP_SUCCESS == osip_call_id_match(msg->call_id, queue[index].msg->call_id)) )
            {
                /// get matched msg
                /// 这里还需要增加对服务器字段的判断，防止服务器被攻击
                out_index   = index; 
                return true;
            }
        }
    }

    return false;
}


/// 调度器
void CGbUnauthReg::update(uint64_t cur_time)
{
    for (uint32_t index = 0; index < queue_len; index++)
    {
        if (NULL != queue[index].msg)
        {
            /// 将超时注册会话信息删除掉
            if (queue[index].birth_time + queue[index].timeout < cur_time)
            {
                osip_message_free(queue[index].msg);
                queue[index].msg            = NULL;
                queue[index].birth_time     = 0;
                queue[index].timeout        = 0;
            }
        }
    }
}


/// reg queue 构造函数
CGbUnauthReg::CNode::CNode()
{
    msg         = NULL;
    birth_time  = 0;
    timeout     = 0;
}

/// 析构函数,主要释放msg
CGbUnauthReg::CNode::~CNode()
{
    if (msg != NULL)
    {
        osip_message_free(msg);
    }
    msg         = NULL;
    birth_time  = 0;
    timeout     = 0;
}
