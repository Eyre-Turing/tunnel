/*
 * Class ByteArray can save any data as byte array.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-21 12:19.
 */

#include "byte_array.h"
#include "general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#define NEED_ADD	1	//means m_serve is at least how much bigger than m_size.

ByteArray::ByteArray(unsigned int serve) : m_serve(serve), m_size(0)
{
	if(m_serve == 0xffffffff)
	{
		m_serve = EYRE_BA_SERVE;
	}
	else
	{
		m_serve += NEED_ADD;
	}
	m_data = (char *) malloc(m_serve);

	if(m_data)
	{
		memset(m_data, 0, m_serve);
	}
	else
	{
		m_serve = 0;
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray(%p) can not malloc!\n", this);
#endif
	}

#if EYRE_DETAIL
	fprintf(stdout, "ByteArray(%p) created, serve: %d.\n", this, m_serve);
#endif
}

bool ByteArray::set(const ByteArray &b)
{
	return set(b.m_data, b.m_size);
}

bool ByteArray::set(const char *str, unsigned int size)
{
	free(m_data);

	if(size == 0xffffffff)
	{
		size = strlen(str);
	}
	m_size = size;
	m_serve = size+NEED_ADD;
	m_data = (char *) malloc(m_serve);

	bool status = true;

	if(m_data)
	{
		memcpy(m_data, str, size);
		memset(m_data+size, 0, m_serve-size);
	}
	else
	{
		m_serve = m_size = 0;
		status = false;
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray(%p) can not malloc!\n", this);
#endif
	}

	return status;
}

ByteArray::ByteArray(const ByteArray &b)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray(const ByteArray&)\n");
#endif
	m_data = NULL;

	set(b);

#if EYRE_DETAIL
	fprintf(stdout, "ByteArray(%p) copy.\n", this);
#endif
}

ByteArray::ByteArray(const char *str, unsigned int size)
{
	m_data = NULL;

	set(str, size);

#if EYRE_DETAIL
	fprintf(stdout, "ByteArray(%p) copy str.\n", this);
#endif
}

ByteArray::~ByteArray()
{
	free(m_data);
	m_data = NULL;
	m_size = 0;
	m_serve = 0;
	
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray(%p) destroyed.\n", this);
#endif
}

bool ByteArray::append(const ByteArray &b)
{
	return append(b.m_data, b.m_size);
}

bool ByteArray::append(const char *str, unsigned int size)
{
	if(size == 0xffffffff)
	{
		size = strlen(str);
	}
	unsigned int serve = m_serve;
	unsigned int needSize = m_size+size;
	if(serve < needSize+NEED_ADD)
	{
		if(serve*2 < needSize+NEED_ADD)
		{
			serve = needSize+NEED_ADD;
		}
		else
		{
			serve *= 2;
		}
		
		if(reserve(serve))
		{
			memcpy(m_data+m_size, str, size);
			//memset(m_data+m_size+size, 0, m_serve-m_size-size);
			m_size = needSize;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		memcpy(m_data+m_size, str, size);
		//memset(m_data+m_size+size, 0, m_serve-m_size-size);
		m_size = needSize;
		return true;
	}
}

bool ByteArray::append(char c)
{
	char str[2];
	str[0] = c;
	str[1] = 0;
	return append(str, 1);
}

ByteArray::operator char*()
{
	return m_data;
}

ByteArray::operator const char*() const
{
	return (const char *) m_data;
}

ByteArray &ByteArray::operator<<(const ByteArray &b)
{
	if(!append(b))
	{
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray(%p) operator<<(const ByteArray &b) error!\n", this);
#endif
	}
	return *this;
}

ByteArray &ByteArray::operator<<(const char *str)
{
	if(!append(str))
	{
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray(%p) operator<<(const char *str) error!\n", this);
#endif
	}
	return *this;
}

ByteArray &ByteArray::operator<<(char c)
{
	if(!append(c))
	{
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray(%p) operator<<(char c) error!\n", this);
#endif
	}
	return *this;
}

ByteArray &ByteArray::operator+=(const ByteArray &b)
{
	return (*this)<<b;
}

ByteArray &ByteArray::operator+=(const char *str)
{
	return (*this)<<str;
}

ByteArray &ByteArray::operator+=(char c)
{
	return (*this)<<c;
}

ByteArray operator+(const ByteArray &a, const ByteArray &b)
{
	ByteArray result(a);
	result.append(b);
	return result;
}

ByteArray operator+(const ByteArray &a, const char *str)
{
	ByteArray result(a);
	result.append(str);
	return result;
}

ByteArray operator+(const char *str, const ByteArray &b)
{
	ByteArray result(str);
	result.append(b);
	return result;
}

ByteArray operator+(const ByteArray &a, char b)
{
	ByteArray result(a);
	result.append(b);
	return result;
}

ByteArray operator+(char a, const ByteArray &b)
{
	char str[2];
	str[0] = a;
	str[1] = 0;
	ByteArray result(str);
	result.append(b);
	return result;
}

ByteArray &ByteArray::operator=(const ByteArray &b)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::operator=(const ByteArray&)\n");
#endif
	set(b);
	return *this;
}

