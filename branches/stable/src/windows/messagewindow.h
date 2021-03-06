#ifndef __MESSAGEWINDOW_H__
#define __MESSAGEWINDOW_H__

#include <wm/window.h>
#include <wm/windowbutton.h>

class cMessageWindow : public cWindow
{
	class cWindowOkButton : public cWindowButton
	{
	public:
		cWindowOkButton(cWindow* parent, TiXmlDocument* skin = &cWM::skin);
		virtual ~cWindowOkButton() {}
		void onClick();
	};

public:
	cMessageWindow();	
};

#endif
