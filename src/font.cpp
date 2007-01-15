#define __FONT_CPP__


#include "font.h"


#include <GL/gl.h>												// Header File For The OpenGL32 Library
#include <GL/glu.h>												// Header File For The GLu32 Library

#include <stdexcept>
#include <vector>
#include <map>
#ifndef WIN32
#include <cstdarg>
#include <unistd.h>
#endif
using namespace std;



cFont::cFont()
{

}

cFont::~cFont()
{
  glDeleteLists(list_base,256);                       // Delete All 256 Display Lists
}
/*

int cFont::print(float r, float g, float b, float x, float y, const char *fmt, ...)
{
	char type = 0;
	char    text[1024];                            // Holds Our String
	int screenStats[4],blendSrc,blendDst;
	char typ=type;
	unsigned char blending;

	va_list ap;                                    // Pointer To List Of Arguments

	if (fmt == NULL)                               // If There's No Text
		return 1;                                      // Do Nothing

	va_start(ap, fmt);                             // Parses The String For Variables
		vsprintf(text, fmt, ap);                       // And Converts Symbols To Actual Numbers
	va_end(ap);                                    // Results Are Stored In Text

	if (type>3)                                     // Did User Choose An Invalid Character Set?
		type=3;                                       // If So, Select Set 2 (Italic)


	glGetIntegerv(GL_VIEWPORT, screenStats);

	glGetIntegerv(GL_BLEND_SRC, &blendSrc);
	glGetIntegerv(GL_BLEND_DST, &blendDst);
	glGetBooleanv(GL_BLEND, &blending);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set the correct blending mode
	glBindTexture(GL_TEXTURE_2D, texture.texid());
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	if (r != 2 && g != 2 && b != 2)
		glColor3f(r,g,b);

	if(type>1) typ=typ-2;
	string s = text;
	glLoadIdentity();
	glTranslated(x,y,0);
	do
	{
		string s2 = "";
		if (s.find((char)1) == string::npos)
		{
			s2 = s;
			s = "";
		}
		else if (s.find((char)1) != 0)
		{
			s2 = s.substr(0, s.find((char)1));
			s = s.substr(s.find((char)1));
		}
		else
		{
			// parse color code
			s2 = "";
			s = s.substr(1);
			int r2 = atoi(s.substr(0,3).c_str());
			s = s.substr(3);
			int g2 = atoi(s.substr(0,3).c_str());
			s = s.substr(3);
			int b2 = atoi(s.substr(0,3).c_str());
			s = s.substr(3);
			glColor3ub(r2, g2, b2);
		}

		if (type == 0)
		{
			glListBase(list_base-32+(128*typ));
			glCallLists(strlen(text),GL_UNSIGNED_BYTE, s2.c_str()); // Write The Text To The Screen
		}
		if(type>0)
		{
			glListBase((signed int)list_base-32+(128*((signed int)type-2)));
			glCallLists(strlen(text),GL_UNSIGNED_BYTE, s2.c_str()); // Write The Text To The Screen
		}


	} while (s.size() > 0);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	if (!blending)
		glDisable(GL_BLEND);
	glBlendFunc(blendSrc, blendDst);
	return 1;
}

*/


