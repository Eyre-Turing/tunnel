#include "eyre_json.h"
#include "general.h"

/*
 * 作者: Eyre Turing (Eyre-Turing)
 * 最后编辑于: 2021/10/04 11:25
 */

Json JsonNone = Json::null();
Json::Iterator JsonIteratorNone;
JsonArray JsonArrayNone;

Json Json::null()
{
	Json result = Json();
	result.m_type = JSON_NULL;
	return result;
}

Json::Json()
{
	m_parent = NULL;
	m_type = JSON_NONE;
}

void Json::asObject(const Json &json, Json *parent)
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	m_parent = parent;
	m_type = json.m_type;
	if (m_type == JSON_NULL)	// 会影响使用方便性，替换为JSON_NONE
	{
		m_type = JSON_NONE;
	}
	switch (m_type)
	{
	case JSON_NONE:
		break;
	case JSON_BOOLEAN:
		m_bolval = json.m_bolval;
		break;
	case JSON_NUMBER:
		m_numval = json.m_numval;
		break;
	case JSON_STRING:
		m_strval = json.m_strval;
		break;
	case JSON_ARRAY:
	{
		size_t count = json.m_arrval.size();
		for (size_t i = 0; i < count; ++i)
		{
			m_arrval.push_back(new Json(*(json.m_arrval[i]), this));
		}
		break;
	}
	case JSON_OBJECT:
	{
		for (std::map<String, Json *>::const_iterator it = json.m_objval.begin();
			it != json.m_objval.end();
			++it)
		{
			m_objval[it->first] = new Json(*(it->second), this);
		}
		break;
	}
	default:
#if EYRE_DEBUG
		fprintf(stderr, "Json(%p) unknow data type \'%d\'!\n", this, m_type);
#endif
		;
	}
}

Json::Json(const Json &json, Json *parent)
{
	asObject(json, parent);
}

Json::Json(const Json &json)
{
	asObject(json, NULL);
}

Json::~Json()
{
	cleanOldTypeData();
}

Json::Json(bool val)
{
	m_parent = NULL;
	m_type = JSON_NONE;
	asBoolean(val);
}

Json::Json(double val)
{
	m_parent = NULL;
	m_type = JSON_NONE;
	asNumber(val);
}

Json::Json(int val)
{
	m_parent = NULL;
	m_type = JSON_NONE;
	asNumber(val);
}

Json::Json(const String &val)
{
	m_parent = NULL;
	m_type = JSON_NONE;
	asString(val);
}

Json::Json(const char *val, StringCodec codec)
{
	m_parent = NULL;
	m_type = JSON_NONE;
	asString(String(val, codec));
}

Json::Json(const JsonArray &val)
{
	m_parent = NULL;
	m_type = JSON_NONE;
	asArray(val);
}

void Json::cleanOldTypeData()
{
	if (m_type == JSON_ARRAY)
	{
		size_t count = m_arrval.size();
		for (size_t i = 0; i < count; ++i)
		{
			delete m_arrval[i];
		}
		m_arrval.clear();
	}
	else if (m_type == JSON_OBJECT)
	{
		for (std::map<String, Json *>::const_iterator it = m_objval.begin();
			it != m_objval.end();
			++it)
		{
			delete it->second;
		}
		m_objval.clear();
	}
}

void Json::asBoolean(bool val)
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	cleanOldTypeData();
	m_type = JSON_BOOLEAN;
	m_bolval = val;
}

void Json::asNumber(double val)
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	cleanOldTypeData();
	m_type = JSON_NUMBER;
	m_numval = val;
}

void Json::asNumber(int val)
{
	asNumber((double)val);
}

void Json::asString(const String &val)
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	cleanOldTypeData();
	m_type = JSON_STRING;
	m_strval = val;
}

void Json::asArray()
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	cleanOldTypeData();
	m_type = JSON_NONE;		// 没有数据，为JSON_NONE类型
}