ByteArray &ByteArray::operator=(const char *str)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::operator=(const char*)\n");
#endif
	set(str);
	return *this;
}

bool ByteArray::operator==(const ByteArray &b) const
{
	if(m_size != b.m_size)
	{
		return false;
	}
	return memcmp(m_data, b.m_data, m_size) == 0;
}

bool ByteArray::operator==(const char *str) const
{
	unsigned int len = strlen(str);
	if(m_size < len)
	{
		return false;
	}
	for(unsigned int i=0; i<len; ++i)
	{
		if(m_data[i] != str[i])
		{
			return false;
		}
	}
	for(unsigned int i=len; i<m_size; ++i)
	{
		if(m_data[i] != 0)
		{
			return false;
		}
	}
	return true;
}

bool ByteArray::operator==(char *str) const
{
	return (*this)==(const char *) str;
}

bool ByteArray::operator!=(const ByteArray &b) const
{
	return !((*this) == b);
}

bool ByteArray::operator!=(const char *str) const
{
	return !((*this) == str);
}

bool ByteArray::operator!=(char *str) const
{
	return !((*this) == str);
}

bool ByteArray::reserve(unsigned int s)
{
	char *temp_data = (char *) realloc(m_data, s);
	if(temp_data)
	{
		m_data = temp_data;
		m_serve = s;
		if(m_size > m_serve-NEED_ADD)
		{
#if EYRE_WARNING
			fprintf(stderr, "warning: ByteArray(%p) leak! size: %d, serve: %d.\n",
					this, m_size, m_serve);
#endif
			m_size = m_serve-NEED_ADD;
		}
		memset(m_data+m_size, 0, m_serve-m_size);
		return true;
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray(%p) can not realloc!\n", this);
#endif
		return false;
	}
}

unsigned int ByteArray::serve() const
{
	return m_serve;
}

unsigned int ByteArray::size() const
{
	return m_size;
}

std::ostream &operator<<(std::ostream &out, const ByteArray &b)
{
	out<<"ByteArray(";
	for(unsigned int i=0; i<b.m_size; ++i)
	{
		if(i)
		{
			out<<", ";
		}
		out<<(int) b.m_data[i];
	}
	out<<")";
	return out;
}

int ByteArray::indexOf(const ByteArray &b, unsigned int offset) const
{
	return indexOf(b.m_data, offset, b.m_size);
}

int ByteArray::indexOf(const char *str, unsigned int offset, unsigned int size) const
{
	if(size == 0xffffffff)
	{
		size = strlen(str);
	}
	if(size == 0)
	{
		return -1;
	}
	return kmpSearch(m_data, str, m_size, size, offset);
}

int ByteArray::lastIndexOf(const ByteArray &b, unsigned int offset) const
{
	return lastIndexOf(b.m_data, offset, b.m_size);
}

