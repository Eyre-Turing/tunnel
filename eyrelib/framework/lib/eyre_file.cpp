/*
 * Class File can read and write file as ByteArray.
 * 
 * Author: Eyre Turing.
 * Last edit: 2021-01-02 16:27.
 */

#include "eyre_file.h"
#include "general.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

File::File(const String &filename)
{
	m_filename = filename;
	m_file = NULL;
	m_openMode = FILE_OPEN_MODE_No;
}

File::File(const File &f)
{
	m_filename = f.m_filename;
	m_file = NULL;
	m_openMode = FILE_OPEN_MODE_No;
}

File &File::operator=(const File &f)
{
	close();
	m_filename = f.m_filename;
	m_file = NULL;
	m_openMode = FILE_OPEN_MODE_No;
	return *this;
}

File::~File()
{
	if(!close())
	{
#if EYRE_DEBUG
		fprintf(stderr, "File(%p) could not close when destroyed!\n", this);
#endif
	}
}

bool File::setFilename(const String &filename)
{
	if(m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p)::setFilename, when file already open!\n", this);
#endif
		return false;
	}
	m_filename = filename;
	return true;
}

String File::getFilename() const
{
	return m_filename;
}

bool File::open(FileOpenMode openMode)
{
	if(m_openMode!=FILE_OPEN_MODE_No || m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p)::open, when file already open!\n", this);
#endif
		return false;
	}
	m_openMode = openMode;
	ByteArray b = ByteArray::fromString(m_filename, CODEC_SYS_DEF);
	if(openMode == FILE_OPEN_MODE_Read)
	{
		m_file = fopen(b, "rb");
	}
	else if(openMode == FILE_OPEN_MODE_Write)
	{
		m_file = fopen(b, "wb");
	}
	else if(openMode == FILE_OPEN_MODE_ReadWrite)
	{
		m_file = fopen(b, "rb+");
	}
	else if(openMode == FILE_OPEN_MODE_Append)
	{
		m_file = fopen(b, "ab");
	}
	if(m_file)
	{
		return true;
	}
	m_openMode = FILE_OPEN_MODE_No;
#if	EYRE_WARNING
	fprintf(stderr, "warning: File(%p)::open fail! open mode: \'%d\'.\n", this, openMode);
#endif
	return false;
}

bool File::close()
{
	if(!m_file)
	{
		m_openMode = FILE_OPEN_MODE_No;
		return true;
	} 
	else if(fclose(m_file) == 0)
	{
		m_file = NULL;
		m_openMode = FILE_OPEN_MODE_No;
		return true;
	}
#if EYRE_DEBUG
	fprintf(stderr, "File(%p)::close fail!\n", this);
#endif
	return false;
}

ByteArray File::read(unsigned int size) const
{
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to read when no open!\n", this);
#endif
		return ByteArray();
	}
	if(m_openMode==FILE_OPEN_MODE_Read || m_openMode==FILE_OPEN_MODE_ReadWrite)
	{
		char *data = (char *) malloc(size);
		unsigned int rsize = fread(data, 1, size, m_file);
		ByteArray result(data, rsize);
		free(data);
		return result;
	}
	else
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to read when no open!\n", this);
#endif
		return ByteArray();
	}
}

bool File::write(const char *data, unsigned int size) const
{
	if(size == 0xffffffff)
	{
		size = strlen(data);
	}
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to write when no open!\n", this);
#endif
		return false;
	}
	if(m_openMode==FILE_OPEN_MODE_Write || m_openMode==FILE_OPEN_MODE_ReadWrite
		|| m_openMode==FILE_OPEN_MODE_Append)
	{
		fwrite(data, 1, size, m_file);
		return true;
	}
	else
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to write when no open!\n", this);
#endif
		return false;
	}
}

bool File::write(const ByteArray &b) const
{
	return write(b, b.size());
}

bool File::isOpen() const
{
	if(m_file)
	{
		return true;
	}
	else
	{
		return false;
	}
}

FileOpenMode File::getOpenMode() const
{
	return m_openMode;
}

ByteArray File::readAll() const
{
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to readAll when no open!\n", this);
#endif
		return ByteArray();
	}
	fseek(m_file, 0, SEEK_END);
	long size = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
	return read(size);
}

long File::getFileSize() const
{
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to getFileSize when no open!\n", this);
#endif
		return 0;
	}
	long pos = ftell(m_file);
	fseek(m_file, 0, SEEK_END);
	long size = ftell(m_file);
	fseek(m_file, pos, SEEK_SET);
	return size;
}

bool File::move(long offset) const
{
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to move when no open!\n", this);
#endif
		return false;
	}
	if(fseek(m_file, offset, SEEK_CUR) != 0)
	{
#if EYRE_DEBUG
		fprintf(stderr, "File(%p)::move fail!\n", this);
#endif 
		return false;
	}
	return true;
}

bool File::moveToBegin() const
{
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to moveToBegin when no open!\n", this);
#endif
		return false;
	}
	if(fseek(m_file, 0, SEEK_SET) != 0)
	{
#if EYRE_DEBUG
		fprintf(stderr, "File(%p)::moveToBegin fail!\n", this);
#endif 
		return false;
	}
	return true;
}

bool File::moveToEnd() const
{
	if(!m_file)
	{
#if EYRE_WARNING
		fprintf(stderr, "warning: File(%p) try to moveToEnd when no open!\n", this);
#endif
		return false;
	}
	if(fseek(m_file, 0, SEEK_END) != 0)
	{
#if EYRE_DEBUG
		fprintf(stderr, "File(%p)::moveToEnd fail!\n", this);
#endif 
		return false;
	}
	return true;
}

long File::getPos() const
{
	return ftell(m_file);
}

File &File::operator<<(const ByteArray &b)
{
	write(b);
	return *this;
}

File &File::operator<<(const char *data)
{
	write(data);
	return *this;
}

bool File::exist(const String &filename) const
{
	if(filename == "")
	{
		if(m_filename == "")
		{
			return false;
		}
		else
		{
			if(access(m_filename, F_OK) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		if(access(filename, F_OK) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