void Json::asArray(const JsonArray &val)
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	asArray();
	size_t count = val.m_json->m_arrval.size();
	for (size_t i = 0; i < count; ++i)
	{
		m_type = JSON_ARRAY;
		m_arrval.push_back(new Json(*(val.m_json->m_arrval[i]), this));
	}
}

void Json::asObject()
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	cleanOldTypeData();
	m_type = JSON_NONE;		// 因为没有数据了，所以先转换为JSON_NONE状态
}

void Json::asObject(const Json &val)
{
	if (m_type == JSON_NULL)
	{
		return ;
	}
	asObject();
	for (std::map<String, Json *>::const_iterator it = val.m_objval.begin();
		it != val.m_objval.end();
		++it)
	{
		m_type = JSON_OBJECT;	// 添加了数据，变为JSON_OBJECT状态
		m_objval[it->first] = new Json(*(it->second), this);
	}
}

Json::Iterator Json::operator[](const String &key)
{
	if (m_type != JSON_OBJECT && m_type != JSON_NONE)
	{
		return JsonIteratorNone;
	}
	std::map<String, Json *>::iterator it = m_objval.find(key);
	return Json::Iterator(this, it, key);
}

const Json &Json::operator[](const String &key) const
{
	if (m_type != JSON_OBJECT && m_type != JSON_NONE)
	{
#if EYRE_DEBUG
		fprintf(stderr, "Json(%p) isn\'t a object!\n", this);
#endif
		return JsonNone;
	}
	std::map<String, Json *>::const_iterator it = m_objval.find(key);
	if (it == m_objval.end())
	{
		return JsonNone;
	}
	return *(it->second);
}

Json &Json::parent()
{
	if (!m_parent)
	{
		return JsonNone;
	}
	return *m_parent;
}

const Json &Json::parent() const
{
	if (!m_parent)
	{
		return JsonNone;
	}
	return *m_parent;
}

int Json::type() const
{
	return m_type;
}

bool Json::boolean(bool def) const
{
	if (m_type != JSON_BOOLEAN)
	{
		return def;
	}
	return m_bolval;
}

double Json::number(double def) const
{
	if (m_type != JSON_NUMBER)
	{
		return def;
	}
	return m_numval;
}

String Json::string(const String &def) const
{
	if (m_type != JSON_STRING)
	{
		return def;
	}
	return m_strval;
}

JsonArray Json::toArray()
{
	if (m_type != JSON_ARRAY && m_type != JSON_NONE)
	{
		return JsonArrayNone;
	}
	return JsonArray(this);
}

bool Json::keyExist(const String &key) const
{
	if (m_type != JSON_OBJECT)
	{
		return false;
	}
	return (m_objval.find(key) != m_objval.end());
}

int Json::set(const String &key, const Json &val)
{
	if (m_type != JSON_OBJECT && m_type != JSON_NONE)
	{
		return JSON_FAIL;
	}
	int method;
	if (keyExist(key))	// 更新
	{
		delete m_objval[key];	// 把旧的删除
		method = JSON_UPDATE;
	}
	else
	{
		method = JSON_APPEND;
	}
	m_objval[key] = new Json(val, this);
	if (m_type == JSON_NONE)
	{
		m_type = JSON_OBJECT;
	}
	return method;
}

bool Json::remove(const String &key)
{
	if (m_type != JSON_OBJECT)
	{
		return false;
	}
	std::map<String, Json *>::iterator it = m_objval.find(key);
	if (it == m_objval.end())
	{
		return false;
	}
	m_objval.erase(it);
	if (m_objval.empty())
	{
		m_type = JSON_NONE;		// 没有数据了，状态变更为JSON_NONE
	}
	return true;
}