int ByteArray::lastIndexOf(const char *str, unsigned int offset, unsigned int size) const
{
	if(size == 0xffffffff)
	{
		size = strlen(str);
	}
	if(size == 0)
	{
		return -1;
	}
	if(offset == 0xffffffff)
	{
		offset = m_size-1;
	}
	offset = m_size-offset;
	if(offset >= size)
	{
		offset -= size;
	}
	else
	{
		offset = 0;
	}
	char *data = (char *) malloc(m_size);
	memcpy(data, m_data, m_size);
	std::reverse(data, data+m_size);
	char *tag = (char *) malloc(size);
	memcpy(tag, str, size);
	std::reverse(tag, tag+size);
	
	int reverseIndex = kmpSearch(data, tag, m_size, size, offset);
	if(reverseIndex == -1)
	{
		return -1;
	}
	
	return m_size-reverseIndex-size;
}

ByteArray &ByteArray::replace(const ByteArray &tag, const ByteArray &to)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::replace(const ByteArray&, const ByteArray&)\n");
#endif
	return replace(tag.m_data, to.m_data, tag.m_size, to.m_size);
}

ByteArray &ByteArray::replace(const ByteArray &tag, const char *to, unsigned int tosize)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::replace(const ByteArray&, const char*, unsigned int)\n");
#endif
	if(tosize == 0xffffffff)
	{
		tosize = strlen(to);
	}
	return replace(tag.m_data, to, tag.m_size, tosize);
}

ByteArray &ByteArray::replace(const char *tag, const ByteArray &to, unsigned int tagsize)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::replace(const char*, const ByteArray&, unsigned int)\n");
#endif
	if(tagsize == 0xffffffff)
	{
		tagsize = strlen(tag);
	}
	return replace(tag, to.m_data, tagsize, to.m_size);
}

ByteArray &ByteArray::replace(const char *tag, const char *to, unsigned int tagsize, unsigned int tosize)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::replace(const char*, const char*, unsigned int, unsigned int)\n");
#endif
	if(tagsize == 0xffffffff)
	{
		tagsize = strlen(tag);
	}
	if(tosize == 0xffffffff)
	{
		tosize = strlen(to);
	}
	if(tagsize == 0)
	{
		return *this;
	}
	unsigned int size, serve;
	char *data = byteReplace(m_data, tag, to, m_size, tagsize, tosize, size, serve);
	free(m_data);
	m_data = data;
	m_size = size;
	m_serve = serve;
	return *this;
}

String ByteArray::toString(StringCodec codec) const
{
	return String(m_data, codec);
}

//codec is what codec want convert into. note: String m_data was save as utf-8.
ByteArray ByteArray::fromString(const String &s, StringCodec codec)
{
	if(codec == CODEC_AUTO)
	{
		//codec = CODEC_AUTO_DEF;
		codec = String::codecAutoDef;
	}
	if(codec == CODEC_GBK)
	{
		char *str = utf8ToGbk(s.m_data->m_data);
		ByteArray result(str);
		free(str);
		return result;
	}
	else if(codec == CODEC_UTF8)
	{
		return ByteArray(s.m_data->m_data);
	}
	else
	{
#if EYRE_DEBUG
		fprintf(stderr, "ByteArray::fromString unknow codec: \'%d\'!\n", codec);
#endif
		return ByteArray("");
	}
}

ByteArray::Iterator ByteArray::operator[](unsigned int offset)
{
	if(offset > m_size)
	{
		offset = 0;
	}
	return Iterator(this, offset);
}

ByteArray &ByteArray::replace(unsigned int offset, unsigned int range, const char *to, unsigned int tosize)
{
	if(tosize == 0xffffffff)
	{
		tosize = strlen(to);
	}
	unsigned int size, serve;
	char *data = byteChange(m_data, to, m_size, tosize, offset, range, size, serve);
	free(m_data);
	m_data = data;
	m_size = size;
	m_serve = serve;
	return *this;
}

