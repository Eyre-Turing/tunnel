/*
 * In class String, m_data save as utf-8.
 * If system using codec gbk, will convert into utf-8.
 * When need print String in console,
 * copy and convert into system codec to print.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-21 12:12.
 */

#include "eyre_string.h"
#include "general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

StringCodec String::codecAutoDef = CODEC_AUTO_DEF;
StringCodec String::codecSysDef = CODEC_SYS_DEF;

//codec means what str codec is.
String::String(const char *str, StringCodec codec)
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *strc = gbkToUtf8(str);
		m_data = new ByteArray(strc);
		free(strc);
	}
	else if(codec == CODEC_UTF8)
	{
		m_data = new ByteArray(str);
	}
	else
	{
		m_data = new ByteArray();
#if EYRE_DEBUG
		fprintf(stderr, "String(%p) unknow codec: \'%d\'!\n", this, codec);
#endif
	}
#if EYRE_DETAIL
	fprintf(stdout, "String(%p) created.\n", this);
#endif
}

String::String(const ByteArray &b, StringCodec codec)
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *strc = gbkToUtf8(b.m_data);
		m_data = new ByteArray(strc);
		free(strc);
	}
	else if(codec == CODEC_UTF8)
	{
		//use b.m_data to create, because b may have '\0' before end, so this operation can clean the data after '\0'.
		m_data = new ByteArray(b.m_data);
	}
	else
	{
		m_data = new ByteArray();
#if EYRE_DEBUG
		fprintf(stderr, "String(%p) unknow codec: \'%d\'!\n", this, codec);
#endif
	}
#if EYRE_DETAIL
	fprintf(stdout, "String(%p) created.\n", this);
#endif
}

String::String(const String &s)
{
	m_data = new ByteArray(*(s.m_data));
#if EYRE_DETAIL
	fprintf(stdout, "String(%p) copy.\n", this);
#endif
}

String::~String()
{
	delete m_data;
#if EYRE_DETAIL
	fprintf(stdout, "String(%p) destroyed.\n", this);
#endif
}

int String::indexOf(const String &s, unsigned int offset) const
{
	return indexOf(s.m_data->m_data, offset, CODEC_UTF8);
}

/*
 * Codec is what str codec is.
 * If codec is UTF8 do nothing and find,
 * else convert to UTF8 and find.
 */
int String::indexOf(const char *str, unsigned int offset, StringCodec codec) const
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *strc = gbkToUtf8(str);
		int result = m_data->indexOf(strc, offset);
		free(strc);
		return result;
	}
	else if(codec == CODEC_UTF8)
	{
		return m_data->indexOf(str, offset);
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, 
		"String(%p) indexOf(const char*, unsigned int, StringCodec) unknow codec: \'%d\'!\n", 
		this, codec);
#endif
		return -1;
	}
}

int String::lastIndexOf(const String &s, unsigned int offset) const
{
	return lastIndexOf(s.m_data->m_data, offset, CODEC_UTF8);
}

int String::lastIndexOf(const char *str, unsigned int offset, StringCodec codec) const
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *strc = gbkToUtf8(str);
		int result = m_data->lastIndexOf(strc, offset);
		free(strc);
		return result;
	}
	else if(codec == CODEC_UTF8)
	{
		return m_data->lastIndexOf(str, offset);
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, 
		"String(%p) lastIndexOf(const char*, unsigned int, StringCodec) unknow codec: \'%d\'!\n", 
		this, codec);
#endif
		return -1;
	}
}

String::operator char*()
{
	return m_data->m_data;
}

String::operator const char*() const
{
	return (const char *) m_data->m_data;
}

std::ostream &operator<<(std::ostream &out, const String &s)
{
//#if (CODEC_SYS_DEF == CODEC_GBK)
	if(String::codecSysDef == CODEC_GBK)
	{
		char *data = utf8ToGbk(s.m_data->m_data);
		out<<data;
		free(data);
	}
//#else
	else
	{
		out<<s.m_data->m_data;
	}
//#endif
	return out;
}