Json Json::parseFromText(const String &text, size_t beg, size_t *endpos)
{
	size_t len = text.size();
	for (; beg < len && (text.at(beg) == ' ' || text.at(beg) == '\n' || text.at(beg) == '\r' || text.at(beg) == ','); ++beg);	// 忽略空格、换行、逗号
	if (beg >= len)	// 没有正文
	{
		return JsonNone;
	}

	Json json;

	/*
	 * 第一个匹配到的符号有6种情况
	 * 1、匹配到 { ，表示接下来是一个json数据
	 * 2、匹配到 [ ，表示接下来是一个列表
	 * 3、匹配到 " ，表示整个是一个字符串
	 * 4、匹配到 [0-9] ，表示整个是一个数字
	 * 5、匹配到 [tf] ，表示整个是一个布尔
	 * 6、匹配到 n ，表示是一个空json
	 */
	if (text.at(beg) == '{')	// 扫描直到遇到 } 完成数据解析（因为用递归的方法去做，所有即使内部也有若干{ }，也是在递归里就完成了，在最外层匹配到的第一个 } 就会是结束标志了）
	{
		json.asObject();
		for (++beg; beg < len && text.at(beg) != '}';)
		{
			if (text.at(beg) != ' ' && text.at(beg) != '\n' && text.at(beg) != '\r' && text.at(beg) != ',')	// 忽略空格、换行、逗号
			{
				// 第一个匹配到的符号只可能为 " ，表示接下来是一个键
				if (text.at(beg) != '\"')
				{
					// 出问题了，文本不正常
					if (endpos)
					{
						*endpos = len;
					}
					return JsonNone;
				}
				size_t e = text.indexOf("\"", beg+1);
				if (e == -1)	// 双引号不匹配，文本不正常
				{
					if (endpos)
					{
						*endpos = len;
					}
					return JsonNone;
				}
				String key = text.mid(beg+1, e-beg-1);	// 得到键
				e = text.indexOf(":", e+1);	// 找 : 这是值开始的标志
				if (e == -1)	// 键后面没有 : ，文本不正常
				{
					if (endpos)
					{
						*endpos = len;
					}
					return JsonNone;
				}
				json[key] = parseFromText(text, e+1, &beg);		// 这里会刷新beg的值，文本偏移将直接跳转到子json的后面
			}
			else
			{
				++beg;
			}
		}
		if (endpos)
		{
			*endpos = beg+1;
		}
	}
	else if (text.at(beg) == '[')	// 扫描直到遇到 ]
	{
		json.asArray();
		JsonArray jsonArray = json.toArray();
		for (++beg; beg < len && text.at(beg) != ']';)
		{
			if (text.at(beg) != ' ' && text.at(beg) != '\n' && text.at(beg) != '\r' && text.at(beg) != ',')	// 忽略空格、换行、逗号
			{
				// 直接匹配子json
				jsonArray.append(parseFromText(text, beg, &beg));
			}
			else
			{
				++beg;
			}
		}
		if (endpos)
		{
			*endpos = beg+1;
		}
	}
	else if (text.at(beg) == '\"')	// 匹配直到 " ，遇到转义的双引号要继续往下获取
	{
		size_t e;
		bool en = false;	// 当前文本是否进入转义状态
		for (e = beg+1; e < len; ++e)
		{
			if (en)	// 转义状态
			{
				en = false;	// 转义状态遇到什么字符都不需要理会，取消转义状态即可
			}
			else if (text.at(e) == '\\')	// 非转义状态遇到转义标志字符
			{
				en = true;	// 进入转义状态
			}
			else if (text.at(e) == '\"')	// 非转义状态遇到 "
			{
				break;
			}
		}
		if (e >= len)
		{
			// 双引号不匹配，文本不正常
			if (endpos)
			{
				*endpos = len;
			}
			return JsonNone;
		}
		json = descript(text.mid(beg+1, e-beg-1));
		if (endpos)
		{
			*endpos = e+1;
		}
	}
	else if (text.at(beg) >= '0' && text.at(beg) <= '9')	// 匹配直到任意 [^0-9.]
	{
		size_t e;
		for (e = beg; e < len && ((text.at(e) >= '0' && text.at(e) <= '9') || text.at(e) == '.'); ++e);
		if (e >= len)
		{
			if (endpos)
			{
				*endpos = len;
			}
			return JsonNone;
		}
		json = text.mid(beg, e-beg).toDouble();
		if (endpos)
		{
			*endpos = e+1;
		}
	}
	else if (text.at(beg) == 't')	// 匹配4个字符
	{
		if (text.mid(beg, 4) != "true")
		{
			if (endpos)
			{
				*endpos = len;
			}
			return JsonNone;
		}
		json = true;
		if (endpos)
		{
			*endpos = beg+4;
		}
	}
	else if (text.at(beg) == 'f')	// 匹配5个字符
	{
		if (text.mid(beg, 5) != "false")
		{
			if (endpos)
			{
				*endpos = len;
			}
			return JsonNone;
		}
		json = false;
		if (endpos)
		{
			*endpos = beg+5;
		}
	}
	else if (text.at(beg) == 'n')	// 匹配4个字符
	{
		if (text.mid(beg, 4) != "null")
		{
			if (endpos)
			{
				*endpos = len;
			}
			return JsonNone;
		}
		json = JsonNone;
		if (endpos)
		{
			*endpos = beg+4;
		}
	}

	return json;
}

