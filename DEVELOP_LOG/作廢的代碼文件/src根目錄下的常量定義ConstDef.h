#pragma once
#include <string>

/*當前文檔把全部boost 改為std*/

// 系统辅助宏定义
#define SAFE_DELETE(X) if(X){delete X; X = NULL;}
#define SAFE_DELETE_ARR(X) if(X){delete []X; X = NULL;}
#define USE(X) {X;}

// 属性Get方法
#define GET_METHOD(type, name) \
	inline type get##name(){ return m_##name;}

// 属性Set方法
#define SET_METHOD(type, name) \
	inline void set##name(type v){ m_##name = v;}

// 成员属性定义
#define ATTRIBUTE(type, name) \
	type m_##name;
#define ATTRIBUTE_V(type, name, value) \
	type m_##name = value;

// 成员属性定义
#define PROPERTY(type, name) \
private:\
	ATTRIBUTE(type, name); \
public:\
	GET_METHOD(type, name) \
	SET_METHOD(type, name)

// 成员属性定义
#define PROPERTY_(type, name) \
private:\
	ATTRIBUTE(type, name);

// 成员属性定义
#define PROPERTY_V(type, name, value) \
private:\
	ATTRIBUTE_V(type, name, value); \
public:\
	GET_METHOD(type, name) \
	SET_METHOD(type, name)

// 设置属性
#define SET_PROPERTY(object, property, value) \
	object->set ##property(value)

// 获取属性
#define GET_PROPERTY(object, property) \
	object->get##property()

// 安全指针释放
#define SAVE_DELETE(X) {if(X != NULL) delete X;}
#define SAVE_DELETE_ARR(X) {if(X != NULL) delete []X;}

// boost单例声明
#define BOOST_SINGLE_TON(class_name) \
class class_name : public std::serialization::singleton<class_name>

// 构造析构定义
#define CONSTRUCT(class_name) \
public:\
	class_name(void);\
	virtual ~class_name();

// 构造析构实现
#define CONSTRUCT_IMPL(class_name) \
	class_name::class_name(void) {} \
	class_name::~class_name(void) {}

// 实例化一对象
#define BOOST_OBJ(classname) \
	std::make_shared<classname>()

// 类型重定义
#define USING(newname, type) \
	using newname = type

// 类型重定义
#define BOOST_USING(newname, classname) \
	using newname = std::shared_ptr<classname>
#define BOOST_WUSING(newname, classname) \
	using newname = std::weak_ptr<classname>

// boost可复制类
#define BOOST_SHARED_CLS(classname) \
class classname : public std::enable_shared_from_this<classname>

// 成员方法定义
#define METHOD(type,name) type name
// 静态成员方法
#define SMETHOD(type,name) static type name
// 成员方法实现
#define METHODIMPL(type,classname,name) type classname::name

// 参数判断返回
#define JUSTIFY(name, v, res) if(name != v) { return res;}
// 字符参数判断
#define JUSTIFY_STR(name, v, res) if(_tcscmp(name, v) != 0) { return res;}

// 播放视频
typedef struct
{
	std::string id;
	int channel;
	int stream;
	std::string url;
	int width;
	int height;
	int bitrate;
	std::string clientId;
	std::string key;
}START_REAL_PLAY;

// 停止播放
typedef struct
{
	std::string id;
	int channel;
	int stream;
}STOP_REAL_PLAY;

// 协议类型
typedef enum
{
	PROTOTYPE_ID_RTSP = 0,
	PROTOTYPE_ID_RTMP = 1,
	PROTOTYPE_ID_SDK = 2,
}PROTOTYPE_ID;

// 录像类型
typedef enum
{
	RECORD_FMT_FLV = 0,
	RECORD_FMT_MP4 = 1,
}RECORD_FMT;

// 设备厂家
typedef enum
{
	MANUFACTURE_ID_HK = 0,
}MANUFACTURE_ID;

// 设备类型
typedef enum
{
	DEVICE_TYPE_IPC = 0,
}DEVICE_TYPE;

// 设备子类型
typedef enum
{
	DEVICE_SUB_TYPE_IPC = 0,
}DEVICE_SUB_TYPE;

// 录像类型
typedef enum
{
	RECORD_TYPE_TIMED = 0,
	RECORD_TYPE_ALARM = 1,
}RECORD_TYPE;