std::istream &operator>>(std::istream &in, String &s)
{
	std::string input;
	in>>input;
//#if (CODEC_SYS_DEF == CODEC_GBK)
	if(String::codecSysDef == CODEC_GBK)
	{
		char *data = gbkToUtf8(input.c_str());
		*(s.m_data) = data;
#if EYRE_DETAIL
		fprintf(stdout, "in>> codec gbk\ninput: %s\nsize: %d\n", data, strlen(data));
#endif
		free(data);
	}
//#else
	else
	{
		*(s.m_data) = input.c_str();
#if EYRE_DETAIL
		fprintf(stdout, "in>> codec utf-8\ninput: %s\nsize: %d\n", input.c_str(), input.size());
#endif
	}
//#endif
	return in; 
}

std::istream &getline(std::istream &in, String &s)
{
	return getline(in, s, '\n');
}

std::istream &getline(std::istream &in, String &s, char delim)
{
	std::string input;
	getline(in, input, delim);
//#if (CODEC_SYS_DEF == CODEC_GBK)
	if(String::codecSysDef == CODEC_GBK)
	{
		char *data = gbkToUtf8(input.c_str());
		*(s.m_data) = data;
		free(data);
	}
//#else
	else
	{
		*(s.m_data) = input.c_str();
	}
//#endif
	return in;
}

String &String::operator=(const String &s)
{
	*m_data = *(s.m_data);
	return *this;
}

String &String::operator=(const char *str)
{
//#if (CODEC_AUTO_DEF == CODEC_GBK)
	if(codecAutoDef == CODEC_GBK)
	{
		char *strc = gbkToUtf8(str);
		*m_data = strc;
		free(strc);
	}
//#else
	else
	{
		*m_data = str;
	}
//#endif
	return *this;
}

String String::fromGbk(const char *str)
{
	return String(str, CODEC_GBK);
}

String String::fromUtf8(const char *str)
{
	return String(str, CODEC_UTF8);
}

String String::fromLocal(const char *str)
{
//#if (CODEC_SYS_DEF == CODEC_GBK)
	if(codecSysDef == CODEC_GBK)
	{
		return fromGbk(str);
	}
//#else
	else
	{
		return fromUtf8(str);
	}
//#endif
}

bool String::append(const char *str, StringCodec codec)
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *strc = gbkToUtf8(str);
		bool status = m_data->append(strc);
		free(strc);
		return status;
	}
	else if(codec == CODEC_UTF8)
	{
		return m_data->append(str);
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::append(const char*, StringCodec) unknow codec: \'%d\'!\n", this, codec);
#endif
		return false;
	}
}

bool String::append(const String &s)
{
	return append(s.m_data->m_data, CODEC_UTF8);
}

bool String::append(char c)
{
	if(c == 0)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: try to append 0 to String(%p), reject!\n", this);
#endif
		return false;
	}
	else if(c < 0)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: try to append invisible char, it\'s easy to miscode.\n");
#endif
	}
	return m_data->append(c);
}

String &String::operator<<(const char *str)
{
	append(str);
	return *this;
}

String &String::operator<<(const String &s)
{
	append(s);
	return *this;
}

String &String::operator<<(char c)
{
	append(c);
	return *this;
}

String &String::operator+=(const char *str)
{
	return (*this)<<str;
}

String &String::operator+=(const String &s)
{
	return (*this)<<s;
}

String &String::operator+=(char c)
{
	return (*this)<<c;
}

bool String::operator==(const char *str) const
{
//#if (CODEC_AUTO_DEF == CODEC_GBK)
	if(codecAutoDef == CODEC_GBK)
	{
		char *strc = gbkToUtf8(str);
		bool status = ((*m_data)==strc);
		free(strc);
		return status;
	}
//#else
	else
	{
		return (*m_data)==str;
	}
//#endif
}

bool String::operator==(char *str) const
{
	return (*this)==(const char *) str;
}

bool String::operator==(const String &s) const
{
	return (*m_data)==(*(s.m_data));
}

bool String::operator!=(const char *str) const
{
	return !((*this)==str);
}

