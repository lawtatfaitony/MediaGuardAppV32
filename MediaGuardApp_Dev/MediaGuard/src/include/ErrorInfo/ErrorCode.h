/**************************************************************
* @brief:       error of camera-guard
* @date:         20200123
* @copyright:
*
***************************************************************/
#pragma once



#define CP_OK                           0
#define CP_UNKNOW_ERROR                 1
#define CP_INVALID_JSON                 2
#define CP_INVALID_PARA                 3
#define CP_EXCUTE_SQL_ERROR             4
#define CP_INVALID_SESSION              5
#define CP_NOT_EXIST                    6
#define CP_INVAOKE_TIMEOUT              7
#define CP_INVALID_DATABASE_CONN        8




#define CP_COMPARE_NOT_CONNECT          50
#define CP_INVALID_COMPARE_INSTANCE     51
#define CP_TASK_SERVER_NOT_CONNECT      52
#define CP_INVALID_TASK_SERVER_INSTANCE 53

// user
#define CP_INVALID_USER                 100
#define CP_USER_ALREADY_EXIST           101
#define CP_PASSWORD_ERROR               102

// library
#define CP_LIBRARY_ALREADY_EXIST        200
#define CP_LIBRARY_NOT_EXIST            201
#define CP_ADD_LIBRARY_COMPARE_FAILED   202
#define CP_ADD_LIBRARY_FAILED           203


// camera
#define CP_CAMERA_INVALID_RTSP          300
#define CP_CAMERA_ALREADY_EXIST         301
#define CP_CAMERA_SEARCH_RTSP_ERROR     302

// task
#define CP_TASK_ALREADY_EXIST           400
#define CP_TASK_SERVER_FAILED           401
#define CP_COMPARE_1V1_FAILED           402

// person
#define CP_ADD_PERSON_FAILED            500
#define CP_PERSON_ALREADY_EXIST         501
#define CP_EXTRACT_FAILED               502
#define CP_INVALID_PERSON_PHOTO         503
#define CP_PERSON_CARD_NO_EXIST         504


// business
#define CP_TASK_IS_RUNNING              601

// camera_mpeg
#define CAMERA_MPEG_ADD_ERROR           701




