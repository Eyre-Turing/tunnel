/*
 * Class IniSettings can read and write ini format configure file.
 *
 * Class IniParse can parse ini format configure String.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-05 15:17.
 */

#include "ini_settings.h"

#define INI_KEY_EXIST			0
#define INI_NO_PARENT			1
#define INI_NO_CHILD			2
#define INI_KEY_FORMAT_ERROR	3

#define INI_PARENT_EXIST		0

IniParse::IniParse(const String &text)
{
	m_text = text;
}

IniParse::IniParse(const IniParse &p)
{
	m_text = p.m_text;
}

IniParse::~IniParse()
{
	
}

void IniParse::setText(const String &text)
{
	m_text = text;
	m_text.replace("\r", "", CODEC_UTF8, CODEC_UTF8);
	while(m_text.indexOf("\n\n") != -1)
	{
		m_text.replace("\n\n", "\n", CODEC_UTF8, CODEC_UTF8);
	}
	if(m_text.size()!=0 && m_text[m_text.size()-1]!='\n')
	{
		m_text += '\n';
	}
}

String IniParse::getText() const
{
	return m_text;
}

/*
 * If parent exist, parentOffset is parent's char '[' pos in m_text,
 * parentIndex is before the first child,
 * and parentEnd is next to the parent's last char in m_text, and
 * return INI_PARENT_EXIST.
 *
 * If parent don't exist, parentOffset=parentIndex=parentEnd is the pos 
 * if you want to append this parent, and return INI_NO_PARENT.
 */
int IniParse::parentPosRange(	const String &parent, unsigned int &parentOffset,
								unsigned int &parentIndex, unsigned int &parentEnd) const
{
	String parentExtern = "["+parent+"]";
	parentOffset = -1;
	do
	{
		parentOffset = (unsigned int) m_text.indexOf(parentExtern, parentOffset+1);
		if(parentOffset == (unsigned int) -1)
		{
			parentOffset = parentIndex = parentEnd = m_text.size();
			return INI_NO_PARENT;
		}
	} while((parentOffset>0 && m_text.at(parentOffset-1)!='\n') || m_text.at(parentOffset+parentExtern.size())!='\n');
	parentIndex = parentOffset+parentExtern.size();
	parentEnd = (unsigned int) m_text.indexOf("\n[", parentIndex);
	if(parentEnd == (unsigned int) -1)
	{
		parentEnd = m_text.size();
	}
	else
	{
		++parentEnd;
	}
	return INI_PARENT_EXIST;
}

/*
 * If param key format error, return INI_KEY_FORMAT_ERROR, and don't change
 * childOffset, valOffset and valEnd.
 *
 * If key exist, childOffset is child key's pos in m_text,
 * valOffset is value's pos in m_text, valEnd is value's end pos in m_text.
 * And return INI_KEY_EXIST.
 *
 * If key don't exist, childOffset=valOffset=valEnd is the pos if you want 
 * to append this configure item.
 * Note: if append a configure item, must add a '\n' as end char.
 * If parent key don't exist, return INI_NO_PARENT, else if child key don't
 * exist, return INI_NO_CHILD.
 */
int IniParse::valueIndex(	const String &key, unsigned int &childOffset,
							unsigned int &valOffset, unsigned int &valEnd) const
{
	std::vector<String> keys = key.split("/");
	if(keys.size() != 2)
	{
		return INI_KEY_FORMAT_ERROR;
	}
//	String parentExtern = "["+keys[0]+"]";
//	int parentIndex = m_text.indexOf(parentExtern);
//	if(parentIndex == -1)
//	{
//		childOffset = valOffset = valEnd = m_text.size();
//		return INI_NO_PARENT;
//	}
//	parentIndex += parentExtern.size();
//	int parentEnd = m_text.indexOf("[", parentIndex);
//	if(parentEnd == -1)
//	{
//		parentEnd = m_text.size();
//	}
	unsigned int parentOffset, parentIndex, parentEnd;
	if(parentPosRange(keys[0], parentOffset, parentIndex, parentEnd) != INI_PARENT_EXIST)
	{
		childOffset = valOffset = valEnd = m_text.size();
		return INI_NO_PARENT;
	}
	
	String parentRange = m_text.mid(parentIndex, parentEnd-parentIndex);
	String childExtern = "\n"+keys[1]+"=";
	int childIndex = parentRange.indexOf(childExtern);
	if(childIndex == -1)
	{
		childOffset = valOffset = valEnd = parentEnd;
		return INI_NO_CHILD;
	}
	childOffset = parentIndex+childIndex+1;	//+1 for char '\n'.
	childIndex += childExtern.size();
	int childEnd = parentRange.indexOf("\n", childIndex);
	if(childEnd == -1)
	{
		childEnd = parentRange.size();
	}
	valOffset = parentIndex+childIndex;
	valEnd = parentIndex+childEnd;
	return INI_KEY_EXIST;
}

String IniParse::value(const String &key, const String &failVal) const
{
	unsigned int childOffset, valOffset, valEnd;
	if(valueIndex(key, childOffset, valOffset, valEnd) != INI_KEY_EXIST)
	{
		return failVal;
	}
	return m_text.mid(valOffset, valEnd-valOffset);
}