bool String::operator!=(char *str) const
{
	return !((*this)==str);
}

bool String::operator!=(const String &s) const
{
	return !((*this)==s);
}

unsigned int String::size() const
{
	return m_data->size();
}

String &String::replace(const String &tag, const String &to)
{
	return replace(tag.m_data->m_data, to.m_data->m_data, CODEC_UTF8, CODEC_UTF8);
}

String &String::replace(const String &tag, const char *to, StringCodec toCodec)
{
	return replace(tag.m_data->m_data, to, CODEC_UTF8, toCodec);
}

String &String::replace(const char *tag, const String &to, StringCodec tagCodec)
{
	return replace(tag, to.m_data->m_data, tagCodec, CODEC_UTF8);
}

String &String::replace(const char *tag, const char *to, StringCodec tagCodec, StringCodec toCodec)
{
	if(tagCodec == CODEC_AUTO)
	{
		//tagCodec = CODEC_AUTO_DEF;
		tagCodec = codecAutoDef;
	}
	if(toCodec == CODEC_AUTO)
	{
		//toCodec = CODEC_AUTO_DEF;
		toCodec = codecAutoDef;
	}
	
	const char *tagcc;
	const char *tocc;
	
	char *tagc = NULL;
	char *toc = NULL;
	
	if(tagCodec == CODEC_GBK)
	{
		tagc = gbkToUtf8(tag);
		tagcc = (const char *) tagc;
	}
	else if(tagCodec == CODEC_UTF8)
	{
		tagcc = tag;
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::replace unknow tagCodec: \'%d\', stop!\n", this, tagCodec);
#endif
		return *this;
	}
	
	if(toCodec == CODEC_GBK)
	{
		toc = gbkToUtf8(to);
		tocc = (const char *) toc;
	}
	else if(toCodec == CODEC_UTF8)
	{
		tocc = to;
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::replace unknow toCodec: \'%d\', stop!\n", this, toCodec);
#endif
		return *this;
	}
	
	m_data->replace(tagcc, tocc);
	
	free(tagc);
	free(toc);
	
	return *this;
	
//#if (CODEC_AUTO_DEF == CODEC_GBK)
//	char *tagc = gbkToUtf8(tag);
//	char *toc = gbkToUtf8(to);
//	m_data->replace(tagc, toc);
//	free(tagc);
//	free(toc);
//#else
//	m_data->replace(tag, to);
//#endif
//	return *this;
}

String operator+(const String &a, const String &b)
{
	String result(a);
	result += b;
	return result;
}

String::Iterator String::operator[](unsigned int offset)
{
	if(offset > size())
	{
		offset = 0;
	}
	return Iterator(this, offset);
}

String String::mid(unsigned int offset, int size) const
{
	return String(m_data->mid(offset, size), CODEC_UTF8);
}

//codec is what tag codec is.
std::vector<String> String::split(const char *tag, StringCodec codec) const
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	std::vector<String> result;
	if(codec == CODEC_GBK)
	{
		char *tagc = gbkToUtf8(tag);
		std::vector<ByteArray> bresult = m_data->split(tagc);
		free(tagc);
		int bsize = bresult.size();
		for(int i=0; i<bsize; ++i)
		{
			result.push_back(String(bresult[i], CODEC_UTF8));
		}
	}
	else if(codec == CODEC_UTF8)
	{
		std::vector<ByteArray> bresult = m_data->split(tag);
		int bsize = bresult.size();
		for(int i=0; i<bsize; ++i)
		{
			result.push_back(String(bresult[i], CODEC_UTF8));
		}
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::split(const char*, StringCodec) unknow codec: \'%d\'!\n", this, codec);
#endif
	}
	return result;
}

std::vector<String> String::split(const String &tag) const
{
	return split(tag.m_data->m_data, CODEC_UTF8);
}

