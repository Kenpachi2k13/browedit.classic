#ifndef __KEYBINDWINDOW_H__
#define __KEYBINDWINDOW_H__

#include <wm/window.h>
#include <wm/windowinputbox.h>
#include <wm/windowbutton.h>
#include <SDL/SDL.h>


std::string keytostring(SDLKey key);


class cKeyBindWindow :	public cWindow
{
public:
	class cWindowOkButton : public cWindowButton
	{
	public:
		cWindowOkButton(cWindow* parent, TiXmlDocument* skin = NULL);
		void click();
	};

	class cCancelButton : public cWindowButton
	{
	public:
		cCancelButton(cWindow* parent, TiXmlDocument* skin = NULL);
		void click();
	};


	class cKeyBindBox :	public cWindowInputBox
	{
		SDLKey key;
	public:
		cKeyBindBox(cWindow* parent, TiXmlDocument* skin = NULL);
		bool onKeyDown(int keyid);
		bool onKeyUp(int keyid);
		bool onChar(char ch);
		void draw(int cutoffleft, int cutoffright, int cutofftop, int cutoffbottom);
		void setInt(int id, int val);
		int getInt(int id);
	};


	class cClearButton : public cWindowButton
	{
		cKeyBindBox* clearbox;
	public:
		cClearButton(cWindow* parent, cKeyBindBox* box, TiXmlDocument* skin = NULL);
		void click();
	};

	void addclearbutton(cKeyBindBox* box, TiXmlDocument* skin);
	cKeyBindBox* addbox(std::string name, int x, int y, int defval, TiXmlDocument* skin = NULL);
	cKeyBindWindow();

};








#endif