static String pad(size_t dep, size_t space)
{
	String str = "\n";
	for (size_t s = 0; s < dep*space; ++s)
	{
		str  += " ";
	}
	return str;
}

String Json::toString(bool fold, size_t dep, size_t space) const
{
	String str;
	if (m_type == JSON_ARRAY)
	{
		str += "[";
		size_t count = m_arrval.size();
		for (size_t i = 0; i < count; ++i)
		{
			if (i)
			{
				str += ",";
			}
			if (fold)
			{
				str += pad(dep+1, space);
			}
			str += m_arrval[i]->toString(fold, dep+1);
		}
		if (fold)
		{
			str += pad(dep, space);
		}
		str += "]";
	}
	else if (m_type == JSON_OBJECT)
	{
		str += "{";
		for (std::map<String, Json *>::const_iterator it = m_objval.begin();
			it != m_objval.end();
			++it)
		{
			if (it != m_objval.begin())
			{
				str += ",";
			}
			if (fold)
			{
				str += pad(dep+1, space);
			}
			str += "\""+it->first+"\":";
			if (fold)
			{
				str += " ";
			}
			str += it->second->toString(fold, dep+1);
		}
		if (fold)
		{
			str += pad(dep, space);
		}
		str += "}";
	}
	else
	{
		switch (m_type)
		{
		case JSON_NULL:
			str += "null";
			break;
		case JSON_NONE:
			str += "null";
			break;
		case JSON_BOOLEAN:
			if (m_bolval)
			{
				str += "true";
			}
			else
			{
				str += "false";
			}
			break;
		case JSON_NUMBER:
			str += String::fromNumber(m_numval);
			break;
		case JSON_STRING:
			str += "\""+escape(m_strval)+"\"";
			break;
		default:
			;
		}
	}
	return str;
}

std::vector<String> Json::keys() const
{
	std::vector<String> result;
	if (m_type != JSON_OBJECT)
	{
		return result;
	}
	for (std::map<String, Json *>::const_iterator it = m_objval.begin();
		it != m_objval.end();
		++it)
	{
		result.push_back(it->first);
	}
	return result;
}

bool Json::isNull() const
{
	return m_type == JSON_NULL;
}

Json &Json::operator=(const Json &json)
{
	if (m_type == JSON_NULL)
	{
		return *this;
	}
	asObject(json);
	return *this;
}

Json &Json::operator=(bool val)
{
	if (m_type == JSON_NULL)
	{
		return *this;
	}
	asBoolean(val);
	return *this;
}

Json &Json::operator=(double val)
{
	if (m_type == JSON_NULL)
	{
		return *this;
	}
	asNumber(val);
	return *this;
}

Json &Json::operator=(int val)
{
	return (*this)=(double)val;
}

Json &Json::operator=(const String &val)
{
	if (m_type == JSON_NULL)
	{
		return *this;
	}
	asString(val);
	return *this;
}

Json &Json::operator=(const char *val)
{
	if (m_type == JSON_NULL)
	{
		return *this;
	}
	asString(String(val));
	return *this;
}

