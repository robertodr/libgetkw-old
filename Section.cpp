/** \file Section.cpp
 *
 * \date Jun 3, 2009
 * \author Jonas Juselius <jonas.juselius@uit.no> \n
 *         CTCC, University of Troms√∏
 *
 * \brief  Container for Sections and Keywords
 */

using namespace std;
#include <iostream>

#include "Section.h"

#define PRINT_FUNC_NAME cout << "@ Section::" << __func__ << endl;

Section::Section(const string &name, const string &tag) :
	Envelope(name, false) {
	cout << "section constructor" << endl;
	PRINT_FUNC_NAME;
	type = Sect;
	if (tag.empty()) {
		this->tag = name;
	} else {
		this->tag = tag;
	}
}

Section::Section(const Section &s) :
	Envelope(s) {
	cout << "section copy " << s.name << " to " << name << "... ";
	tag = s.tag;
	nkeys = s.nkeys;
	nsect = s.nsect;
	//	sects=s.sects;
	//	keys=s.keys;
	//	tags=s.tags;
	map<string, Section *>::const_iterator sit;
	map<string, Keyword *>::const_iterator kit;
	for (sit = s.sects.begin(); sit != s.sects.end(); ++sit) {
		sects[sit->first] = new Section(*sit->second);
		tags[(*sit->second).tag] = sects[sit->first];
	}
	for (kit = s.keys.begin(); kit != s.keys.end(); ++kit) {
		keys[kit->first] = new Keyword(*kit->second);
	}
	cout << "done" << endl;
}

Section &Section::operator=(const Section &s) {
	cout << "operator= " << name << endl;
	type = s.type;
	name = s.name;
	tag = s.tag;
	nkeys = s.nkeys;
	nsect = s.nsect;
	//	sects = s.sects;
	//	keys = s.keys;
	//	tags = s.tags;
	map<string, Section *>::const_iterator sit;
	map<string, Keyword *>::const_iterator kit;
	for (sit = s.sects.begin(); sit != s.sects.end(); ++sit) {
		sects[sit->first] = new Section(*sit->second);
		tags[(*sit->second).tag] = sects[sit->first];
	}
	for (kit = s.keys.begin(); kit != s.keys.end(); ++kit) {
		keys[kit->first] = new Keyword(*kit->second);
	}
	return *this;
}

Section::~Section() {
	cout << "section del... " << name << " ";
	map<string, Section *>::iterator sit;
	map<string, Keyword *>::iterator kit;
	for (sit = sects.begin(); sit != sects.end(); ++sit) {
		delete sects[sit->first];
	}
	for (kit = keys.begin(); kit != keys.end(); ++kit) {
		delete keys[kit->first];
	}
	cout << " done" << endl;
}

Keyword &Section::getKey(const string &path) {
	PRINT_FUNC_NAME;
	Envelope &key = find(path);
	if (key.getType() != Key) {
		string err = "Error! Section::getKey(): Not a keyword, " + path;
		throw err;
	}
	Keyword &kw = static_cast<Keyword &> (key);
	return kw;
}

Section *Section::readSect(ifstream &fis) {
	return 0;
}

Section &Section::getSect(const string &path) {
	PRINT_FUNC_NAME;
	Envelope &key = find(path);
	if (key.getType() != Sect) {
		string err = "Error! Section::getSect(): Not a section, " + path;
		throw err;
	}
	Section &sect = static_cast<Section &> (key);
	return sect;

}

/* Sections can be multiply defined, provided they have different tags.
 * The first section section is added to the box, as well as to the tags
 * map if applicable. The following sections with the same name are added
 * to the tags map.
 */
void Section::add(Section &sect) {
	PRINT_FUNC_NAME;

	string name = sect.name + "<" + sect.tag + ">";
	if (has_sect(name)) {
		string err = "Error! Section::add: Section already defined, " + name;
		throw err;
	}

	sects[name] = new Section(sect);
	tags[sect.tag] = sects[name];
}

void Section::add(Keyword &key) {
	PRINT_FUNC_NAME;
	string name = key.getName();

	if (has_key(name)) {
		string err = "Error Section::add: Key already defined, " + name;
		throw err;
	}

	keys[name] = new Keyword(key);
}