//codec is what to codec is.
String &String::replace(unsigned int offset, unsigned int range, const char *to, StringCodec codec)
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *toc = gbkToUtf8(to);
		m_data->replace(offset, range, toc);
		free(toc);
	}
	else if(codec == CODEC_UTF8)
	{
		m_data->replace(offset, range, to);
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::replace(unsigned int, unsigned int, const char*, StringCodec) unknow codec: \'%d\'!\n", this, codec);
#endif
	}
	return *this;
}

String &String::replace(unsigned int offset, unsigned int range, const String &to)
{
	return replace(offset, range, to.m_data->m_data, CODEC_UTF8);
}

String &String::insert(unsigned int offset, const char *to, StringCodec codec)
{
	return replace(offset, 0, to, codec);
}

String &String::insert(unsigned int offset, const String &to)
{
	return replace(offset, 0, to);
}

char String::at(unsigned int pos) const
{
	return m_data->at(pos);
}

bool String::operator<(const String &s) const
{
	return (*m_data)<(*(s.m_data));
}

bool String::operator>(const String &s) const
{
	return (*m_data)>(*(s.m_data));
}

bool String::operator<=(const String &s) const
{
	return (*m_data)<=(*(s.m_data));
}

bool String::operator>=(const String &s) const
{
	return (*m_data)>=(*(s.m_data));
}

String::Iterator::Iterator(String *s, unsigned int offset) : m_s(s), m_offset(offset)
{
#if EYRE_DETAIL
	fprintf(stdout, "String::Iterator(%p) created.\n", this);
#endif
}

String::Iterator::~Iterator()
{
#if EYRE_DETAIL
	fprintf(stdout, "String::Iterator(%p) destroyed.\n", this); 
#endif
}

String::Iterator::operator char() const
{
	return m_s->m_data->m_data[m_offset];
}

String String::Iterator::operator[](int size) const
{
	return m_s->mid(m_offset, size);
} 

String::Iterator &String::Iterator::operator=(char c)
{
	if(m_offset >= m_s->size())
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: try to change end char in String(%p), reject!\n", m_s);
#endif
		return *this;
	}
	if(c == 0)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: try to set 0 to String(%p), reject!\n", m_s);
#endif
		return *this;
	}
	else if((c<0 && m_s->m_data->m_data[m_offset]>0) || (c>0 && m_s->m_data->m_data[m_offset]<0))
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: it\'s easy to miscode.\n");
#endif
	}
	
	m_s->m_data->m_data[m_offset] = c;
	
	return *this;
}

//codec is what to codec is.
void String::Iterator::replace(unsigned int range, const char *to, StringCodec codec)
{
	m_s->replace(m_offset, range, to, codec);
}

void String::Iterator::replace(unsigned int range, const String &to)
{
	replace(range, to.m_data->m_data, CODEC_UTF8);
}

void String::Iterator::insert(const char *to, StringCodec codec)
{
	replace(0, to, codec);
}

void String::Iterator::insert(const String &to)
{
	replace(0, to);
}

std::ostream &operator<<(std::ostream &out, const std::vector<String> &sv)
{
	unsigned int size = sv.size();
	out<<"StringVector(";
	for(unsigned int i=0; i<size; ++i)
	{
		if(i)
		{
			out<<", ";
		}
		out<<sv[i];
	}
	out<<")";
	return out;
}

int String::toInt() const
{
	int result = 0;
	if(sscanf(m_data->m_data, "%d", &result) != 1)
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::toInt fail!\n", this);
#endif
	}
	return result;
}

unsigned int String::toUInt() const
{
	unsigned int result = 0;
	if(sscanf(m_data->m_data, "%u", &result) != 1)
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::toUInt fail!\n", this);
#endif
	}
	return result;
}

long long String::toInt64() const
{
	long long result = 0;
	if(sscanf(m_data->m_data, "%lld", &result) != 1)
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::toInt64 fail!\n", this);
#endif
	}
	return result;
}

unsigned long long String::toUInt64() const
{
	unsigned long long result = 0;
	if(sscanf(m_data->m_data, "%llu", &result) != 1)
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::toUInt64 fail!\n", this);
#endif
	}
	return result;
}

