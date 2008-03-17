#ifndef __SOUNDWINDOW_H__
#define __SOUNDWINDOW_H__

#include "window.h"

#include "windowobject.h"
#include "windowroundbutton.h"
#include "windowbutton.h"
#include "windowlabel.h"
#include "windowinputbox.h"
#include "objectwindow.h"
#include "../undo.h"
extern cUndoStack undostack;

class cSoundWindow : public cWindow
{
public:
	cUndoItem* undo;

	cSoundWindow(cTexture* t, cFont* f) : cWindow(t,f)
	{
		cWindowObject* o;
		wtype = WT_LIGHT;
		resizable = false;
		visible = true;

		h = 340;
		w = 350;
		title = GetMsg("wm/sound/TITLE");
		center();

		defaultobject = "OkButton";

		objects["rollup"] = new cWindowRollupButton(this);
		objects["close"] = new cWindowCloseButton(this);


		addlabel("lblName", 15,20,GetMsg("wm/sound/NAME"));
		addlabel("lblFile", 15,40,GetMsg("wm/sound/FILE"));
		addlabel("lblPos", 15, 60, GetMsg("wm/sound/POSITION"));
		addlabel("lblScale", 15,80, GetMsg("wm/sound/SCALE"));
		addlabel("lblRot", 15,100,GetMsg("wm/sound/ROTATION"));



		o = new cWindowInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,20);
		o->resizeto(210,20);
		o->SetText(0,"");
		o->SetInt(0,0);
		objects["objectname"] = o;

		o = new cWindowStringInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,40);
		o->resizeto(210,20);
		o->SetText(0,"");
		objects["objectfile"] = o;
		
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,60);
		o->resizeto(70,20);
		objects["posx"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(170,60);
		o->resizeto(70,20);
		objects["posy"] = o;
		
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(240,60);
		o->resizeto(70,20);
		objects["posz"] = o;
		
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,80);
		o->resizeto(70,20);
		objects["scalex"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(170,80);
		o->resizeto(70,20);
		objects["scaley"] = o;
		
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(240,80);
		o->resizeto(70,20);
		objects["scalez"] = o;
		
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,100);
		o->resizeto(70,20);
		objects["rotx"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(170,100);
		o->resizeto(70,20);
		objects["roty"] = o;
		
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(240,100);
		o->resizeto(70,20);
		objects["rotz"] = o;
////////////////
		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,120);
		o->resizeto(210,20);
		objects["unknown1"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,140);
		o->resizeto(210,20);
		objects["unknown2"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,160);
		o->resizeto(210,20);
		objects["unknown3"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,180);
		o->resizeto(210,20);
		objects["unknown4"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,200);
		o->resizeto(210,20);
		objects["unknown5"] = o;

		o = new cWindowInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,220);
		o->resizeto(210,20);
		objects["unknown6"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,240);
		o->resizeto(210,20);
		objects["unknown7"] = o;

		o = new cWindowFloatInputBox(this);
		o->alignment = ALIGN_TOPLEFT;
		o->moveto(100,260);
		o->resizeto(210,20);
		objects["unknown8"] = o;



		objects["OkButton"] = new cWindowOkButton(this);
		objects["CancelButton"] = new cWindowCancelButton(this);
	}	

	void* userfunc(void* param)
	{
		intptr_t p = (intptr_t)param;
		if(p == 0) // cancel
		{
			undo->undo();
			delete undo;
		}
		else
		{

			for(map<string, cWindowObject*, less<string> >::iterator i = objects.begin(); i !=  objects.end(); i++)
			{
				if(i->second->type == OBJECT_FLOATINPUTBOX)
					i->second->onkeydown(SDLK_RETURN, false);
			}
/*			cWindow* w = Graphics.WM.getwindow(WT_MODELOVERVIEW);
			if(w != NULL)
				w->userfunc(NULL);*/
			undostack.push(undo);
		}

		return NULL;
	}
};

#endif