int cFont::print(float r, float g, float b, float x, float y, const char *fmt, ...)
{
	char type = 0;
	char    text[1024];                            // Holds Our String
	int screenStats[4],blendSrc,blendDst;
	char typ=type;
	unsigned char blending;

	va_list ap;                                    // Pointer To List Of Arguments

	if (fmt == NULL)                               // If There's No Text
		return 1;                                      // Do Nothing

	va_start(ap, fmt);                             // Parses The String For Variables
		vsprintf(text, fmt, ap);                       // And Converts Symbols To Actual Numbers
	va_end(ap);                                    // Results Are Stored In Text

	if (type>3)                                     // Did User Choose An Invalid Character Set?
		type=3;                                       // If So, Select Set 2 (Italic)


	glGetIntegerv(GL_VIEWPORT, screenStats);

	glGetIntegerv(GL_BLEND_SRC, &blendSrc);
	glGetIntegerv(GL_BLEND_DST, &blendDst);
	glGetBooleanv(GL_BLEND, &blending);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set the correct blending mode
	glBindTexture(GL_TEXTURE_2D, texture.texid());
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);


	if (r != 2 && g != 2 && b != 2)
		glColor3f(r,g,b);

	if(type>1) typ=typ-2;
	glLoadIdentity();
	glTranslated(x,y,0);
	char* pointer = text;

	char colorcode[2] = "\1";

	do
	{
		string s2 = "";
		if (pointer[0] == (char)1)
		{
			char tmp;
			pointer++;
			
			tmp = pointer[3];
			pointer[3] = '\0';

			int r2 = atoi(pointer);

			pointer[3] = tmp;
			pointer+=3;
			tmp = pointer[3];
			pointer[3] = '\0';

			int g2 = atoi(pointer);

			pointer[3] = tmp;
			pointer+=3;
			tmp = pointer[3];
			pointer[3] = '\0';

			int b2 = atoi(pointer);
			pointer[3] = tmp;
			pointer+=3;

			glColor3ub(r2, g2, b2);
		}

		int tmp = (int)strstr(pointer, colorcode) - 1;

		if (tmp != -1)
		{
			tmp -= ((int)pointer-1);
			pointer[tmp] = '\0';
		}


		if (type == 0)
		{
			glListBase(list_base-32+(128*typ));
			if (tmp == -1)
				glCallLists(strlen(pointer),GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen
			else
				glCallLists(tmp,GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen

		}
		if(type>0)
		{
			glListBase((signed int)list_base-32+(128*((signed int)type-2)));
			if (tmp == -1)
				glCallLists(strlen(pointer),GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen
			else
				glCallLists(tmp,GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen
		}
		if (tmp != -1)
		{
			pointer[tmp] = '\1';
			pointer += tmp;
		}
		else
			break;
	} while (strstr(pointer, colorcode) != NULL);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


	if (!blending)
		glDisable(GL_BLEND);
	glBlendFunc(blendSrc, blendDst);
	return 1;
}



int cFont::print3d(float r, float g, float b, float a, float x, float y, float z, float scale, const char *fmt, ...)
{
	glDisable(GL_LIGHTING);
	char type = 0;
	char    text[1024];                            // Holds Our String
	int screenStats[4],blendSrc,blendDst;
	char typ=type;
	unsigned char blending;

	va_list ap;                                    // Pointer To List Of Arguments

	if (fmt == NULL)                               // If There's No Text
		return 1;                                      // Do Nothing

	va_start(ap, fmt);                             // Parses The String For Variables
		vsprintf(text, fmt, ap);                       // And Converts Symbols To Actual Numbers
	va_end(ap);                                    // Results Are Stored In Text

	if (type>3)                                     // Did User Choose An Invalid Character Set?
		type=3;                                       // If So, Select Set 2 (Italic)


	glGetIntegerv(GL_VIEWPORT, screenStats);

	glGetIntegerv(GL_BLEND_SRC, &blendSrc);
	glGetIntegerv(GL_BLEND_DST, &blendDst);
	glGetBooleanv(GL_BLEND, &blending);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set the correct blending mode
	glBindTexture(GL_TEXTURE_2D, texture.texid());
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	if (r != 2 && g != 2 && b != 2)
		glColor4f(r,g,b, a);

	if(type>1) typ=typ-2;
	glTranslatef(x,y,z);

	glPushMatrix();
	float modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	for(int xx = 0; xx < 3; xx++)
		for(int yy = 0; yy < 3; yy++)
			modelview[xx*4+yy] = ((xx == yy) ? 1.0 : 0.0);
	glLoadMatrixf(modelview);
	
	glScalef(scale, scale, scale);
	
	char* pointer = text;

	char colorcode[2] = "\1";

	glTranslatef(-textlen(text)/2.0f,0,0);

	do
	{
		string s2 = "";
		if (pointer[0] == (char)1)
		{
			char tmp;
			pointer++;
			
			tmp = pointer[3];
			pointer[3] = '\0';

			int r2 = atoi(pointer);

			pointer[3] = tmp;
			pointer+=3;
			tmp = pointer[3];
			pointer[3] = '\0';

			int g2 = atoi(pointer);

			pointer[3] = tmp;
			pointer+=3;
			tmp = pointer[3];
			pointer[3] = '\0';

			int b2 = atoi(pointer);
			pointer[3] = tmp;
			pointer+=3;

			glColor3ub(r2, g2, b2);
		}

		int tmp = (int)strstr(pointer, colorcode) - 1;

		if (tmp != -1)
		{
			tmp -= ((int)pointer-1);
			pointer[tmp] = '\0';
		}


		if (type == 0)
		{
			glListBase(list_base-32+(128*typ));
			if (tmp == -1)
				glCallLists(strlen(pointer),GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen
			else
				glCallLists(tmp,GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen

		}
		if(type>0)
		{
			glListBase((signed int)list_base-32+(128*((signed int)type-2)));
			if (tmp == -1)
				glCallLists(strlen(pointer),GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen
			else
				glCallLists(tmp,GL_UNSIGNED_BYTE, pointer); // Write The Text To The Screen
		}
		if (tmp != -1)
		{
			pointer[tmp] = '\1';
			pointer += tmp;
		}
		else
			break;
	} while (strstr(pointer, colorcode) != NULL);

	if (!blending)
		glDisable(GL_BLEND);
	glBlendFunc(blendSrc, blendDst);
	
	glPopMatrix();
	glTranslatef(-x,-y,-z);
	glEnable(GL_LIGHTING);
	return 1;
}


int cFont::load(string filename)
{
	Log(5,0,"Font: Loading font %s", filename.c_str());
	if (texture.Load(filename, false) == 0)
		return 0;
	
	if (texture.type != GL_BGRA_EXT)
		return 0;

	float cx, cy;

	list_base=glGenLists(256);                          // Creating 256 Display Lists
	glBindTexture(GL_TEXTURE_2D, texture.texid());	 // Select Our Font Texture

	for (int loop1=0; loop1<256; loop1++)              // Loop Through All 256 Lists
	{
		cx=(float)(loop1%16)/16.0f;                  // X Position Of Current Character
		cy=(float)(loop1/16)/16.0f;                  // Y Position Of Current Character
	
		int w = 2;
		for(int x = (int)(cx*(float)texture.width)+2; x < (int)((cx+(1.0f/16.0f))*(float)texture.width); x++)
		{
			bool found = false;
			for(int y = (int)(cy*(float)texture.height); y < (int)((cy+(1.0f/16.0f))*(float)texture.height); y++)
			{
				if (texture.imageData[4*(texture.width*y+x)+3] > 1)
					found = true;
			}
			if (found)
				w++;
			else
				break;
		}
		width[loop1] = w;

		// for the letter J overlapping the letter I
		#define fontwidth  0.059f
		#define fontheight 0.0625f


		glNewList(list_base+loop1,GL_COMPILE);            // Start Building A List
		glBegin(GL_QUADS);                           // Use A Quad For Each Character
			glTexCoord2f(cx,cy);          // Texture Coord (Bottom Left)
			glVertex2d(0,16);                          // Vertex Coord (Bottom Left)

			glTexCoord2f(cx,cy+fontheight);           // Texture Coord (Top Left)
			glVertex2i(0,0);                           // Vertex Coord (Top Left)

			glTexCoord2f(cx+fontwidth,cy+fontheight);   // Texture Coord (Top Right)
			glVertex2i(16,0);                          // Vertex Coord (Top Right)

			glTexCoord2f(cx+fontwidth,cy);  // Texture Coord (Bottom Right)
			glVertex2i(16,16);                         // Vertex Coord (Bottom Right)
		glEnd();                                     // Done Building Our Quad (Character)
		glTranslated(width[loop1]+1,0,0);                        // Move To The Right Of The Character
		glEndList();                                 // Done Building The Display List
	}                                              // Loop Until All 256 Are Built

	free(texture.imageData);
	Log(3,0,"Font: Loaded font %s ok", filename.c_str());

	return 1;
}

int	cFont::textlen(string s)
{
	int l = 0;
	for(int i = 0; i < (int)s.length(); i++)
	{
		if (((BYTE)s[i])-32 < 256)
		{
			l = l + width[((BYTE)s[i])-32]+1;
			if(width[((BYTE)s[i])-32] == 0)
				l+=10;
		}
		else
			l = l + 10;

	}
	return l;
}