float String::toFloat() const
{
	float result = 0;
	if(sscanf(m_data->m_data, "%f", &result) != 1)
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::toFloat fail!\n", this);
#endif
	}
	return result;
}

double String::toDouble() const
{
	double result = 0;
	if(sscanf(m_data->m_data, "%lf", &result) != 1)
	{
#if EYRE_DEBUG
		fprintf(stderr, "String(%p)::toDouble fail!\n", this);
#endif
	}
	return result;
}

String String::fromNumber(int num)
{
	char temp[128];
	sprintf(temp, "%d", num);
	return String(temp, CODEC_UTF8);
}

String String::fromNumber(unsigned int num)
{
	char temp[128];
	sprintf(temp, "%u", num);
	return String(temp, CODEC_UTF8);
}

String String::fromNumber(long long num)
{
	char temp[128];
	sprintf(temp, "%lld", num);
	return String(temp, CODEC_UTF8);
}

String String::fromNumber(unsigned long long num)
{
	char temp[128];
	sprintf(temp, "%llu", num);
	return String(temp, CODEC_UTF8);
}

String String::fromNumber(float num)
{
	char temp[128];
	sprintf(temp, "%f", num);
	return String(temp, CODEC_UTF8);
}

String String::fromNumber(double num)
{
	char temp[128];
	sprintf(temp, "%lf", num);
	return String(temp, CODEC_UTF8);
}

bool String::chIsNumber(char ch)
{
	return (ch>='0' && ch<='9');
}

String String::argFindMinTag() const
{
	int minNumber = 100;
	int index = -1;
	int tempNumber;
	while((index=indexOf("%", index+1, CODEC_UTF8)) != -1)
	{
		if(chIsNumber(at(index+1)))
		{
			tempNumber = (int)(at(index+1)-'0');
			if(tempNumber == 0)
			{
				continue;
			}
			if(chIsNumber(at(index+2)))
			{
				tempNumber *= 10;
				tempNumber += (int)(at(index+2)-'0');
			}
			if(tempNumber>0 && tempNumber<minNumber)
			{
				minNumber = tempNumber;
			}
		}
	}
	if(minNumber<100 && minNumber>0)
	{
		return String("%", CODEC_UTF8)+String::fromNumber(minNumber);
	}
	return "";
}

String &String::replaceForArg(const String &tag, const char *to, StringCodec codec)
{
	unsigned int tagsize = tag.size();
	unsigned int tosize = strlen(to);
	int index = 0;
	while((index=indexOf(tag, index)) != -1)
	{
		if(tagsize==3 || !chIsNumber(at(index+2)))
		{
			replace(index, tagsize, to, codec);
			index += tosize;
		}
		else
		{
			index += tagsize;
		}
	}
	return *this;
}

String &String::replaceForArg(const String &tag, const String &to)
{
	return replaceForArg(tag, to, CODEC_UTF8);
}

String &String::arg(const char *to, StringCodec codec)
{
	return replaceForArg(argFindMinTag(), to, codec);
}

String &String::arg(const String &to)
{
	return replaceForArg(argFindMinTag(), to);
}

String &String::arg(int to)
{
	return replaceForArg(argFindMinTag(), String::fromNumber(to));
}

String &String::arg(unsigned int to)
{
	return replaceForArg(argFindMinTag(), String::fromNumber(to));
}

String &String::arg(long long to)
{
	return replaceForArg(argFindMinTag(), String::fromNumber(to));
}

String &String::arg(unsigned long long to)
{
	return replaceForArg(argFindMinTag(), String::fromNumber(to));
}

String &String::arg(float to)
{
	return replaceForArg(argFindMinTag(), String::fromNumber(to));
}

String &String::arg(double to)
{
	return replaceForArg(argFindMinTag(), String::fromNumber(to));
}

void String::setAutoCodec(StringCodec codec)
{
	codecAutoDef = codec;
}

void String::setLocalCodec(StringCodec codec)
{
	codecSysDef = codec;
}

StringCodec String::getAutoCodec()
{
	return codecAutoDef;
}

StringCodec String::getLocalCodec()
{
	return codecSysDef;
}