Json &Json::operator=(const JsonArray &val)
{
	if (m_type == JSON_NULL)
	{
		return *this;
	}
	asArray(val);
	return *this;
}

Json::Iterator::Iterator()
{
	m_j = NULL;
}

Json::Iterator::Iterator(Json *json, const std::map<String, Json *>::iterator &it, const String &key)
{
	m_j = json;
	m_it = it;
	m_key = key;
}

Json &Json::Iterator::obj()
{
	if (!m_j)
	{
		return JsonNone;
	}
	if (m_it == m_j->m_objval.end())
	{
		return JsonNone;
	}
	return *(m_it->second);
}

Json::Iterator::operator Json &()
{
	return obj();
}

Json::Iterator &Json::Iterator::operator=(const Json &json)
{
	if (m_j)
	{
		m_j->set(m_key, json);
	}
	return *this;
}

Json::Iterator &Json::Iterator::operator=(bool val)
{
	if (m_j)
	{
		m_j->set(m_key, Json(val));
	}
	return *this;
}

Json::Iterator &Json::Iterator::operator=(double val)
{
	if (m_j)
	{
		m_j->set(m_key, Json(val));
	}
	return *this;
}

Json::Iterator &Json::Iterator::operator=(int val)
{
	return (*this)=(double)val;
}

Json::Iterator &Json::Iterator::operator=(const String &val)
{
	if (m_j)
	{
		m_j->set(m_key, Json(val));
	}
	return *this;
}

Json::Iterator &Json::Iterator::operator=(const char *val)
{
	if (m_j)
	{
		m_j->set(m_key, Json(String(val)));
	}
	return *this;
}

Json::Iterator &Json::Iterator::operator=(const JsonArray &val)
{
	if (m_j)
	{
		m_j->set(m_key, Json(val));
	}
	return *this;
}

Json::Iterator Json::Iterator::operator[](const String &key)
{
	if (!m_j)
	{
		return *this;
	}
	if (m_it == m_j->m_objval.end())	// 原先元素不存在
	{
		m_j->set(m_key, Json());	// 创建
		m_it = m_j->m_objval.find(m_key);	// 更新迭代器
	}
	
	if (m_it->second->m_type != JSON_OBJECT && m_it->second->m_type != JSON_NONE)	// 类型不对
	{
		m_it->second->asObject();	// 强行转换为JSON_NONE
	}

	return Iterator(m_it->second, m_it->second->m_objval.find(key), key);
}

JsonArray Json::Iterator::toArray()
{
	return ((Json &)(*this)).toArray();
}

static std::map<char, String> genEscapeMethod()
{
	std::map<char, String> result;
	result['\r'] = "\\r";
	result['\n'] = "\\n";
	result['\t'] = "\\t";
	result['\"'] = "\\\"";
	result['\\'] = "\\\\";
	return result;
}

String Json::escape(const String &str)
{
	static std::map<char, String> escapeMethod = genEscapeMethod();

	String result = "";
	size_t len = str.size();
	for (size_t i = 0; i < len; ++i)
	{
		std::map<char, String>::const_iterator it = escapeMethod.find(str.at(i));
		if (it == escapeMethod.end())
		{
			result += str.at(i);
		}
		else
		{
			result += it->second;
		}
	}
	return result;
}

static std::map<char, char> genDescriptMethod()
{
	std::map<char, char> result;
	result['r'] = '\r';
	result['n'] = '\n';
	result['t'] = '\t';
	result['\"'] = '\"';
	result['\\'] = '\\';
	return result;
}

String Json::descript(const String &str)
{
	static std::map<char, char> descriptMethod = genDescriptMethod();

	String result = "";
	size_t len = str.size();
	for (size_t i = 0; i < len; ++i)
	{
		if (str.at(i) == '\\' && ++i < len)	// 以反斜杠为转义字符开始标志
		{
			std::map<char, char>::const_iterator it = descriptMethod.find(str.at(i));
			if (it != descriptMethod.end())
			{
				result += it->second;
			}
		}
		else
		{
			result += str.at(i);
		}
	}
	return result;
}