bool IniParse::setValue(const String &key, const String &val)
{
	unsigned int childOffset, valOffset, valEnd;
	int status = valueIndex(key, childOffset, valOffset, valEnd);
	if(status == INI_KEY_FORMAT_ERROR)
	{
		return false;
	}
	if(status == INI_KEY_EXIST)
	{
		m_text[valOffset].replace(valEnd-valOffset, val);
	}
	else
	{
		std::vector<String> keys = key.split("/");
		if(status == INI_NO_PARENT)
		{
			m_text[childOffset].insert("["+keys[0]+"]\n"+keys[1]+"="+val+"\n");
		}
		else if(status == INI_NO_CHILD)
		{
			m_text[childOffset].insert(keys[1]+"="+val+"\n");
		}
	}
	return true;
}

bool IniParse::cleanItem(const String &key)
{
	unsigned int childOffset, valOffset, valEnd;
	bool status = valueIndex(key, childOffset, valOffset, valEnd);
	if(status == INI_KEY_FORMAT_ERROR)
	{
		return false;
	}
	if(status == INI_KEY_EXIST)
	{
		m_text[childOffset-1].replace(valEnd-childOffset+1, "", CODEC_UTF8);
	}
	return true;
}

IniParse &IniParse::operator=(const IniParse &p)
{
	m_text = p.m_text;
	return *this;
}

void IniParse::cleanParent(const String &parent)
{
	unsigned int parentOffset, parentIndex, parentEnd;
	if(parentPosRange(parent, parentOffset, parentIndex, parentEnd) == INI_PARENT_EXIST)
	{
		m_text[parentOffset].replace(parentEnd-parentOffset, "", CODEC_UTF8);
	}
}

std::vector<String> IniParse::parents() const
{
	std::vector<String> result;
	int index = m_text.indexOf("[");
	bool first = true;
	while(index != -1)
	{
		int end = m_text.indexOf("]", index);
		if(end != -1)
		{
			if(first)
			{
				result.push_back(m_text.mid(index+1, end-index-1));
				first = false;
			}
			else
			{
				result.push_back(m_text.mid(index+2, end-index-2));
			}
		}
		index = m_text.indexOf("\n[", end);
	}
	return result;
}

std::vector<String> IniParse::childs(const String &parent) const
{
	std::vector<String> result;
	unsigned int parentOffset, parentIndex, parentEnd;
	if(parentPosRange(parent, parentOffset, parentIndex, parentEnd) == INI_PARENT_EXIST)
	{
		String parentRange = m_text.mid(parentIndex+1, parentEnd-parentIndex-1);
		std::vector<String> items = parentRange.split("\n");
		int count = items.size();
		for(int i=0; i<count; ++i)
		{
			std::vector<String> keys = items[i].split("=");
			if(keys.size() == 2)
			{
				result.push_back(keys[0]);
			}
		}
	}
	return result;
}

IniSettings::IniSettings(const String &filename, StringCodec codec) : m_codec(codec)
{
	setIniFilename(filename);
}

IniSettings::IniSettings(const IniSettings &s)
{
	m_file = s.m_file;
	m_iniParse = s.m_iniParse;
	m_codec = s.m_codec;
}

IniSettings::~IniSettings()
{
	
}

String IniSettings::value(const String &key, const String &failVal) const
{
	return m_iniParse.value(key, failVal);
}

bool IniSettings::setValue(const String &key, const String &val)
{
	if(!m_iniParse.setValue(key, val))
	{
		return false;
	}
	if(!m_file.open(FILE_OPEN_MODE_Write))
	{
		return false;
	}
	bool status = m_file.write(ByteArray::fromString(m_iniParse.getText(), m_codec));
	m_file.close();
	return status;
}

bool IniSettings::setIniFilename(const String &filename)
{
	if(!m_file.setFilename(filename))
	{
		return false;
	}
	if(m_file.exist())
	{
		if(!m_file.open(FILE_OPEN_MODE_Read))
		{
			return false;
		}
		m_iniParse.setText(m_file.readAll().toString(m_codec));
		m_file.close();
	}
	else
	{
		m_iniParse.setText("");
	}
	return true;
}

String IniSettings::getIniFilename() const
{
	return m_file.getFilename();
}

bool IniSettings::cleanItem(const String &key)
{
	if(!m_iniParse.cleanItem(key))
	{
		return false;
	}
	if(!m_file.open(FILE_OPEN_MODE_Write))
	{
		return false;
	}
	bool status = m_file.write(ByteArray::fromString(m_iniParse.getText(), m_codec));
	m_file.close();
	return status;
}

IniSettings &IniSettings::operator=(const IniSettings &s)
{
	m_file = s.m_file;
	m_iniParse = s.m_iniParse;
	m_codec = s.m_codec;
	return *this;
}

bool IniSettings::cleanParent(const String &parent)
{
	m_iniParse.cleanParent(parent);
	if(!m_file.open(FILE_OPEN_MODE_Write))
	{
		return false;
	}
	bool status = m_file.write(ByteArray::fromString(m_iniParse.getText(), m_codec));
	m_file.close();
	return status;
}

std::vector<String> IniSettings::parents() const
{
	return m_iniParse.parents();
}

std::vector<String> IniSettings::childs(const String &parent) const
{
	return m_iniParse.childs(parent);
}
