#ifndef __CULVERTGENERATOR_H__
#define __CULVERTGENERATOR_H__

#include "../base/base.h"

class cCulvertGenerator : public cPluginBase
{
public:
	cCulvertGenerator();	
	bool action(cWorld*);	
};

#endif