JsonArray::JsonArray(Json *json)
{
	m_json = json;
	needDeleteJson = false;
}

JsonArray::JsonArray()
{
	m_json = &JsonNone;
	needDeleteJson = false;
}

JsonArray::JsonArray(const JsonArray &jsonArray)
{
	m_json = new Json(*(jsonArray.m_json));
	needDeleteJson = true;
}

JsonArray::~JsonArray()
{
	if (needDeleteJson)
	{
		delete m_json;
	}
}

size_t JsonArray::size() const
{
	if (m_json->m_type != JSON_ARRAY && m_json->m_type != JSON_NONE)
	{
		return 0;
	}
	return m_json->m_arrval.size();
}

void JsonArray::append(const Json &json)
{
	if (m_json->m_type != JSON_ARRAY && m_json->m_type != JSON_NONE)
	{
		return ;
	}
	m_json->m_arrval.push_back(new Json(json, m_json));
	m_json->m_type = JSON_ARRAY;	// 因为有数据了，所以类型修改为JSON_ARRAY
}

bool JsonArray::remove(size_t index)
{
	if (index >= size())
	{
		return false;
	}
	m_json->m_arrval.erase(m_json->m_arrval.begin()+index);
	if (size() == 0)
	{
		m_json->m_type == JSON_NONE;
	}
	return true;
}

Json &JsonArray::toJson()
{
	return *m_json;
}

Json &JsonArray::operator[](size_t index)
{
	if (index >= size())
	{
		return JsonNone;
	}
	return *(m_json->m_arrval[index]);
}

const Json &JsonArray::operator[](size_t index) const
{
	if (index >= size())
	{
		return JsonNone;
	}
	return *(m_json->m_arrval[index]);
}

JsonArray &JsonArray::operator=(const JsonArray &jsonArray)
{
	m_json->asArray(jsonArray);
	return *this;
}

static size_t currentOutputJsonDepth = 0;	// 当前输出json到哪一层了
static size_t depthSpaces = 2;				// 每升一层往右偏移多少个空格

std::ostream &operator<<(std::ostream &out, const Json &json)
{
	switch (json.m_type)
	{
	case JSON_NULL:
		out << "none";		// JSON_NULL也显示成none，因为JSON_NULL只是用于方法调用失败时的异常返回值而已，并不是真正意义上的一个类型
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		break;
	case JSON_NONE:
		out << "none";
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		break;
	case JSON_BOOLEAN:
		if (json.m_bolval)
		{
			out << "true";
		}
		else
		{
			out << "false";
		}
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		break;
	case JSON_NUMBER:
		out << json.m_numval;
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		break;
	case JSON_STRING:
		out << "\"" << Json::escape(json.m_strval) << "\"";
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		break;
	case JSON_ARRAY:
	{
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		size_t count = json.m_arrval.size();
		for (size_t i = 0; i < count; ++i)
		{
			for (size_t j = 0; j < currentOutputJsonDepth*depthSpaces; ++j)
			{
				out << " ";
			}
			++currentOutputJsonDepth;
			out << "- " << *(json.m_arrval[i]);
			--currentOutputJsonDepth;
		}
		break;
	}
	case JSON_OBJECT:
	{
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		std::vector<String> keys = json.keys();
		size_t keycount = keys.size();
		for (size_t i = 0; i < keycount; ++i)
		{
			
			for (size_t j = 0; j < currentOutputJsonDepth*depthSpaces; ++j)
			{
				out << " ";
			}
			out << keys[i] << ": ";
			++currentOutputJsonDepth;
			out << json[keys[i]];
			--currentOutputJsonDepth;
		}
		break;
	}
	default:
		out << "Error: Unknow type \'" << json.m_type << "\'!" << "\n";
	}
	return out;
}

std::ostream &operator<<(std::ostream &out, const JsonArray &jsonArray)
{
	out << JsonArray(jsonArray).toJson();

	return out;
}
