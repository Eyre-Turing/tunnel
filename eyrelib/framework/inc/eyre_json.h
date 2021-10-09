#ifndef EYRE_JSON_H
#define EYRE_JSON_H

/*
 * json读写能力
 * 非线程安全
 * 内部实现全是深复制，不存在两个及以上json同时引用同一个内存地址的情况
 * 
 * 作者: Eyre Turing (Eyre-Turing)
 * 最后编辑于: 2021-09-25 11:24
 */

#include <vector>
#include <map>
#include <iostream>
#include "eyre_string.h"

#define JSON_NULL		(-1)	// 不可编辑的json对象，用于方法调用失败时的异常返回值类型
#define JSON_NONE		0		// JSON_NONE和JSON_ARRAY, JSON_OBJECT是可以互相转换的，当JSON_ARRAY或JSON_OBJECT没有数据时会变成JSON_NONE
#define JSON_BOOLEAN	1
#define JSON_NUMBER		2
#define JSON_STRING		3
#define JSON_ARRAY		4		// 当往JSON_NONE里添加元素时，JSON_NONE也会变成JSON_ARRAY
#define JSON_OBJECT		5		// 当往JSON_NONE里添加键值对时，JSON_NONE也会变成JSON_OBJECT

#define JSON_FAIL		0
#define JSON_APPEND		1
#define JSON_UPDATE		2

class JsonArray;

class Json
{
public:
	static Json null();	// 生成一个不可编辑的json对象
	Json();
	Json(const Json &json);		// 深复制（包括ARRAY和OBJECT，都重新分配内存）

	Json(bool val);
	Json(double val);
	Json(int val);
	Json(const String &val);
	Json(const char *val, StringCodec codec = CODEC_AUTO);
	Json(const JsonArray &val);

	virtual ~Json();

	void asBoolean(bool val = false);					// 保存数据为布尔
	void asNumber(double val = 0.0);					// 保存数据为数字
	void asNumber(int val = 0);
	void asString(const String &val = "");				// 保存数据为文本
	void asArray();
	void asArray(const JsonArray &val);				// 保存数据为列表
	void asObject();
	void asObject(const Json &val);					// 保存数据为json对象

	Json &parent();
	const Json &parent() const;

	int type() const;

	bool boolean(bool def = false) const;	// 当该json保存的内容为布尔类型时可以用这个方法提取为布尔内容，修改返回值内容不会影响json
	double number(double def = 0) const;	// 提取为数字内容，修改返回值内容不会影响json
	String string(const String &def = "") const;	// 提取为文本内容，修改返回值内容不会影响json

	JsonArray toArray();		// 获取列表

	bool keyExist(const String &key) const;				// 判断键是否存在
	int set(const String &key, const Json &val);		// 加入键值对，返回操作状态，将以复制val的形式加入
	bool remove(const String &key);						// 删除键值对，this是json对象且key存在则删除并返回true，否则返回false
	
	static Json parseFromText(const String &text, size_t beg = 0, size_t *endpos = NULL);	// 从文本解析json，beg为解析开始偏移位置，endpos为解析完毕后的偏移位置（不是相对beg的位置，而是按text的下标位置）
	String toString(bool fold = false, size_t dep = 0, size_t space = 2) const;		// 输出为字符串，参数fold控制输出文本是否折行美化

	std::vector<String> keys() const;

	bool isNull() const;	// 是否是JSON_NULL类型，用于判断返回值为Json或Json&的方法是否调用失败

	class Iterator
	{
	public:
		Iterator();
		Iterator(Json *json, const std::map<String, Json *>::iterator &it, const String &key);

		Json &obj();	// 获取json对象
		operator Json &();

		Iterator &operator=(const Json &json);
		Iterator &operator=(bool val);
		Iterator &operator=(double val);
		Iterator &operator=(int val);
		Iterator &operator=(const String &val);
		Iterator &operator=(const char *val);
		Iterator &operator=(const JsonArray &val);

		Iterator operator[](const String &key);	// 强行插入，如果原先不是JSON_OBJECT对象会强行转换并清除原先数据，如果原先不存在该对象会创建

		JsonArray toArray();

	private:
		Json *m_j;
		std::map<String, Json *>::iterator m_it;
		String m_key;
	};

	Iterator operator[](const String &key);	// 返回key对应的子json对象
	const Json &operator[](const String &key) const;

	Json &operator=(const Json &json);
	Json &operator=(bool val);
	Json &operator=(double val);
	Json &operator=(int val);
	Json &operator=(const String &val);
	Json &operator=(const char *val);
	Json &operator=(const JsonArray &val);

	static String escape(const String &str);	// 转义
	static String descript(const String &str);	// 还原转义

	friend std::ostream &operator<<(std::ostream &out, const Json &json);

	friend class JsonArray;

private:
	Json *m_parent;

	int m_type;
	bool m_bolval;
	double m_numval;
	String m_strval;
	std::vector<Json *> m_arrval;
	std::map<String, Json *> m_objval;

	Json(const Json &json, Json *parent);
	void asObject(const Json &val, Json *parent);
	void cleanOldTypeData();
};

class JsonArray
{
public:
	JsonArray();
	JsonArray(const JsonArray &jsonArray);	// 深复制
	virtual ~JsonArray();

	size_t size() const;
	void append(const Json &json);	// 往列表里添加元素
	bool remove(size_t index);

	Json &toJson();					// 整个列表作为一个json对象

	Json &operator[](size_t index);
	const Json &operator[](size_t index) const;

	JsonArray &operator=(const JsonArray &jsonArray);

	friend std::ostream &operator<<(std::ostream &out, const JsonArray &jsonArray);

	friend class Json;

private:
	Json *m_json;
	bool needDeleteJson;
	JsonArray(Json *json);
};

extern Json JsonNone;	// 当操作失败时会返回此Json
extern Json::Iterator JsonIteratorNone;
extern JsonArray JsonArrayNone;

std::ostream &operator<<(std::ostream &out, const Json &json);
std::ostream &operator<<(std::ostream &out, const JsonArray &jsonArray);

#endif
