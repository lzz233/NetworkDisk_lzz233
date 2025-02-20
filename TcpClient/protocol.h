#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef unsigned int uint; // 用无符号整形是因为长度不会为负数

#define REGIST_OK "regist ok" // 注册
#define REGIST_FAILED "regist fail: name existed"

#define LOGIN_OK "login ok" // 登录
#define LOGIN_FAILED "login failed"

#define SEARCH_USR_NO "no such people" // 搜索用户功能
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOWN_ERROR "unknown error" // 通用未知错误

#define ADD_FRIEND_OK "add friend ok" // 添加好友 0已是好友，1对方存在，2不存在，-1未知错误
#define ADD_FRIEND_EMPTY "no such people"
#define ADD_FRIEND_EXIST "added friend already exist"

#define AGREE_OK "add friend request ok" // 同意加好友
#define AGREE_FAILED "add friend request fail"

#define DELETE_OK "delete friend request ok" // 删好友
#define DELETE_FAILED "delete friend request fail"

#define DELETE_DIR_OK "delete request ok" // 删文件/文件夹
#define DELETE_DIR_FAILED "delete request fail"

#define RENAME_FILE_OK "rename file ok"                  // 重命名文件
#define RENAME_FILE_FAILED "rename file failed"

#define ENTRY_DIR_OK "entry dir ok"                      // 进入目录
#define ENTRY_DIR_FAILED "entry dir failed"

#define PRE_DIR_OK "return pre dir ok"                   // 上一目录
#define PRE_DIR_FAILED "return pre dir failed"

#define UPLOAD_FILE_OK "upload file ok"                  // 上传文件
#define UPLOAD_FILE_FAILED "upload file failed"
#define UPLOAD_FILE_START "start upload file data"

#define DOWNLOAD_FILE_OK "download file ok"              // 下载文件
#define DOWNLOAD_FILE_FAILED "download file failed"
#define DOWNLOAD_FILE_START "start download file data"

#define MOVE_FILE_OK "move file ok"                      // 移动文件
#define MOVE_FILE_FAILED "move file failed"

#define SHARE_FILE_OK "share file ok"                      // 移动文件
#define SHARE_FILE_FAILED "share file failed"

enum ENUM_MSG_TYPE{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST, // 注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND, // 注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST, // 登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND, // 登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST, // 查看所有在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND, // 查看所有在线用户回复

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST, // 搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND, // 搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST, // 添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND, // 添加好友回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE, // 同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE, // 不同意添加好友

    ENUM_MSG_TYPE_FRESH_FRIEND_REQUEST, // 刷新好友列表请求
    ENUM_MSG_TYPE_FRESH_FRIEND_RESPOND, // 刷新好友列表回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST, // 删除好友列表请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND, // 删除好友列表回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, // 私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND, // 私聊回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST, // 群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND, // 群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST, // 创建路径请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND, // 创建路径回复

    ENUM_MSG_TYPE_FRESH_FLIE_REQUEST, // 刷新文件请求
    ENUM_MSG_TYPE_FRESH_FLIE_RESPOND, // 刷新文件回复

    ENUM_MSG_TYPE_DELETE_DIR_REQUEST, // 删除路径请求
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND, // 删除路径回复

    ENUM_MSG_TYPE_RENAME_FLIE_REQUEST, // 重命名文件请求
    ENUM_MSG_TYPE_RENAME_FLIE_RESPOND, // 重命名文件回复

    ENUM_MSG_TYPE_ENTRY_DIR_REQUEST, // 进入文件夹请求
    ENUM_MSG_TYPE_ENTRY_DIR_RESPOND, // 进入文件夹回复

    ENUM_MSG_TYPE_PRE_DIR_REQUEST, // 上一文件夹请求
    ENUM_MSG_TYPE_PRE_DIR_RESPOND, // 上一文件夹回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST, // 上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND, // 上传文件回复

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST, // 下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND, // 下载文件响应

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST, // 移动文件请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND, // 移动文件响应

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST, // 移动文件请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND, // 移动文件响应
    ENUM_MSG_TYPE_SHARE_FILE_NOTE, // 移动文件提示
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND, // 移动文件提示响应


    ENUM_MSG_TYPE_MAX = 0x00ffffff, // uint最大值 0xffffffff
};

struct FileInfo
{
    char caFileName[32]; // 文件名字
    int iFileType; // 文件类型
};

struct PDU{
    uint uiPDULen; // 总的协议数据大小（此次发送多少数据）（ui代表数据类型）
    uint uiMsgType; // 消息类型
    char caData[64]; // ca代表数据类型，char array。用来放用户名和密码
    uint uiMsgLen; // 实际消息长度
    int caMsg[]; // 实际消息。柔性数组，实际长度根据 uiMsgLen 决定
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