Envelope &Section::find(const string &pathspec) {
	PRINT_FUNC_NAME;
	vector<string> path;
	splitPath(pathspec, path);

	Envelope &key = traversePath(path, pathspec);
	return key;
}

Envelope &Section::traversePath(vector<string> &path, const string &pathspec) {
	PRINT_FUNC_NAME;
	string cur = path[0];

	cout << "cur " + name + " " << cur << endl;
	if (path.size() == 1) {
		if (has_key(cur))
			return *keys[cur];
		if (has_sect(cur))
			return *sects[cur];
		cur = cur + "<" + cur + ">";
		if (has_sect(cur))
			return *sects[cur];
		string err = "Error! traversePath: Invalid path, " + pathspec;
		throw err;
	}

	if (!has_sect(cur))
		cur = cur + "<" + cur + ">";
	cout << "cur tag " << cur << endl;
	if (!has_sect(cur)) {
		string err = "Error! traversePath: Invalid path, " + pathspec;
		throw err;
	}

	path.erase(path.begin());
	Envelope &key = sects[cur]->traversePath(path, pathspec);
	return key;

}

void Section::splitPath(const string &pathspec, vector<string> &path) {
	PRINT_FUNC_NAME;
	string str = pathspec;
	string::size_type m = 0;
	while (true) {
		m = str.find('.', m);
		if (m == string::npos) {
			path.push_back(str);
			break;
		} else {
			path.push_back(str.substr(0, m));
		}
		m++;
		str = str.substr(m);
	}
}

int Section::splitTag(const string &path, string &tag) {
	PRINT_FUNC_NAME;
	unsigned int m, n = 0;
	m = path.find('<');
	if (m == string::npos)
		return 0;
	n = path.find('>');
	if (n == string::npos or n - m - 1 < 1)
		return 0;
	tag.clear();
	tag.append(path.substr(m + 1, n - m - 1));
	return m;
}

bool Section::has_key(const string &b) {
	if (keys.find(b) == keys.end())
		return false;
	return true;

}

bool Section::has_sect(const string &b) {
	if (sects.find(b) == sects.end())
		return false;
	return true;

}

bool Section::has_tag(const string &b) {
	if (tags.find(b) == tags.end())
		return false;
	return true;

}

//
//static Section_t *read_sect(FILE *fd)
//{
//	Section_t *sect;
//	char name[MAX_KEYLEN];
//	char tagname[2*MAX_KEYLEN+2];
//	char tag[MAX_KEYLEN];
//	int nsect, nkeys, i;
//	char set[6];
//
//	i=fscanf(fd, "%*s %s %d %s\n", name, &nsect, set);
//	i=fscanf(fd, "%*s %1c %*s %d\n", tag, &nkeys);
///*    printf("SECT: %s %c %d %d %c \n", name, tag[0], nsect, nkeys, set[0]);*/
//	if (tag[0] == 'T') {
//		i=fscanf(fd, "%s\n", tag);
//		sprintf(tagname, "%s(%s)", name, tag);
///*        printf("TAG: %s -> %s\n", tag, tagname);*/
//		sect=new_section(tagname, nkeys, nsect);
//	} else {
//		sect=new_section(name, nkeys, nsect);
//	}
//
//	sect->set=conv_bool(set[0]);
//
//	for (i=0; i < nkeys; i++) {
//		sect->kw[i]=read_key(fd);
//	}
//	for (i=0; i < nsect; i++) {
//		sect->sect[i]=read_sect(fd);
//	}
//
//	return sect;
//}
//
//static Section_t *getsect(Section_t *self, const char *name)
//{
//	int i, len;
//
//	for (i=0; i < self->nsect; i++) {
///*        printf("kuk %d %p %s\n", i, self->sect[i], self->sect[i]->name);*/
///*        printf("name=%s\n", name);*/
//		len=strlen(self->sect[i]->name);
//		if (strncmp(self->sect[i]->name, name, len) == 0 ) {
///*            printf("foundit!\n");*/
//			return self->sect[i];
//		}
//	}
//	return NULL;
//}
//
