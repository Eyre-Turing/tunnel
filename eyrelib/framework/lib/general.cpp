/*
 * Some general algorithm.
 * 
 * Author: Eyre Turing.
 * Last edit: 2021-01-15 11:46.
 */

#include "general.h"
#include <string.h>
#include <stdlib.h>

char *EyreFrameworkGeneral::byteReplace(	const char *src, const char *tag, const char *to,
			unsigned int srcsize, unsigned int tagsize, unsigned int tosize,
			unsigned int &resultSize, unsigned int &resultServe)
{
#if EYRE_DETAIL
	fprintf(stdout, "byteReplace(const char *src, const char *tag, const char *to,\n"
					"            unsigned int srcsize, unsigned int tagsize, unsigned int tosize,\n"
					"            unsigned int &resultSize, unsigned int &resultServe)\n"
					"srcsize: %d, tagsize: %d, tosize: %d\n", srcsize, tagsize, tosize);
	
	fprintf(stdout, "src: (");
	for(unsigned int i=0; i<srcsize; ++i)
	{
		if(i)
		{
			fprintf(stdout, ", ");
		}
		fprintf(stdout, "%d", (int) src[i]);
	}
	fprintf(stdout, ")\n");
	
	fprintf(stdout, "tag: (");
	for(unsigned int i=0; i<tagsize; ++i)
	{
		if(i)
		{
			fprintf(stdout, ", ");
		}
		fprintf(stdout, "%d", (int) tag[i]);
	}
	fprintf(stdout, ")\n");
	
	fprintf(stdout, "to: (");
	for(unsigned int i=0; i<tosize; ++i)
	{
		if(i)
		{
			fprintf(stdout, ", ");
		}
		fprintf(stdout, "%d", (int) to[i]);
	}
	fprintf(stdout, ")\n");
#endif
	int cnt = 0;
	
	std::vector<int> next = kmpGetNext(tag, tagsize);
	int index = -tagsize;
	while((index=kmpSearch(src, tag, srcsize, tagsize, next, index+tagsize)) != -1)
	{
		++cnt;
	}
	
#if EYRE_DETAIL
	fprintf(stdout, "next: (");
	for(unsigned int i=0; i<tagsize; ++i)
	{
		if(i)
		{
			fprintf(stdout, ", ");
		}
		fprintf(stdout, "%d", next[i]);
	}
	fprintf(stdout, ")\ncnt: %d\n", cnt);
#endif
	
	resultSize = srcsize+(tosize-tagsize)*cnt;
	
#if EYRE_DETAIL
	fprintf(stdout, "resultSize: %d\n", resultSize);
#endif
	
	resultServe = resultSize+1;
	
#if EYRE_DETAIL
	fprintf(stdout, "resultServe: %d\n", resultServe);
#endif
	
	char *result = (char *) malloc(resultServe);
	
#if EYRE_DETAIL
	fprintf(stdout, "result: %p\n", result);
#endif
	
	int ci = 0;
	int index_;
	index = -tagsize;
	do
	{
		index_ = index+tagsize;
		index = kmpSearch(src, tag, srcsize, tagsize, next, index_);
		if(index != -1)
		{
#if EYRE_DETAIL
			fprintf(stdout, "find tag, index_: %d, index: %d\n"
						"(tosize-tagsize)*ci: %d\n", index_, index, (tosize-tagsize)*ci);
#endif
			memcpy(result+(index_+(tosize-tagsize)*ci), src+index_, index-index_);
			memcpy(result+(index+(tosize-tagsize)*ci), to, tosize);
		}
		else
		{
#if EYRE_DETAIL
			fprintf(stdout, "no tag, index_: %d, index: %d\n"
						"(tosize-tagsize)*ci: %d\n", index_, index, (tosize-tagsize)*ci);
#endif
			memcpy(result+(index_+(tosize-tagsize)*ci), src+index_, srcsize-index_);
		}
		
		++ci;
	} while(index != -1);
	result[resultSize] = 0;
	
	return result;
}

char *EyreFrameworkGeneral::byteChange(	const char *src, const char *to, unsigned int srcsize, unsigned int tosize,
					unsigned int srcOffset, unsigned int srcRange,
					unsigned int &resultSize, unsigned int &resultServe)
{
	if(srcOffset > srcsize)
	{
		srcOffset = srcsize;
		srcRange = 0;
	}
	else if(srcOffset+srcRange > srcsize)
	{
		srcRange = srcsize-srcOffset;
	}
	
	resultSize = srcsize+(tosize-srcRange);
	resultServe = resultSize+1;
	char *result = (char *) malloc(resultServe);
	memcpy(result, src, srcOffset);
	memcpy(result+srcOffset, to, tosize);
	memcpy(result+(srcOffset+tosize), src+(srcOffset+srcRange), srcsize-srcOffset-srcRange);
	result[resultSize] = 0;
	return result;
}

std::vector<int> EyreFrameworkGeneral::kmpGetNext(const char *tag, unsigned int tagsize)
{
	std::vector<int> next;
	next.resize(tagsize+1);
	next[0] = -1;
	int k = -1;
	for(int q=1; q<tagsize; ++q)
	{
		while (k>-1 && tag[k+1]!=tag[q])
		{
			k = next[k];
		}
		if (tag[k+1] == tag[q])
		{
			++k;
		}
		next[q] = k;
	}
	return next;
}

