#ifndef __PLUGIN_BASE_H__
#define __PLUGIN_BASE_H__

#ifndef __MINGW32__
#pragma warning( disable : 4786 )
#endif

#include <string>
class TiXmlDocument;
class cWindow;
class cBrowInterface;

class cPluginBase
{
public:
	char				name[128];
	char				menu[512];

	cPluginBase(const char*, const char*);

	void				setInterface(cBrowInterface*);
	int					getVersion();
	virtual bool		action() = 0;
	cBrowInterface*		browInterface;
};


#endif
