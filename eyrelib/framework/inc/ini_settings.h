#ifndef INI_SETTINGS_H
#define INI_SETTINGS_H

/*
 * For read and write ini format configure file easily.
 *
 * Author: Eyre Turing.
 * Last edit: 2020-12-28 16:04.
 */

#include "eyre_string.h"
#include "eyre_file.h"
#include <vector>

/*
 * Method of use:
 *
 *-------Ini format as follows---------
 * [parent1]
 * child1=value1_1
 * child2=value1_2
 * ...
 * [parent2]
 * child1=value2_1
 * child2=value2_2
 * ...
 *--------------As above---------------
 *
 * If you want to get value1_1, just use:
 * ```
 * IniParseObject.value("parent1/child1");
 * ```
 * Or:
 * ```
 * IniSettingsObject.value("parent1/child1");
 * ```
 *
 * If you want to add or change configure item:
 * ```
 * IniParseObject.setValue("parentx/childy", "content");
 * ```
 *
 * The parent:
 * Like above 'parent1', 'parent2' and 'parentx', it's call: 'parent'.
 *
 * The child:
 * Like above 'child1', 'child2' and 'childy', it's call: 'child'.
 *
 * The key:
 * The parent and child combine like 'parentx/childy', it's call: 'key'.
 *
 * The value:
 * Like above 'value1_1', 'value1_2', 'value2_1', 'value2_2',
 * it's call: 'value'.
 *
 * The item:
 * The key and value combine like 'parent1/child1=value1_1', it's
 * call: 'item'.
 *
 * Note: both parent and child can not appear '/' '[' and ']', all value
 * can not appear '\n'.
 */
class IniParse
{
public:
	IniParse(const String &text = "");
	IniParse(const IniParse &p);
	virtual ~IniParse();

	void setText(const String &text);
	String getText() const;
	
	String value(const String &key, const String &failVal="") const;
	bool setValue(const String &key, const String &val);
	
	/*
	 * This function cleanItem will remove the key in configure.
	 * If a configure is:
	 *---------------------------------
	 * [parent]
	 * child1=a
	 * child2=b
	 *---------------------------------
	 *
	 * If you ```IniParseObject.setValue("parent/child1", "");```, it
	 * only make the configure become to:
	 *---------------------------------
	 * [parent]
	 * child1=
	 * child2=b
	 *---------------------------------
	 *
	 * If you use ```IniParseObject.cleanItem("parent/child1");```, it
	 * will be:
	 *---------------------------------
	 * [parent]
	 * child2=b
	 *---------------------------------
	 */
	bool cleanItem(const String &key);
	
	/*
	 * Like cleanItem, this function cleanParent will remove a parent.
	 * If a configure is:
	 *-----------------------------------
	 * [parent]
	 * child1=abc
	 * child2=def
	 * [parent2]
	 * child1=ddd
	 *-----------------------------------
	 *
	 * If you use ```IniParseObject.cleanParent("parent");```, it will
	 * be:
	 *-----------------------------------
	 * [parent2]
	 * child1=ddd
	 *-----------------------------------
	 */
	void cleanParent(const String &parent);
	
	std::vector<String> parents() const;
	std::vector<String> childs(const String &parent) const;
	
	IniParse &operator=(const IniParse &p);

private:
	String m_text;
	
	int parentPosRange(	const String &parent, unsigned int &parentOffset,
						unsigned int &parentIndex, unsigned int &parentEnd) const;
	
	int valueIndex(	const String &key, unsigned int &childOffset,
					unsigned int &valOffset, unsigned int &valEnd) const;
};

class IniSettings
{
public:
	IniSettings(const String &filename="", StringCodec codec=CODEC_AUTO);
	IniSettings(const IniSettings &s);
	virtual ~IniSettings();
	
	bool setIniFilename(const String &filename);
	String getIniFilename() const;
	
	String value(const String &key, const String &failVal="") const;
	bool setValue(const String &key, const String &val);
	
	bool cleanItem(const String &key);
	
	bool cleanParent(const String &parent);
	
	std::vector<String> parents() const;
	std::vector<String> childs(const String &parent) const;
	
	IniSettings &operator=(const IniSettings &s);
	
private:
	File m_file;
	IniParse m_iniParse;
	StringCodec m_codec;
};

#endif	//INI_SETTINGS_H
