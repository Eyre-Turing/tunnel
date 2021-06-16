#ifndef EYRE_STRING_H
#define EYRE_STRING_H

/*
 * For using string easily in Windows and Linux.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-21 12:11.
 */

#include <iostream>
#include <vector>

#define CODEC_AUTO	0
#define CODEC_GBK		1
#define CODEC_UTF8	2

typedef int StringCodec;

class ByteArray;
#include "byte_array.h"

class String
{
public:
	String(const char *str="", StringCodec codec=CODEC_AUTO);
	String(const ByteArray &b, StringCodec codec=CODEC_AUTO);
	String(const String &s);
	virtual ~String();
	
	int indexOf(const String &s, unsigned int offset=0) const;
	int indexOf(const char *str, unsigned int offset=0, StringCodec codec=CODEC_AUTO) const;
	
	int lastIndexOf(const String &s, unsigned int offset=0xffffffff) const;
	int lastIndexOf(const char *str, unsigned int offset=0xffffffff, StringCodec codec=CODEC_AUTO) const;
	
	operator char*();
	operator const char*() const;

	String &operator=(const String &s);
	
	/* 
	 * Will using CODEC_AUTO, if str is a string constant(aka const char*) in code, default
	 * for Windows code save as GBK will normal, for Linux code save as UTF-8.
	 * If str is a variable from a extern file or socket, it's easy to miscode,
	 * because this function don't know what str codec is.
	 * Can use for string constant like: 'String a; a = "hello"',
	 * else not recommended.
	 */
	String &operator=(const char *str);

	static String fromGbk(const char *str);
	static String fromUtf8(const char *str);

	//default for Windows same as fromGbk(const char*), for Linux same as fromUtf8(const char*).
	static String fromLocal(const char *str);

	bool append(const char *str, StringCodec codec=CODEC_AUTO);
	bool append(const String &s);
	bool append(char c);

	//same reason as operator=(const char*), it's easy to miscode.
	String &operator<<(const char *str);

	String &operator<<(const String &s);
	String &operator<<(char c);

	//same reason as operator=(const char*), it's easy to miscode.
	String &operator+=(const char *str);

	String &operator+=(const String &s);
	String &operator+=(char c);

	/*
	 * Same reason as operator=(const char*),
	 * if str's codec isn't auto codec(default: GBK for Windows, UTF8 for Linux),
	 * and str isn't plain English,
	 * will return false.
	 */
	bool operator==(const char *str) const;
	bool operator==(char *str) const;

	bool operator==(const String &s) const;

	//same reason as operator==(const char*), it's easy return true.
	bool operator!=(const char *str) const;
	bool operator!=(char *str) const;

	bool operator!=(const String &s) const;

	unsigned int size() const;

	String &replace(const String &tag, const String &to);
	String &replace(const String &tag, const char *to, StringCodec toCodec=CODEC_AUTO);
	String &replace(const char *tag, const String &to, StringCodec tagCodec=CODEC_AUTO);
	String &replace(const char *tag, const char *to,
					StringCodec tagCodec=CODEC_AUTO, StringCodec toCodec=CODEC_AUTO);
	
	String mid(unsigned int offset, int size=-1) const;
	
	std::vector<String> split(const String &tag) const;
	std::vector<String> split(const char *tag, StringCodec codec=CODEC_AUTO) const;
	
	String &replace(unsigned int offset, unsigned int range, const char *to, StringCodec codec=CODEC_AUTO);
	String &replace(unsigned int offset, unsigned int range, const String &to);
	
	String &insert(unsigned int offset, const char *to, StringCodec codec=CODEC_AUTO);
	String &insert(unsigned int offset, const String &to);
	
	char at(unsigned int pos) const;

	friend std::ostream &operator<<(std::ostream &out, const String &s);
	friend std::istream &operator>>(std::istream &in, String &s);
	
	friend std::istream &getline(std::istream &in, String &s);
	friend std::istream &getline(std::istream &in, String &s, char delim);

	friend String operator+(const String &a, const String &b);

	friend class ByteArray;
	
	bool operator<(const String &s) const;
	bool operator>(const String &s) const;
	bool operator<=(const String &s) const;
	bool operator>=(const String &s) const;
	
	class Iterator
	{
	public:
		Iterator(String *s, unsigned int offset);
		virtual ~Iterator();
		
		operator char() const;
		
		String operator[](int size) const;	//size -1 means all.
		
		Iterator &operator=(char c);
		
		void replace(unsigned int range, const char *to, StringCodec codec=CODEC_AUTO);
		void replace(unsigned int range, const String &to);
		
		void insert(const char *to, StringCodec codec=CODEC_AUTO);
		void insert(const String &to);
		
	private:
		String *m_s;
		unsigned int m_offset;
	};
	
	Iterator operator[](unsigned int offset);
	
	int toInt() const;
	unsigned int toUInt() const;
	long long toInt64() const;
	unsigned long long toUInt64() const;
	float toFloat() const;
	double toDouble() const;
	
	static String fromNumber(int num);
	static String fromNumber(unsigned int num);
	static String fromNumber(long long num);
	static String fromNumber(unsigned long long num);
	static String fromNumber(float num);
	static String fromNumber(double num);
	
	/*
	 * The function arg will replace %[number] to param 'to'.
	 * For example:
	 * ```String("hello %1 world").arg("new");```
	 * wile return "hello new world".
	 *
	 * And you can:
	 * ```String("info:\n  name: %1 ... app_user_name: %1 ...").arg("Eyre");```
	 * will return "info:\n  name: Eyre ... app_user_name: Eyre ...".
	 *
	 * And you can:
	 * ```String("name: %1 %2, first name: %1, last name: %2.").arg("Eyre").arg("Turing");```
	 * will return "name: Eyre Turing, first name: Eyre, last name: Turing.".
	 *
	 * For the %[number], number is a int from 1 to 99.
	 * Note: %1 can not write as %01, else %01 replace will fail.
	 */
	String &arg(const char *to, StringCodec codec=CODEC_AUTO);
	String &arg(const String &to);
	String &arg(int to);
	String &arg(unsigned int to);
	String &arg(long long to);
	String &arg(unsigned long long to);
	String &arg(float to);
	String &arg(double to);
	
	static void setAutoCodec(StringCodec codec);	//redefine CODEC_AUTO.
	static void setLocalCodec(StringCodec codec);	//redefine system codec.
	
	static StringCodec getAutoCodec();
	static StringCodec getLocalCodec();
	
private:
	ByteArray *m_data;
	
	static StringCodec codecAutoDef;
	static StringCodec codecSysDef;
	
	//if not found, return "".
	String argFindMinTag() const;
	String &replaceForArg(const String &tag, const char *to, StringCodec codec=CODEC_AUTO);
	String &replaceForArg(const String &tag, const String &to);
	
	static bool chIsNumber(char ch);
};

std::ostream &operator<<(std::ostream &out, const std::vector<String> &sv);

#endif	//EYRE_STRING_H
