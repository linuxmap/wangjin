#ifndef __NCPROXY_PROTOCOL_H__ 
#define __NCPROXY_PROTOCOL_H__ 

#define NCPROXY_ANALYSIS_INVALID_HANDLE     NULL

typedef void *      HNcproxyAnalysis;


typedef enum tagNcproxyProtMainCmd{
    
    ENCPROXY_MAIN_CMD_SYS_QUERY     = 1,    // query NVR sys status
    ENCPROXY_MAIN_CMD_SYS_SET       = 2,    // set NVR sys param

    ENCPROXY_MAIN_CMD_NET_QUERY     = 3,    // query NVR net status
    ENCPROXY_MAIN_CMD_NET_SET       = 4,    // set NVR param

    ENCPROXY_MAIN_CMD_AVPARAM_QUERY = 5,    // query NVR video/audio param
    ENCPROXY_MAIN_CMD_AVPARAM_SET   = 6,    // set NVR video/audio param

    ENCPROXY_MAIN_CMD_EVENT_QUERY   = 7,    // query NVR event param
    ENCPROXY_MAIN_CMD_EVENT_SET     = 8,    // set NVR event param

    ENCPROXY_MAIN_CMD_REC_QUERY     = 9,    // query NVR record param
    ENCPROXY_MAIN_CMD_REC_SET       = 10,    // set NVR record param

    ENCPROXY_MAIN_CMD_LOGIN_REQ     = 11,    // LOGIN request
    ENCPROXY_MAIN_CMD_LOGOUT_REQ    = 12,    // LOGOUT request

    ENCPROXY_MAIN_CMD_UPDATE_NC     = 13,    //  update NVR client
    ENCPROXY_MAIN_CMD_KEEPALIVE     = 14,    // linker keepalive


    ENCPROXY_MAIN_CMD_DEVLIST       = 100,   // all registered device query
    ENCPROXY_MAIN_CMD_GBDEVLIST     = 101,   // GB28181 device query

    ENCPROXY_MAIN_CMD_AVSTREAM      = 200,   // play/stop av stream 

}ENcproxyProtMainCmd;

typedef enum tagNcproxyProtDevlistSubCmd{
    ENCPROXY_SUB_CMD_GET_DEVLIST        = 1,   // get device list

}tagNcproxyProtDevlistSubCmd;

/*
 * cmd def
 */
typedef struct tagNcproxyCmd
{
    uint16_t    main_cmd;
    uint16_t    sub_cmd;
}TNcproxyCmd;



/// analysis event callback
typedef int (*FNcproxyAnalysisEvCb)(void *context, TNcproxyCmd *cmd, void *buf, int len);


/// send/recv data call back function
typedef int (*FNcproxyAnalysisDataCb)(void *context, void *buf, int len);

/*
 * create analysis client or server
 *@param type this param can either be ANALYSIS_TYPE_CLIENT or ANALYSIS_TYPE_SERVER
 *@return if successfull,return handle. On error @ANALYSIS_INVALID_HANDLE is returned
 */
HNcproxyAnalysis ncproxy_analysis_create();




/*
 * destroy analysis client or server
 */
int ncproxy_analysis_destroy(HNcproxyAnalysis handle);

/*
 * data send/recv for specified handle, if recv_cb not NULL, this is for pull mode
 */
void ncproxy_analysis_set_cb(HNcproxyAnalysis handle, FNcproxyAnalysisEvCb cb, void *ev_cb_context);


/*
 * analysis data input,push mode
 */
int ncproxy_analysis_input_data(HNcproxyAnalysis handle, void *buf, int len);


#endif  // __NCPROXY_PROTOCOL_H__ 
