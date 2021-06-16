#ifndef GENERAL_H
#define GENERAL_H

#include <stdio.h>
#include <vector>

#ifndef EYRE_DETAIL
#define EYRE_DETAIL	0
#endif	//EYRE_DETAIL

#ifndef EYRE_DEBUG
#define EYRE_DEBUG	0
#endif	//EYRE_DEBUG

#ifndef EYRE_WARNING
#define EYRE_WARNING	0
#endif	//EYRE_WARNING

#ifndef EYRE_BA_SERVE
#define EYRE_BA_SERVE	1024
#endif	//EYRE_BA_SERVE

#ifndef CODEC_SYS_DEF
#ifdef _WIN32
#define CODEC_SYS_DEF	CODEC_GBK
#else
#define CODEC_SYS_DEF	CODEC_UTF8
#endif
#endif	//CODEC_SYS_DEF

#ifndef CODEC_AUTO_DEF
#define CODEC_AUTO_DEF	CODEC_SYS_DEF
#endif	//CODEC_AUTO_DEF

namespace EyreFrameworkGeneral
{
char *byteReplace(	const char *src, const char *tag, const char *to,
			unsigned int srcsize, unsigned int tagsize, unsigned int tosize,
			unsigned int &resultSize, unsigned int &resultServe);	//result need free.
			
char *byteChange(	const char *src, const char *to, unsigned int srcsize, unsigned int tosize,
					unsigned int srcOffset, unsigned int srcRange,
					unsigned int &resultSize, unsigned int &resultServe);	//result need free.

std::vector<int> kmpGetNext(const char *tag, unsigned int tagsize);
int kmpSearch(	const char *src, const char *tag, unsigned int srcsize, unsigned int tagsize,
			unsigned int offset=0);
int kmpSearch(	const char *src, const char *tag, unsigned int srcsize, unsigned int tagsize,
			const std::vector<int> &next, unsigned int offset=0);

#ifdef _WIN32
char *GbkToUtf8(const char *src_str);	//result need free.
char *Utf8ToGbk(const char *src_str);	//result need free.
#else
int GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len);
int Utf8ToGbk(char *src_str, size_t src_len, char *dst_str, size_t dst_len);
#endif

char *gbkToUtf8(const char *gbk_str);	//result need free.
char *utf8ToGbk(const char *utf8_str);	//result need free.
}

using namespace EyreFrameworkGeneral;
#endif	//GENERAL_H
