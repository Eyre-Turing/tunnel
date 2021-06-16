#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

/*
 * For save byte array data. Thread unsafe.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-15 12:00.
 */

#include <iostream>
#include <vector>

class String;
#include "eyre_string.h"

class ByteArray
{
public:
	ByteArray(unsigned int serve = 0xffffffff);
	ByteArray(const ByteArray &b);
	ByteArray(const char *str, unsigned int size = 0xffffffff);
	
	virtual ~ByteArray();
	bool append(const ByteArray &b);
	bool append(const char *str, unsigned int size = 0xffffffff);
	bool append(char c);
	
	operator char*();
	operator const char*() const;
	ByteArray &operator<<(const ByteArray &b);
	ByteArray &operator<<(const char *str);
	ByteArray &operator<<(char c);

	ByteArray &operator+=(const ByteArray &b);
	ByteArray &operator+=(const char *str);
	ByteArray &operator+=(char c);

	ByteArray &operator=(const ByteArray &b);
	ByteArray &operator=(const char *str);

	bool operator==(const ByteArray &b) const;
	bool operator==(const char *str) const;
	bool operator==(char *str) const; 

	bool operator!=(const ByteArray &b) const;
	bool operator!=(const char *str) const;
	bool operator!=(char *str) const;

	bool reserve(unsigned int s);
	unsigned int serve() const;
	unsigned int size() const;

	int indexOf(const ByteArray &b, unsigned int offset = 0) const;
	int indexOf(const char *str, unsigned int offset = 0, unsigned int size = 0xffffffff) const;
	
	int lastIndexOf(const ByteArray &b, unsigned int offset = 0xffffffff) const;
	int lastIndexOf(const char *str, unsigned int offset = 0xffffffff, unsigned int size = 0xffffffff) const;

	ByteArray &replace(const ByteArray &tag, const ByteArray &to);
	ByteArray &replace(const ByteArray &tag, const char *to, unsigned int tosize=0xffffffff);
	ByteArray &replace(const char *tag, const ByteArray &to, unsigned int tagsize=0xffffffff);
	ByteArray &replace(	const char *tag, const char *to,
					unsigned int tagsize=0xffffffff, unsigned int tosize=0xffffffff);

	friend ByteArray operator+(const ByteArray &a, const ByteArray &b);
	friend ByteArray operator+(const ByteArray &a, const char *str);
	friend ByteArray operator+(const char *str, const ByteArray &b);

	friend ByteArray operator+(const ByteArray &a, char b);
	friend ByteArray operator+(char a, const ByteArray &b);
	
	ByteArray &replace(unsigned int offset, unsigned int range, const char *to, unsigned int tosize=0xffffffff);
	ByteArray &replace(unsigned int offset, unsigned int range, const ByteArray &to);
	
	ByteArray &insert(unsigned int offset, const char *to, unsigned int tosize=0xffffffff);
	ByteArray &insert(unsigned int offset, const ByteArray &to);
	
	char at(unsigned int pos) const;
	
	ByteArray mid(unsigned int offset, int size=-1) const;
	
	std::vector<ByteArray> split(const char *tag, unsigned int tagsize=0xffffffff) const;
	std::vector<ByteArray> split(const ByteArray &tag) const;

	friend std::ostream &operator<<(std::ostream &out, const ByteArray &b);

	String toString(StringCodec codec = CODEC_AUTO) const;
	static ByteArray fromString(const String &s, StringCodec codec = CODEC_AUTO);

	friend class String;
	friend std::ostream &operator<<(std::ostream &out, const String &s);
	
	bool operator<(const ByteArray &b) const;
	bool operator>(const ByteArray &b) const;
	bool operator<=(const ByteArray &b) const;
	bool operator>=(const ByteArray &b) const;
	
	class Iterator
	{
	public:
		Iterator(ByteArray *b, unsigned int offset);
		virtual ~Iterator();
		
		operator char() const;
		
		ByteArray operator[](int size) const;	//size -1 means all.
		
		Iterator &operator=(char c);
		
		void replace(unsigned int range, const char *to, unsigned int tosize=0xffffffff);
		void replace(unsigned int range, const ByteArray &to);
		
		void insert(const char *to, unsigned int tosize=0xffffffff);
		void insert(const ByteArray &to);
		
	private:
		ByteArray *m_b;
		unsigned int m_offset;
		
	};
	
	Iterator operator[](unsigned int offset);
	
private:
	char *m_data;
	unsigned int m_size;
	unsigned int m_serve;

	bool set(const ByteArray &b);
	bool set(const char *str, unsigned int size = 0xffffffff);
	
};

std::ostream &operator<<(std::ostream &out, const std::vector<ByteArray> &sv);

#endif	//BYTE_ARRAY_H
