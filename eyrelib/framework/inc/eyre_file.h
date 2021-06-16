#ifndef EYRE_FILE_H
#define EYRE_FILE_H

/*
 * For read and write file as ByteArray.
 *
 * Author: Eyre Turing.
 * Last edit: 2020-12-26 16:57.
 */

#include "byte_array.h"
#include "eyre_string.h"
#include <stdio.h>

#define FILE_OPEN_MODE_No			0
#define FILE_OPEN_MODE_Read			1
#define FILE_OPEN_MODE_Write		2
#define FILE_OPEN_MODE_ReadWrite	3
#define FILE_OPEN_MODE_Append		4

typedef int FileOpenMode;

class File
{
public:
	File(const String &filename="");
	File(const File &f);
	virtual ~File();
	bool setFilename(const String &filename);
	String getFilename() const;
	bool open(FileOpenMode openMode);
	bool close();
	ByteArray read(unsigned int size) const;
	bool write(const ByteArray &b) const;
	bool write(const char *data, unsigned int size=0xffffffff) const;
	bool isOpen() const;
	FileOpenMode getOpenMode() const;
	
	ByteArray readAll() const;
	long getFileSize() const;
	
	bool move(long offset) const;
	bool moveToBegin() const;
	bool moveToEnd() const;
	
	long getPos() const;
	
	File &operator<<(const ByteArray &b);
	File &operator<<(const char *data);
	
	File &operator=(const File &f);
	
	bool exist(const String &filename="") const;
	
private:
	String m_filename;
	FILE *m_file;
	FileOpenMode m_openMode;
};

#endif	//EYRE_FILE_H