int EyreFrameworkGeneral::kmpSearch(const char *src, const char *tag, unsigned int srcsize, unsigned int tagsize, unsigned int offset)
{
	std::vector<int> next = kmpGetNext(tag, tagsize);
	return kmpSearch(src, tag, srcsize, tagsize, next, offset);
}

int EyreFrameworkGeneral::kmpSearch(	const char *src, const char *tag, unsigned int srcsize, unsigned int tagsize,
			const std::vector<int> &next, unsigned int offset)
{
	int k = -1;
	for (int i=offset; i<srcsize; ++i)
	{
 		while(k>-1 && tag[k+1]!=src[i])
		{
			k = next[k];
		}
		if(tag[k+1] == src[i])
		{
			++k;
		}
		if(k == tagsize-1)
		{
			return i-tagsize+1;
		}
	}
	return -1;
}

#ifdef _WIN32
#include <windows.h>

char *EyreFrameworkGeneral::GbkToUtf8(const char *src_str)
{
	//int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	int len = MultiByteToWideChar(936, 0, src_str, -1, NULL, 0);
	//wchar_t* wstr = new wchar_t[len + 1];
	wchar_t *wstr = (wchar_t *) malloc(sizeof(wchar_t)*(len+1));
	memset(wstr, 0, len + 1);
	//MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	MultiByteToWideChar(936, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	//char* str = new char[len + 1];
	char *str = (char *) malloc(len+1);
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	//std::string strTemp = str;
	//delete[] wstr;
	//delete[] str;
	//return strTemp;
	free(wstr);
	return str;
}

char *EyreFrameworkGeneral::Utf8ToGbk(const char *src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	//wchar_t* wszGBK = new wchar_t[len + 1];
	wchar_t *wszGBK = (wchar_t *) malloc(sizeof(wchar_t)*(len+1));
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	//len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	len = WideCharToMultiByte(936, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	//char* szGBK = new char[len + 1];
	char *szGBK = (char *) malloc(len+1);
	memset(szGBK, 0, len + 1);
	//WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	WideCharToMultiByte(936, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	//std::string strTemp(szGBK);
	//delete[] wszGBK;
	//delete[] szGBK;
	//return strTemp;
	free(wszGBK);
	return szGBK;
}
#else
#include <iconv.h>

int EyreFrameworkGeneral::GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len)
{
	iconv_t cd;
	char **pin = &str_str;
	char **pout = &dst_str;

	cd = iconv_open("utf8", "gbk");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
		return -1;
	iconv_close(cd);
	**pout = '\0';

	return 0;
}

int EyreFrameworkGeneral::Utf8ToGbk(char *src_str, size_t src_len, char *dst_str, size_t dst_len)
{
	iconv_t cd;
	char **pin = &src_str;
	char **pout = &dst_str;

	cd = iconv_open("gbk", "utf8");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
		return -1;
	iconv_close(cd);
	**pout = '\0';

	return 0;
}
#endif

char *EyreFrameworkGeneral::gbkToUtf8(const char *gbk_str)
{
#ifdef _WIN32
//	std::string utf8_str = GbkToUtf8(gbk_str);
//	const char *utf8_str_c = utf8_str.c_str();
//	unsigned int len = strlen(utf8_str_c);
//	char *result = (char *) malloc(len+1);
//	memcpy(result, utf8_str_c, len);
//	result[len] = 0;
//	return result;
	return GbkToUtf8(gbk_str);
#else
	unsigned int gbk_strLen = strlen(gbk_str);
	unsigned int boundLen = gbk_strLen*3;	//in gbk one word most use 2 byte, in utf8 one word most use 6 byte, so 3 times.
	char *boundResult = (char *) malloc(boundLen+1);
	memset(boundResult, 0, boundLen+1);
	char *gbk_strc = (char *) malloc(gbk_strLen+1);
	memcpy(gbk_strc, gbk_str, gbk_strLen);
	gbk_strc[gbk_strLen] = 0;
	GbkToUtf8(gbk_strc, gbk_strLen, boundResult, boundLen);
	unsigned int len = strlen(boundResult);
	char *result = (char *) realloc(boundResult, len+1);
	free(gbk_strc);
	return result;
#endif
}

char *EyreFrameworkGeneral::utf8ToGbk(const char *utf8_str)
{
#ifdef _WIN32
//	std::string gbk_str = Utf8ToGbk(utf8_str);
//	const char *gbk_str_c = gbk_str.c_str();
//	unsigned int len = strlen(gbk_str_c);
//	char *result = (char *) malloc(len+1);
//	memcpy(result, gbk_str_c, len);
//	result[len] = 0;
//	return result;
	return Utf8ToGbk(utf8_str);
#else
	unsigned int utf8_strLen = strlen(utf8_str);
	unsigned int boundLen = utf8_strLen;
	char *boundResult = (char *) malloc(boundLen+1);
	memset(boundResult, 0, boundLen+1);
	char *utf8_strc = (char *) malloc(utf8_strLen+1);
	memcpy(utf8_strc, utf8_str, utf8_strLen);
	utf8_strc[utf8_strLen] = 0;
	Utf8ToGbk(utf8_strc, utf8_strLen, boundResult, boundLen);
	unsigned int len = strlen(boundResult);
	char *result = (char *) realloc(boundResult, len+1);
	free(utf8_strc);
	return result;
#endif
}