ByteArray &ByteArray::replace(unsigned int offset, unsigned int range, const ByteArray &to)
{
	return replace(offset, range, to.m_data, to.m_size);
}

ByteArray &ByteArray::insert(unsigned int offset, const char *to, unsigned int tosize)
{
	return replace(offset, 0, to, tosize);
}

ByteArray &ByteArray::insert(unsigned int offset, const ByteArray &to)
{
	return replace(offset, 0, to);
}

char ByteArray::at(unsigned int pos) const
{
	if(pos < m_size)
	{
		return m_data[pos];
	}
	else
	{
		return 0;
	}
}

ByteArray ByteArray::mid(unsigned int offset, int size) const
{
	if(size<0 || size>m_size-offset)	//return [offset, m_size)
	{
		size = m_size-offset;
	}
	//return [offset, offset+size)
	return ByteArray(m_data+offset, size);
}

std::vector<ByteArray> ByteArray::split(const char *tag, unsigned int tagsize) const
{
	if(tagsize == 0xffffffff)
	{
		tagsize = strlen(tag);
	}
	int index_;
	int index = -tagsize;
	std::vector<ByteArray> result;
	do
	{
		index_ = index+tagsize;
		index = indexOf(tag, index_, tagsize);
		if(index != -1)
		{
			//C-Type, [index_, index) is one part need append to result.
			result.push_back(mid(index_, index-index_));
		}
	} while(index != -1);
	//[index_, m_size) is the last part need to append.
	result.push_back(mid(index_, m_size-index_));
	return result;
}

std::vector<ByteArray> ByteArray::split(const ByteArray &tag) const
{
	return split(tag.m_data, tag.m_size);
}

bool ByteArray::operator<(const ByteArray &b) const
{
	unsigned int len = m_size;
	if(len > b.m_size)
	{
		len = b.m_size;
	}
	len += NEED_ADD;
	return memcmp(m_data, b.m_data, len)<0;
}

bool ByteArray::operator>(const ByteArray &b) const
{
	return b<(*this);
}

bool ByteArray::operator<=(const ByteArray &b) const
{
	return !((*this)>b);
}

bool ByteArray::operator>=(const ByteArray &b) const
{
	return !((*this)<b);
}

ByteArray::Iterator::Iterator(ByteArray *b, unsigned int offset) : m_b(b), m_offset(offset)
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::Iterator(%p) created.\n", this);
#endif
}

ByteArray::Iterator::~Iterator()
{
#if EYRE_DETAIL
	fprintf(stdout, "ByteArray::Iterator(%p) destroyed.\n", this);
#endif
}

ByteArray::Iterator::operator char() const
{
	return m_b->m_data[m_offset];
}

ByteArray ByteArray::Iterator::operator[](int size) const 
{
	return m_b->mid(m_offset, size);
}

ByteArray::Iterator &ByteArray::Iterator::operator=(char c)
{
	if(m_offset >= m_b->m_size)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: subscript overstep, reject!\n");
#endif
		return *this;
	} 
	m_b->m_data[m_offset] = c;
	return *this;
}

void ByteArray::Iterator::replace(unsigned int range, const char *to, unsigned int tosize)
{
	m_b->replace(m_offset, range, to, tosize);
}

void ByteArray::Iterator::replace(unsigned int range, const ByteArray &to)
{
	replace(range, to.m_data, to.m_size);
}

void ByteArray::Iterator::insert(const char *to, unsigned int tosize)
{
	replace(0, to, tosize);
}

void ByteArray::Iterator::insert(const ByteArray &to)
{
	replace(0, to);
}

std::ostream &operator<<(std::ostream &out, const std::vector<ByteArray> &bv)
{
	unsigned int size = bv.size();
	out<<"ByteArrayVector(";
	for(unsigned int i=0; i<size; ++i)
	{
		if(i)
		{
			out<<", ";
		}
		out<<bv[i];
	}
	out<<")";
	return out;
}
