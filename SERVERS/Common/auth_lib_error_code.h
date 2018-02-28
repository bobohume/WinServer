
#ifndef _AUTH_LIB_ERROR_CODE_H_
#define _AUTH_LIB_ERROR_CODE_H_

// 网络层错误码
#define ERROR_OK                             0   /* 成功 */
#define ERROR_UNSUPPORTED_PROTOCOL           101 /* 不支持的协议 */
#define ERROR_UNSUPPORTED_PROTOCOL_VERSION   102 /* 不兼容的协议版本 */
#define ERROR_CHECKSUM                       103 /* 校验和错误 */
#define ERROR_ENCRYPTED                      104 /* 加密错误 */
#define ERROR_DECRYPTED                      105 /* 解密错误 */
#define ERROR_DISCONNECTED                   106 /* 网络连接中断 */
#define ERROR_DISPATCH                       107 /* 消息转发错误 */

#define ERROR_INVALID_SERVICE                151 /* 不合法的服务 */
#define ERROR_INVOKE_SERVICE                 152 /* 调用服务错误 */
#define ERROR_INVALID_SITE                   153 /* 未知的站点 */

#define ERROR_OVERLOAD                       9998 /* 系统繁忙 */
#define ERROR_UNKNOWN                        9999 /* 未知错误 */

// 库内错误码
#define ERROR_PENDING                        901  /* 请求正在处理中 */
#define ERROR_NO_RESULT   ERROR_PENDING           /* 暂时没有返回结果 */


// 业务错误码
#define ERROR_MEM                            1002 /* 内存分配错误 */
#define ERROR_OUT_SERVICE                    1003 /* 调用不存在服务 */
#define ERROR_UNKOWN_TXN                     1005 /* 操作码不存在 */
#define ERROR_PARA                           1006 /* 参数错误 */
#define ERROR_FILE                           1007 /* 操作文件失败 */
#define ERROR_IN_OVERFLOW                    1008 /* 输入消息越界 */
#define ERROR_MEMCACHED                      1011 /*更新memcached错误*/

#define ERROR_NO_PROTECT_METHOD              9000 /*用户没有绑定安全保护*/
#define ERROR_TIMESTAMP_EXPIRED              9001 /*时间戳已过期*/
#define ERROR_BAN_FOR_PASSWD_ERROR           9002 /*用户密码出错次数过多而被锁定*/
#define ERROR_GET_BASIC_FROM_DB              9003 /*从数据库获取用户基本信息失败*/
#define ERROR_ERROR_PASSWD                   9004 /*密码错误*/
#define ERROR_GEN_TICKET                     9005 /*生成ticket出错*/
#define ERROR_GET_PROTECT_STATUS             9006 /*获取用户安全保护状态出错*/
#define ERROR_PASSWD_DECRYPT                 9007 /*密码解密失败*/
#define ERROR_MD5                            9008 /*MD5失败*/
#define ERROR_LOGINNAME_NOT_EXIST            9009 /*用户名不存在*/
#define ERROR_ERROR_TICKET                   9010 /*ticket错误*/
#define ERROR_GAME_NOT_ACTIVE                9011 /*用户没有激活游戏区*/
#define ERROR_GAME_PARTITION_BAN             9012 /*游戏区被封停*/
#define ERROR_TICKET_EXPIRED                 9013 /*ticket过期*/
#define ERROR_INVALID_CERT_ID                9014 /*错误的身份证格式*/
#define ERROR_CACHE_SET                      9015 /*写cache失败*/
#define ERROR_FCM_CANT_ACTIVE_MORE_PARTITION 9016 /*防沉迷用户只能激活一款游戏的一个大区*/
#define ERROR_NO_MATRIX_SERVICE              9017 /*用户没有开通密保卡服务*/
#define ERROR_GET_GAME_CONFIG                9018 /*用户游戏配置失败*/
#define ERROR_INVALID_SITECD                 9019 /*非法的游戏*/
#define ERROR_INVALID_GAME_PARTIITON_SERVER  9020 /*非法的游戏区服*/
#define ERROR_ERROR_MATRIX_CODE              9021 /*密保卡动态位置验证失败*/
#define ERROR_NO_TOKEN_SERVICE               9022 /*用户没有开通令牌服务*/
#define ERROR_MATRIX_NOT_EXIST               9023 /*密保卡不存在*/
#define ERROR_TOKEN_NOT_EXIST                9024 /*令牌不存在*/
#define ERROR_TOKEN_CODE                     9025 /*令牌动态口令验证失败*/
#define ERROR_MATRIX_GEN_CODE                9026 /*生成密保卡动态位置错误*/
#define ERROR_YAUTH_INIT                     9027 /*YAuth_init初始化失败*/
#define ERROR_MUTEX_LOCK                     9028 /*线程lock错误*/
#define ERROR_MUTEX_UNLOCK                   9029 /*线程unlock错误*/
#define ERROR_VERIFY_YY_TOKEN                9030 /*验证YYtoken失败*/
#define ERROR_INSERT_YYUSER                  9031 /*插入新的yy用户错误*/
#define ERROR_PASSWD_ENCRYPT                 9032 /*密码加密失败*/

#endif //_AUTH_LIB_ERROR_CODE_H_

