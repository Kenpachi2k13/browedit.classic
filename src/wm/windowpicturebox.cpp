#include "windowpicturebox.h"
#include "window.h"
#include "../texturecache.h"

#include <GL/gl.h>												// Header File For The OpenGL32 Library
#include <GL/glu.h>												// Header File For The GLu32 Library

void cWindowPictureBox::draw()
{
	if (loaded)
	{
		if (tex->texid() != 0)
		{
			GLfloat colors[4];
			glGetFloatv(GL_CURRENT_COLOR, colors);
			int xx, yy;
			xx = realx();
			yy = realy();
			if (highlightmode == 1)
			{
				if (inobject() && parent->istopwindow())
					glColor3f(1.0f, 1.0f, 1.0f);
				else
					glColor3f(0.2f, 0.2f, 0.2f);
			}
			else if (highlightmode == 2)
				glColor3f(1.0f, 1.0f, 1.0f);
			else if (highlightmode == 3)
			{
				if (inobject() && parent->istopwindow())
					glColor3f(0.7f, 0.7f, 0.7f);
				else
					glColor3f(1.0f, 1.0f, 1.0f);
			}
			else if (highlightmode == 4)
				glColor3f(0.3f, 0.3f, 0.3f);


			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, tex->texid());
			glBegin(GL_QUADS);
				glTexCoord2f(0,0);		glVertex2d(xx, yy);
				glTexCoord2f(1,0);		glVertex2d(xx+h, yy);
				glTexCoord2f(1,1);		glVertex2d(xx+h, yy+w);
				glTexCoord2f(0,1);		glVertex2d(xx, yy+w);
			glEnd();
			glColor4fv(colors);
		}
	}
}


void cWindowPictureBox::click()
{
//	MessageBox(NULL, "You clicked the PictureBox!", "", MB_OK);
}


void cWindowPictureBox::SetText(int i, string fil)
{
	loaded = true;
	tex = TextureCache.load(fil);
}

void cWindowPictureBox::SetInt(int i, int v)
{
	highlightmode = v;
}

int cWindowPictureBox::GetInt(int i)
{
	return highlightmode;
}