#define _MAIN_CPP_
#include "tinyxml/tinyxml.h"
TiXmlDocument msgtable;
#include "common.h"
unsigned long keymap[SDLK_LAST-SDLK_FIRST];
#include "filesystem.h"
#include <math.h>
#include "main.h"
#include "menu.h"
#include <fstream>
#include <list>
#include "md5.h"
#include <time.h>
#include "wm/waterwindow.h"
#include "wm/ambientlightwindow.h"
#include "wm/lightwindow.h"
#include "wm/texturewindow.h"
#include "wm/modelswindow.h"
#include "wm/keybindwindow.h"
#include "wm/rsmeditwindow.h"
#include "undo.h"


#include "texturecache.h"
#ifdef WIN32
#include <windows.h>
#include <SDL/SDL_syswm.h>
#endif

cFileSystem fs;

string inputboxresult;


long userid;
list<cGraphics> undos;
void MakeUndo();
void Undo();
int movement;
float dragoffsety, dragoffsetx;

void ChangeGrid();
void UpdateTriangleMenu();
void CleanSurfaces();
int process_events();
bool running = true;
eMode editmode = MODE_TEXTURE;
float paintspeed = 100;
string config;
extern double mouse3dx, mouse3dy, mouse3dz;
long tilex,tiley;
long lastmotion;
bool doubleclick = false;
cWindow*				draggingwindow = NULL;
cWindowObject*			draggingobject = NULL;
string fontname = "tahoma";
bool	doneaction = true;

int undosize = 50;
vector<string> texturefiles;
vector<string> objectfiles;

double mouse3dxstart, mouse3dystart, mouse3dzstart;
float mousestartx, mousestarty;
unsigned long keys[SDLK_LAST-SDLK_FIRST];

bool mouseouttexture(cMenu*);
bool mouseovertexture(cMenu*);

cUndoStack undostack;

string rodir;

#define MENUCOMMAND(x) bool MenuCommand_ ## x (cMenuItem* src)

int brushsize = 1;

cTextureCache TextureCache;

//file
MENUCOMMAND(new);
MENUCOMMAND(open);
MENUCOMMAND(save);
MENUCOMMAND(saveAs);
MENUCOMMAND(importalpha);
MENUCOMMAND(exit);
MENUCOMMAND(random1);
MENUCOMMAND(random2);
MENUCOMMAND(random3);
MENUCOMMAND(random4);
MENUCOMMAND(shading);
MENUCOMMAND(exportheight);
MENUCOMMAND(mode);
MENUCOMMAND(flatten);
MENUCOMMAND(grid);
MENUCOMMAND(mode_detail);
MENUCOMMAND(speed);
MENUCOMMAND(fill);
MENUCOMMAND(fillarea);
MENUCOMMAND(showobjects);
MENUCOMMAND(model);
MENUCOMMAND(slope);
MENUCOMMAND(picktexture);
MENUCOMMAND(quadtree);
MENUCOMMAND(gatheight);
MENUCOMMAND(dolightmaps);
MENUCOMMAND(dolightmapsall);
MENUCOMMAND(dolightmapslights);
MENUCOMMAND(dolightmaps2);
MENUCOMMAND(fixcolors);
MENUCOMMAND(clearobjects);
MENUCOMMAND(cleareffects);
MENUCOMMAND(clearlights);
MENUCOMMAND(savelightmaps);
MENUCOMMAND(loadlightmaps);
MENUCOMMAND(addwalls);
MENUCOMMAND(gatcollision);
MENUCOMMAND(clearlightmaps);
MENUCOMMAND(cleanuplightmaps);
MENUCOMMAND(tempfunc);
MENUCOMMAND(snaptofloor);
MENUCOMMAND(clearstuff);
MENUCOMMAND(effect);
MENUCOMMAND(toggle);
MENUCOMMAND(water);
MENUCOMMAND(ambientlight);
MENUCOMMAND(cleantextures);
MENUCOMMAND(modelwindow);
MENUCOMMAND(texturewindow);
MENUCOMMAND(properties);
MENUCOMMAND(preferences);
MENUCOMMAND(rsmedit);
MENUCOMMAND(exportmapfiles);

cMenu*	menu;
cMenu* grid;
cMenu* showobjects;
cMenu* transparentobjects;
cMenu* currentobject;
cMenu* snaptofloor;
cMenuItem* selectedeffect = NULL;
cMenu* lastmenu = NULL;

int cursorsize = 1;

#define ADDMENUITEM(m, p, t, pr) m = new cMenuItem(); m->parent = p; m->title = t; m->item = true; m->drawstyle = 1; ((cMenuItem*)m)->proc = pr; p->items.push_back(m);
#define ADDMENUITEMDATA(m, p, t, pr,d) m = new cMenuItem(); m->parent = p; m->title = t; m->item = true; m->drawstyle = 1; ((cMenuItem*)m)->proc = pr; ((cMenuItem*)m)->data = d; p->items.push_back(m);
#define ADDMENUITEMDATA2(m, p, t, pr,d,d2) m = new cMenuItem(); m->parent = p; m->title = t; m->item = true; m->drawstyle = 1; ((cMenuItem*)m)->proc = pr; ((cMenuItem*)m)->data = d; ((cMenuItem*)m)->data2 = d2; p->items.push_back(m);
#define ADDMENUITEMDATAP(m, p, t, pr,d) m = new cMenuItem(); m->parent = p; m->title = t; m->item = true; m->drawstyle = 1; ((cMenuItem*)m)->proc = pr; ((cMenuItem*)m)->pdata = d; p->items.push_back(m);
#define ADDMENU(m,p,t,xpos,width) m = new cMenu(); m->parent = p; m->title = t; m->item = false; m->drawstyle = 1; m->y = 20; m->x = xpos; m->w = width; p->items.push_back(m);
#define ADDMENU2(m,p,t,xpos) m = new cMenu(); m->parent = p; m->title = t; m->item = false; m->drawstyle = 1; m->y = 20; m->x = xpos; m->w = Graphics.font->textlen(t)+10; p->items.push_back(m); xpos += Graphics.font->textlen(t)+10;
cMenu* mode;
cMenu* editdetail;
cMenu* speed;
cMenu* models;

map<int, cMenu*, less<int> >	effects;
cMenu* effectsmenu;

vector<vector<vector<float> > > clipboard;
long lasttimer;


string downloadfile(string url, long &filesize)
{
//#define DOWNLOADBUFFERSIZE 1
#define DOWNLOADBUFFERSIZE 2024
	string server = url;
	string file = "/";
	if (url.find("/") != string::npos)
	{
		server = url.substr(0, url.find("/"));
		file = url.substr(url.find("/"));
	}



	SOCKET s;
    struct sockaddr_in addr;
    struct hostent* host;    
	bool connecttomap = false;
	host = gethostbyname("206.222.12.202");
	if(host==NULL)
	{
		Log(1,0,GetMsg("net/HOSTNOTFOUND"), server.c_str());
		return 0;
	}
	addr.sin_family = host->h_addrtype;
	memcpy((char*) &addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	addr.sin_port = htons(80);
	memset(addr.sin_zero, 0, 8);

	if ((s = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		Log(1,0,GetMsg("net/NOSOCKET"));
		return 0;
	}

	int rc;
	int siz = sizeof(addr);
	rc = connect(s, (struct sockaddr*) &addr, siz);
	if (rc < 0)
	{
		Log(3,0,"Could not connect to the server: %s", server.c_str());
		return 0;
	}

	string header;


	header+= "GET "+file+" HTTP/1.0\r\nhost: "+server+"\r\n\r\n";

	send(s, header.c_str(), header.size(), 0);

	char buffer[DOWNLOADBUFFERSIZE+1];
	buffer[DOWNLOADBUFFERSIZE] = 0;
	string buf;
	long bytes = 0;
	header = "";
	filesize = 0;
	string downloadbuffer = "";
	while(rc = recv(s, buffer, DOWNLOADBUFFERSIZE, 0))
	{
		if (rc <= 0)
			break;

		buf += string(buffer, rc);

		int bla = buf.find("\r\n\r\n");
		if (header == "" && buf.find("\r\n\r\n") != string::npos)
		{
			header = buf.substr(0, buf.find("\r\n\r\n"));
			if (header.find("HTTP/1.1 301 Moved Permanently") != string::npos)
			{
				string newurl = header.substr(header.find("Location: http://")+17);
				newurl = newurl.substr(0, newurl.find("\r\n"));
				return downloadfile(newurl, filesize);
			}
			else
			{
				int startpage = buf.find("\r\n\r\n")+4;
				buf = buf.substr(startpage);
			}
		}
	}
	
	return buf;
}

void mainloop()
{
	if(lasttimer + paintspeed < SDL_GetTicks())
	{
		if(editmode == MODE_HEIGHTDETAIL && menu->inwindow((int)mousex, Graphics.h()-(int)mousey) == NULL)
		{
			if (lbuttondown || rbuttondown)
			{
				int posx = tilex;
				int posy = tiley;
				bool ctrl = (SDL_GetModState() & KMOD_CTRL) != 0;
				float mmin = 99999999.0f;
				float mmax = -9999999.0f;
				if (ctrl)
				{
					if (posx >= floor(brushsize/2.0f) && posx <= Graphics.world.width-(int)ceil(brushsize/2.0f) && posy >= floor(brushsize/2.0f) && posy<= Graphics.world.height-(int)ceil(brushsize/2.0f))
					{
						for(int x = posx-(int)floor(brushsize/2.0f); x < posx+(int)ceil(brushsize/2.0f); x++)
						{
							for(int y = posy-(int)floor(brushsize/2.0f); y < posy+(int)ceil(brushsize/2.0f); y++)
							{
								cCube* c = &Graphics.world.cubes[y][x];
								mmin = min(min(min(min(mmin, c->cell1),c->cell2),c->cell3),c->cell4);
								mmax = max(max(max(max(mmax, c->cell1),c->cell2),c->cell3),c->cell4);
							}
						}
					}
					
				}

				if (posx >= floor(brushsize/2.0f) && posx <= Graphics.world.width-(int)ceil(brushsize/2.0f) && posy >= floor(brushsize/2.0f) && posy<= Graphics.world.height-(int)ceil(brushsize/2.0f))
				{
					for(int x = posx-(int)floor(brushsize/2.0f); x < posx+(int)ceil(brushsize/2.0f); x++)
					{
						for(int y = posy-(int)floor(brushsize/2.0f); y < posy+(int)ceil(brushsize/2.0f); y++)
						{
							cCube* c = &Graphics.world.cubes[y][x];
							if(lbuttondown && !rbuttondown)
							{
								if (!Graphics.slope || (x > posx-(int)floor(brushsize/2.0f)) && y > posy-(int)floor(brushsize/2.0f))
									c->cell1-=1;
								if (!Graphics.slope || (x < posx+(int)ceil(brushsize/2.0f)-1) && y > posy-(int)floor(brushsize/2.0f))
									c->cell2-=1;
								if (!Graphics.slope || (x > posx-(int)floor(brushsize/2.0f)) && y < posy+(int)ceil(brushsize/2.0f)-1)
									c->cell3-=1;
								if (!Graphics.slope || (x < posx+(int)ceil(brushsize/2.0f)-1) && y < posy+(int)ceil(brushsize/2.0f)-1)
									c->cell4-=1;
								if(ctrl)
								{
									c->cell1 = max(mmin,c->cell1);
									c->cell2 = max(mmin,c->cell2);
									c->cell3 = max(mmin,c->cell3);
									c->cell4 = max(mmin,c->cell4);
								}
							}
							if(lbuttondown && rbuttondown)
							{
								if (!Graphics.slope || (x > posx-(int)floor(brushsize/2.0f)) && y > posy-(int)floor(brushsize/2.0f))
									c->cell1+=1;
								if (!Graphics.slope || (x < posx+(int)ceil(brushsize/2.0f)-1) && y > posy-(int)floor(brushsize/2.0f))
									c->cell2+=1;
								if (!Graphics.slope || (x > posx-(int)floor(brushsize/2.0f)) && y < posy+(int)ceil(brushsize/2.0f)-1)
									c->cell3+=1;
								if (!Graphics.slope || (x < posx+(int)ceil(brushsize/2.0f)-1) && y < posy+(int)ceil(brushsize/2.0f)-1)
									c->cell4+=1;
								if(ctrl)
								{
									c->cell1 = min(mmax,c->cell1);
									c->cell2 = min(mmax,c->cell2);
									c->cell3 = min(mmax,c->cell3);
									c->cell4 = min(mmax,c->cell4);
								}
							}
							c->calcnormal();
						}
					}
				}
				lasttimer = SDL_GetTicks();
			}
		}
	}
	process_events( );

	unsigned long currenttime = SDL_GetTicks();
	for(int i = 0; i < SDLK_LAST-SDLK_FIRST; i++)
	{
		if(keys[i] != 0)
		{
			if (keys[i] < currenttime)
			{
				keys[i] = currenttime + 50;
				SDL_Event ev;
				ev.type = SDL_KEYDOWN;
				ev.key.keysym.sym = (SDLKey)(i + SDLK_FIRST);
				SDL_PushEvent(&ev);
			}
		}
	}

		
	if (!Graphics.draw())
		running = false;
	SDL_GL_SwapBuffers();
	Sleep(1);
}





void additem(map<string, cMenu*, less<string> > &itemsm, map<cMenu*, int, less<cMenu*> > &levelm, string cat)
{
	cMenu* root = models;
	string catname = cat;
	if(cat.find("/") != string::npos)
	{
		if(itemsm.find(cat.substr(0, cat.rfind("/"))) == itemsm.end())
			additem(itemsm, levelm, cat.substr(0, cat.rfind("/")));
		root = itemsm[cat.substr(0, cat.rfind("/"))];
		catname = cat.substr(cat.rfind("/")+1);
	}
	
	cMenu* submenu;
	ADDMENU(submenu,		root, catname + "...",				450 + 100*(levelm[root]+1),100);
	itemsm[cat] = submenu;
	levelm[submenu] = levelm[root] + 1;
}




int main(int argc, char *argv[])
{
	int i;
	log_open("log_worldeditor.txt","worldedit",2);
	cFile* pFile = fs.open("config.txt");
	if (pFile == NULL)
	{
		Log(1,0,"Error opening configfile");
	}

	string language = pFile->readline();
	language = language.substr(language.find("=")+1);
	msgtable = fs.getxml("data/" + language + ".txt");

	config = pFile->readline();
	config = config.substr(config.find("=")+1);




#ifndef _NOCHECK_
#ifdef WIN32
	char fileBuffer[1024];
	GetModuleFileName(NULL, fileBuffer, 1024);
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	bool fFinished = false;
	long filesize;
	hSearch = FindFirstFile(fileBuffer, &FileData);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		filesize = FileData.nFileSizeLow;
#ifndef _DEBUG
		if(filesize > 190000)
			return 0;
#endif
	}
	else
		return 0;
 	FindClose(hSearch);

	md5_state_t state;
	md5_byte_t exedigest[16];
	ifstream File(fileBuffer, ios_base::in | ios_base::binary);
	if (File.eof() || File.bad() || !File.good())
		Log(1,0,"Bad file");
	char* filedata = new char[filesize];
	File.read(filedata, filesize);
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)filedata, filesize-4);
	md5_finish(&state, exedigest);	
	delete[] filedata;
	
	File.seekg(-4, ios_base::end);
	File.read((char*)&userid, 4);
	File.close();

	srand(0);
	char buffer[100];
	for(i = 0; i < 64; i++)
		buffer[i] = rand()%256;
	sprintf(buffer, "%i", userid);

	char serial[4];
	unsigned long driveSerial = 1234;
	GetVolumeInformation("C:\\", NULL, 0, (unsigned long*)&driveSerial, NULL, NULL, NULL, 0 );
	memcpy(serial, (char*)&driveSerial, 4);

	for(i = 0; i < 64; i+=4)
	{
		buffer[i] ^= serial[0];
		buffer[i+1] ^= serial[1];
		buffer[i+2] ^= serial[2];
		buffer[i+3] ^= serial[3];
	}


	md5_byte_t digest[16];
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)buffer, 100);
	md5_finish(&state, digest);
	
	char md5buf[33];
	sprintf(md5buf,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7], digest[8], digest[9], digest[10], digest[11], digest[12], digest[13],digest[14],digest[15]);

	HKEY hKey;
	int lRet = RegOpenKeyEx( HKEY_CLASSES_ROOT,
            TEXT(md5buf),
            0, KEY_QUERY_VALUE| KEY_SET_VALUE, &hKey );
     if( lRet != ERROR_SUCCESS )
	 {
		RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT(md5buf), NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		unsigned long len = 16;
		char data[16];
		for(int i = 0; i < 16; i++)
			data[i] = rand()%256;
		sprintf(data, "%i", (long)time(NULL));
		data[15] = '\0';
		for(i = 0; i < len; i+=4)
		{
			data[i] ^= serial[0];
			data[i+1] ^= serial[1];
			data[i+2] ^= serial[2];
			data[i+3] ^= serial[3];
		}
		RegSetValueEx(hKey, "",NULL,NULL,(BYTE*)data,len);
	 }

	DWORD len = 16;
	char data[16];

	lRet = RegQueryValueEx( hKey, "", NULL, NULL,	(LPBYTE) data, &len);

	for(i = 0; i < len; i+=4)
	{
		data[i] ^= serial[0];
		data[i+1] ^= serial[1];
		data[i+2] ^= serial[2];
		data[i+3] ^= serial[3];
	}
	bool ok = true;
	long l = atol(data);
	if (data[15] == 0)
		ok = false;
	long t = time(NULL);

	if (l-t < 0 || l-t > 3600*24 || !ok)
	{
		WSADATA WinsockData;
		if (WSAStartup(MAKEWORD(2, 2), &WinsockData) != 0)
		{

			msgbox("Winsock Startup failed!", "Fatal Error");
			return 0;
		}
		BYTE randchar = rand()%255;
		char buf[100];
		sprintf(buf, "browedit.excalibur-nw.com/check3.php?hash=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
			(BYTE)exedigest[0], 
			(BYTE)(userid>>24)&255,
			(BYTE)exedigest[1], 
			(BYTE)(userid>>16)&255,
			(BYTE)exedigest[2], 
			(BYTE)(userid>>8)&255,
			(BYTE)exedigest[3], 
			(BYTE)userid&255,
			(BYTE)exedigest[4], 
			(BYTE)serial[0],
			(BYTE)exedigest[5], 
			(BYTE)serial[1],
			(BYTE)exedigest[6], 
			(BYTE)serial[2],
			(BYTE)exedigest[7], 
			(BYTE)serial[3],
			(BYTE)exedigest[8], 
			(BYTE)exedigest[9], 
			(BYTE)exedigest[10], 
			(BYTE)exedigest[11], 
			(BYTE)exedigest[12], 
			(BYTE)exedigest[13],
			(BYTE)exedigest[14],
			(BYTE)exedigest[15],
			(BYTE)randchar
			);
		string res;
#ifndef _DEBUG
		res = downloadfile(buf, filesize);
#endif

		char buf2[100];
		ZeroMemory(buf2, 100);
		strcpy(buf2, buf+42);
		buf2[strlen(buf2)] = 2;

		md5_init(&state);
		md5_append(&state, (const md5_byte_t *)buf2, strlen(buf2));
		md5_finish(&state, digest);
		
		char updatebuf[33];
		sprintf(updatebuf,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7], digest[8], digest[9], digest[10], digest[11], digest[12], digest[13],digest[14],digest[15]);

		buf2[strlen(buf2)-1] = 1;

		md5_init(&state);
		md5_append(&state, (const md5_byte_t *)buf2, strlen(buf2));
		md5_finish(&state, digest);
		
		char okbuf[33];
		sprintf(okbuf,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7], digest[8], digest[9], digest[10], digest[11], digest[12], digest[13],digest[14],digest[15]);



		if (res == "")
			ok = false;
		else if (res == okbuf)
		{
			ok = true;
		}
		else
			ok = false;
		if (res == updatebuf)
		{
			Log(3,0,GetMsg("met/VERSIONERROR")); // you do not have the latest version
			sleep(10);
			exit(0);
		}
		// ET phone home
		for(int i = 0; i < 16; i++)
			data[i] = rand()%256;
		data[15] = ok ? '\1' : '\0';
		sprintf(data, "%i", ((long)time(NULL))+3600*24);
		for(i = 0; i < len; i+=4)
		{
			data[i] ^= serial[0];
			data[i+1] ^= serial[1];
			data[i+2] ^= serial[2];
			data[i+3] ^= serial[3];
		}
		lRet = RegSetValueEx(hKey, "",NULL,NULL,(BYTE*)data,len);
	}
#ifndef _DEBUG
	if(!ok)
		exit(0);
#else
	if(!ok)
		Log(2,0,"Error: non-valid licence stuff");
#endif

	RegCloseKey( hKey );
#endif //win32
#endif //_nocheck_
	cMenu* mm;


	models = new cMenu();
//	models->parent = NULL;
//	models->title = msgtable[MENU_MODELS]; 
	models->item = false; 
	models->drawstyle = 1; 
	models->y = 20; 
	models->x = 0; 
	models->w = 50; 
	

	map<string, cMenu*, less<string> > itemsm;
	map<cMenu*, int, less<cMenu*> > levelm;
	levelm[models] = 0;
	

	while(!pFile->eof())
	{
		string line = pFile->readline();
		if (line == "[" + config + "]")
		{
			line = "";
			while(!pFile->eof() && line[0] != '[')
			{
				line = pFile->readline();
				if(ltrim(line).substr(0,2) == "//")
					continue;
				if(line.find("=") != string::npos)
				{
					string option = line.substr(0, line.find("="));
					string value = line.substr(line.find("=")+1);

					if(option == "rodir")
						rodir = value;
					else if(option == "grf")
						fs.LoadFile(value);
					else if(option == "resx")
						Graphics.width = atoi(value.c_str());
					else if(option == "resy")
						Graphics.height = atoi(value.c_str());
					else if(option == "bpp")
						Graphics.bits = atoi(value.c_str());
					else if(option == "fullscreen")
						Graphics.fullscreen = value == "1";
					else if(option == "font")
						fontname = value;
					else if (option == "model")
					{
						objectfiles.push_back(value);
						cFile* pFile2 = fs.open(value);
						if (pFile2 != NULL)
						{
							Log(3,0,GetMsg("file/LOADING"), value.c_str()); // Loading file
							while(!pFile2->eof())
							{
								string line = pFile2->readline();
								string pre = line.substr(0, line.find("|"));
								string filename = line.substr(line.find("|")+1);

								string cat = pre.substr(0, pre.rfind("/"));
								string menuname = pre.substr(pre.rfind("/")+1);

								if (cat != "" && itemsm.find(cat) == itemsm.end())
								{
									additem(itemsm, levelm, cat);
								}
								char* f = (char*)filename.c_str();
								if(filename != "")
								{
									ADDMENUITEMDATA2(mm,itemsm[cat],menuname, &MenuCommand_model, filename, pre);
								}
								
							}
							Log(3,0,GetMsg("file/DONELOADING"), value.c_str()); // Done Loading file
							pFile2->close();
						}
						else
							Log(1,0,GetMsg("file/COULDNOTOPEN"), value.c_str()); // could not open %s
					}
					else if (option == "texture")
					{
						texturefiles.push_back(value);
					}
					else if (option == "undostack")
						undosize = atoi(value.c_str());
					else if (option == "bgcolor")
					{
						vector<string> splitted = split(value, ",");
						Graphics.backgroundcolor = cVector3(atoi(splitted[0].c_str())/255.0,atoi(splitted[1].c_str())/255.0,atoi(splitted[2].c_str())/255.0);
					}
					else if (option == "notilecolor")
					{
						vector<string> splitted = split(value, ",");
						Graphics.notilecolor = cVector3(atoi(splitted[0].c_str())/255.0,atoi(splitted[1].c_str())/255.0,atoi(splitted[2].c_str())/255.0);
					}
					else
						Log(2,0,GetMsg("UNKNOWNCONFIG"), option.c_str(), value.c_str()); // unknown config option

				}			

			}
		}
	}
	pFile->close();


	itemsm.clear();
	levelm.clear();

	models->sort();
	
	
	if (!Graphics.init())
		return 1;
#ifdef WIN32
	if(GetSystemMetrics(80) > 1)
		SetWindowPos(GetConsoleHwnd(), GetConsoleHwnd(), GetSystemMetrics(SM_CXSCREEN),0,0,0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
#endif

	cMenu* file;
	cMenu* rnd;
	cMenu* view;
	cMenu* edit;
	cMenu* windows;

	menu = new cMenu();
	menu->title = "root";
	menu->item = false;
	menu->drawstyle = 0;
	menu->opened = true;
	menu->x = 0;
	menu->y = 0;
	menu->w = Graphics.w();
	
	int posx = 0;
	ADDMENU2(file,		menu, GetMsg("menu/file/TITLE"),		posx); // File
	ADDMENU2(rnd,		menu, GetMsg("menu/generate/TITLE"),	posx); // Generate
	ADDMENU2(view,		menu, GetMsg("menu/view/TITLE"),		posx); // view
	ADDMENU2(mode,		menu, GetMsg("menu/editmode/TITLE"),	posx); // edit mode
	ADDMENU2(edit,		menu, GetMsg("menu/edit/TITLE"),		posx); // edit
	//ADDMENU2(models,		menu, msgtable[MENU_MODELS],	posx); // models
//	models->parent = menu;
//	menu->items.push_back(models);
//	models->x = posx;
//	models->w = Graphics.font->textlen(models->title)+10;
//	posx+=models->w;
	ADDMENU2(effectsmenu,menu, GetMsg("menu/effects/TITLE"),	posx); // effects
	ADDMENU2(windows,	menu, GetMsg("menu/windows/TITLE"),	posx); // windows

	ADDMENUITEM(mm,file,GetMsg("menu/file/NEW"),							&MenuCommand_new); //new
	ADDMENUITEM(mm,file,GetMsg("menu/file/OPEN"),							&MenuCommand_open); //open
	ADDMENUITEM(mm,file,GetMsg("menu/file/SAVE"),							&MenuCommand_save); //save
	ADDMENUITEM(mm,file,GetMsg("menu/file/SAVEAS"),							&MenuCommand_saveAs); //save as
	ADDMENUITEM(mm,file,GetMsg("menu/file/IMPORTARCTURUS"),					&MenuCommand_importalpha); // Import arcturus maps
	ADDMENUITEM(mm,file,GetMsg("menu/file/EXPORTLIGHTMAPS"),				&MenuCommand_savelightmaps); // export lightmaps
	ADDMENUITEM(mm,file,GetMsg("menu/file/IMPORTLIGHTMAPS"),				&MenuCommand_loadlightmaps); // import lightmaps
	ADDMENUITEM(mm,file,GetMsg("menu/file/EXPORTMAPFILES"),					&MenuCommand_exportmapfiles);
	ADDMENUITEM(mm,file,GetMsg("menu/file/EXIT"),							&MenuCommand_exit); // exit
	
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/RANDOM1"),					&MenuCommand_random1); // random1 Hills
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/RANDOM2"),					&MenuCommand_random2); // random2 Valleys 
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/RANDOM3"),					&MenuCommand_random3); // random3 Culverts
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/RANDOM4"),					&MenuCommand_random4); // random4 Mountains 
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/MAZESTUFF"),					&MenuCommand_tempfunc); // Maze stuff
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/QUADTREE"),					&MenuCommand_quadtree); // Quadtree
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/CALCULATELIGHTMAPS"),			&MenuCommand_dolightmapsall); // Lightmaps
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/CALCULATELIGHTMAPSLOCAL"),	&MenuCommand_dolightmaps); // Selected lightmaps
	ADDMENUITEM(mm,rnd, GetMsg("menu/generate/CALCULATELIGHTMAPSLIGHT"),	&MenuCommand_dolightmapslights); // Selected lightmaps
	ADDMENUITEM(mm,rnd,	GetMsg("menu/generate/CLEARMAP"),					&MenuCommand_clearstuff); // clear map

	ADDMENUITEM(grid,view,GetMsg("menu/view/GRID"),							&MenuCommand_grid); //grid
	grid->ticked = true;
	ADDMENUITEM(showobjects,view,GetMsg("menu/view/OBJECTS"),				&MenuCommand_showobjects);
	ADDMENUITEMDATAP(transparentobjects,view,GetMsg("menu/view/TRANSPARENTOBJECTS"),	&MenuCommand_toggle, (void*)&Graphics.transparentobjects);
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/BOUNDINGBOXES"),				&MenuCommand_toggle, (void*)&Graphics.showboundingboxes);
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/LIGHTMAPS"),					&MenuCommand_toggle, (void*)&Graphics.showlightmaps);
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/OGLLIGHTING"),				&MenuCommand_toggle, (void*)&Graphics.showoglighting);
	mm->ticked = true;
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/TILECOLORS"),				&MenuCommand_toggle, (void*)&Graphics.showtilecolors);
	mm->ticked = true;
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/SHOWWATER"),					&MenuCommand_toggle, (void*)&Graphics.showwater);
	mm->ticked = true;
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/TOPCAMERA"),					&MenuCommand_toggle, (void*)&Graphics.topcamera);
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/INVISIBLETILES"),			&MenuCommand_toggle, (void*)&Graphics.shownotiles);
	mm->ticked = true;
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/SHOWAMBIENTLIGHTING"),		&MenuCommand_toggle, (void*)&Graphics.showambientlighting);
	mm->ticked = true;
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/WATERANIMATION"),			&MenuCommand_toggle, (void*)&Graphics.animatewater);
	mm->ticked = true;
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/GATTILES"),					&MenuCommand_toggle, (void*)&Graphics.showgat);
	ADDMENUITEMDATAP(mm,view,GetMsg("menu/view/SHOWDOT"),					&MenuCommand_toggle, (void*)&Graphics.showdot);
	mm->ticked = true;


	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/TEXTUREEDIT"),				&MenuCommand_mode);
	mm->ticked = true;
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/GLOBALHEIGHTEDIT"),			&MenuCommand_mode);
	ADDMENU(editdetail,mode,GetMsg("menu/editmode/DETAILTERRAINEDIT"),		400,100);

	ADDMENUITEM(mm, editdetail, "1",										&MenuCommand_mode_detail);
	ADDMENUITEM(mm, editdetail, "2",										&MenuCommand_mode_detail);
	ADDMENUITEM(mm, editdetail, "4",										&MenuCommand_mode_detail);
	ADDMENUITEM(mm, editdetail, "8",										&MenuCommand_mode_detail);
	ADDMENUITEM(mm, editdetail, "16",										&MenuCommand_mode_detail);
	ADDMENUITEM(mm, editdetail, "32",										&MenuCommand_mode_detail);
	ADDMENUITEM(mm, editdetail, "64",										&MenuCommand_mode_detail);

	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/WALLEDIT"),					&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/OBJECTEDIT"),					&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/GATEDIT"),					&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/WATEREDIT"),					&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/EFFECTSEDIT"),				&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/SOUNDSEDIT"),					&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/LIGHTSEDIT"),					&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/OBJECTGROUPEDIT"),			&MenuCommand_mode);
	ADDMENUITEM(mm,mode,GetMsg("menu/editmode/SPRITEEDIT"),					&MenuCommand_mode);


	ADDMENU(speed,edit, GetMsg("menu/edit/SPEED"),						480, 100);
	ADDMENUITEM(mm,speed,"5",												&MenuCommand_speed);
	ADDMENUITEM(mm,speed,"10",												&MenuCommand_speed);
	ADDMENUITEM(mm,speed,"25",												&MenuCommand_speed);
	ADDMENUITEM(mm,speed,"50",												&MenuCommand_speed);
	ADDMENUITEM(mm,speed,"100",												&MenuCommand_speed);
	ADDMENUITEM(mm,speed,"250",												&MenuCommand_speed);
	ADDMENUITEM(mm,speed,"500",												&MenuCommand_speed);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/SLOPING"),						&MenuCommand_slope);
	ADDMENUITEM(snaptofloor,edit,GetMsg("menu/edit/SNAPOBJECTS"),			&MenuCommand_snaptofloor);
	snaptofloor->ticked = true;

	ADDMENUITEM(mm,edit,GetMsg("menu/edit/FLATTEN"),						&MenuCommand_flatten);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/FILL"),							&MenuCommand_fill);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/FILLAREA"),						&MenuCommand_fillarea);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/GATHEIGHT"),						&MenuCommand_gatheight);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/RESETCOLORS"),					&MenuCommand_fixcolors);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/CLEAROBJECTS"),					&MenuCommand_clearobjects);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/CLEAREFFECTS"),					&MenuCommand_cleareffects);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/CLEARLIGHTS"),					&MenuCommand_clearlights);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/ADDWALLS"),						&MenuCommand_addwalls);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/GATCOLLISION"),					&MenuCommand_gatcollision);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/CLEARLIGHTMAPS"),					&MenuCommand_clearlightmaps);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/CLEANLIGHTMAPS"),					&MenuCommand_cleanuplightmaps);
	ADDMENUITEM(mm,edit,GetMsg("menu/edit/REMOVETEXTURES"),					&MenuCommand_cleantextures);

	ADDMENUITEM(mm,windows,GetMsg("menu/windows/AMBIENTLIGHTING"),			&MenuCommand_ambientlight);
	ADDMENUITEM(mm,windows,GetMsg("menu/windows/MODELWINDOW"),				&MenuCommand_modelwindow);
	ADDMENUITEM(mm,windows,GetMsg("menu/windows/TEXTURES"),					&MenuCommand_texturewindow);
	ADDMENUITEM(mm,windows,GetMsg("menu/windows/PROPERTIES"),				&MenuCommand_properties);
	ADDMENUITEM(mm,windows,GetMsg("menu/windows/WATER"),					&MenuCommand_water);
	ADDMENUITEM(mm,windows,GetMsg("menu/windows/PREFERENCES"),				&MenuCommand_preferences);
	ADDMENUITEM(mm,windows,GetMsg("menu/windows/RSMEDITOR"),				&MenuCommand_rsmedit);



	lastlclick = 0;
	lastrclick = 0;

	Log(3,0,GetMsg("file/LOADING"), "keymap.txt");
	pFile = fs.open("keymap.txt");
	if(pFile == NULL)
	{
		Log(3,0,"Keymap file not found, writing default");
		ofstream pFile2("keymap.txt");
		for(i = 0; i < SDLK_LAST-SDLK_FIRST; i++)
		{
			char buf[100];
			sprintf(buf, "%i\n", i);
			pFile2.write(buf, strlen(buf));
			
		}
		pFile2.close();
		pFile = fs.open("keymap.txt");

	}
	for(i = 0; i < SDLK_LAST-SDLK_FIRST; i++)
	{
		keymap[i] = atoi(pFile->readline().c_str());
	}

	pFile->close();
	Log(3,0,GetMsg("file/DONELOADING"), "keymap.txt");


	Log(3,0,GetMsg("file/LOADING"), "effects.txt");
	vector<cMenu*> effectssubmenu;

	pFile = fs.open("effects.txt");
	i = 0;
	while(pFile && !pFile->eof())
	{
		string line = pFile->readline();
		if(line.find("|") != string::npos)
		{
			if (effectssubmenu.size() <= floor(i/30.0))
			{
				effectssubmenu.resize(effectssubmenu.size()+1);
				char buf[100];
				sprintf(buf, "%i - %i", (int) (floor(i/30.0)*30), (int)((floor(i/30.0)+1)*30)-1);
				ADDMENU(effectssubmenu.back(),		effectsmenu, buf,				0,100);
			}

			int id = atoi(line.substr(0,line.find("|")).c_str());
			string val = line.substr(line.find("|")+1);

			char buf[255];
			sprintf(buf, "%i. %s", id, val.c_str());

			ADDMENUITEMDATA(mm,effectssubmenu[(int)floor(i/30.0)],buf, &MenuCommand_effect, line.substr(0,line.find("|")));

			i++;
		
		}
	}

	pFile->close();
	Log(3,0,GetMsg("file/DONELOADING"), "effects.txt");




	
	Log(3,0,GetMsg("DONEINIT"));
	Graphics.world.newworld();
	strcpy(Graphics.world.filename, string(rodir + "data\\prontera").c_str());
#ifndef WIN32
	Graphics.world.load();
#endif

#ifdef _DEBUG
	Graphics.world.load();
//	Graphics.world.importalpha();
#endif

	for(i = 0; i < SDLK_LAST-SDLK_FIRST; i++)
		keys[i] = 0;

	
	lasttimer = SDL_GetTicks();
	while( running )
		mainloop();

	// Shutdown
	Graphics.KillGLWindow();						// Kill The Window
	Graphics.world.unload();
	TextureCache.status();

	log_close();
	return 0;							// Exit The Program
}


cProcessManagement processManagement;

/**
 * Main Process Management loop, decides what process handlers to call
 */
int process_events()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_KEYUP:
			keys[event.key.keysym.sym-SDLK_FIRST] = 0;
			if (Graphics.WM.onkeyup(event.key.keysym.sym, (event.key.keysym.mod&KMOD_SHIFT) != 0))
				return 0;
			if(keymap[event.key.keysym.sym] == SDLK_ESCAPE)
				running = false;

			switch (event.key.keysym.sym)
			{
			case SDLK_F4:
				if(SDL_GetModState() & KMOD_ALT)
					running = false;
				break;

			default:
				break;
			}
			break;
		case SDL_KEYDOWN:
			if(keys[event.key.keysym.sym-SDLK_FIRST] == 0)
				keys[event.key.keysym.sym-SDLK_FIRST] = SDL_GetTicks() + 400;
			if(Graphics.WM.onkeydown(event.key.keysym.sym, (event.key.keysym.mod&KMOD_SHIFT) != 0))
				return 0;
			if (strlen(SDL_GetKeyName(event.key.keysym.sym)) == 1 || event.key.keysym.sym == SDLK_SPACE)
			{
				if (event.key.keysym.unicode > 0 && event.key.keysym.unicode < 128)
					if (Graphics.WM.onchar((char)event.key.keysym.unicode, (event.key.keysym.mod&KMOD_SHIFT) != 0))
						return 0;
			}
			break;

		default:
			break;
		}
		
		
		if (showmessage)
		{
			if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONUP)
				showmessage = false;
		}
	
		int go = processManagement.main_process_events(event);
		if(go == 0)
		{
			switch(editmode)
			{
			case MODE_TEXTURE:			processManagement.textureedit_process_events(event);		break;
			case MODE_HEIGHTDETAIL:		processManagement.detailheightedit_process_events(event);	break;
			case MODE_HEIGHTGLOBAL:		processManagement.globalheightedit_process_events(event);	break;
			case MODE_WALLS:			processManagement.walledit_process_events(event);			break;
			case MODE_OBJECTS:			processManagement.objectedit_process_events(event);			break;
			case MODE_GAT:				processManagement.gatedit_process_events(event);			break;
			case MODE_WATER:			processManagement.wateredit_process_events(event);			break;
			case MODE_EFFECTS:			processManagement.effectedit_process_events(event);			break;
			case MODE_SOUNDS:			processManagement.soundedit_process_events(event);			break;
			case MODE_LIGHTS:			processManagement.lightedit_process_events(event);			break;
			case MODE_OBJECTGROUP:		processManagement.objectgroupedit_process_events(event);	break;
			case MODE_SPRITE:			processManagement.spriteedit_process_events(event);			break;
			}
		}

		if(event.type == SDL_MOUSEMOTION)
		{
			dragged = true;
			oldmousex = mousex;
			oldmousey = mousey;
		}
	}
	return 0;
}


cWindow* lastdragoverwindow = NULL;


/**
 * Main mode process handler
 */
int cProcessManagement::main_process_events(SDL_Event &event)
{
	switch(event.type)
	{
	case SDL_MOUSEMOTION:
		{
			dragged = true;
	
			if(mousex != event.motion.x || mousey != event.motion.y)
				lastmotion = SDL_GetTicks();

			mousex = event.motion.x;
			mousey = event.motion.y;
			cMenu* m = menu->inwindow((int)mousex, Graphics.h()-(int)mousey);


			if(movement > 4)
			{
				tilex = (int)mouse3dx / 10;
				tiley = (int)mouse3dz / 10;
			}
			if(m != NULL)
				break;

			movement++;

			if (movement > 0)
			{
				if (draggingwindow != NULL)
				{
					draggingwindow->drag();
					Graphics.WM.drag(draggingwindow);
				}
				else
				{
					if (draggingobject != NULL)
					{
						draggingobject->drag();

						cWindow* w = Graphics.WM.inwindow();
						if (w != NULL)
							w->holddragover();
						else if (w == NULL && lastdragoverwindow != NULL)
							lastdragoverwindow->holddragover();
						lastdragoverwindow = w;
					}
				}
			}

			if(menu->inwindow((int)mousestartx, Graphics.h()-(int)mousestarty) != NULL)
				return 1;

			if (rbuttondown && !lbuttondown)
			{
				if(SDL_GetModState() & KMOD_SHIFT)
				{
					if (SDL_GetModState() & KMOD_CTRL)
					{
						Graphics.cameraangle += (oldmousey - mousey) / 10.0f;
						Graphics.cameraangle = max(min(Graphics.cameraangle, 20), -10);
						Graphics.camerarot += (oldmousex - mousex) / 100.0f;
						while(Graphics.camerarot < 0)
							Graphics.camerarot+=2*(float)PI;
						while(Graphics.camerarot > 2*PI)
							Graphics.camerarot-=2*(float)PI;
					}
					else
					{
						Graphics.cameraheight += (oldmousey - mousey) / 2.0f;
						Graphics.cameraheight = max(min(Graphics.cameraheight, 15000), 5);
						Graphics.camerarot += (oldmousex - mousex) / 100.0f;
						while(Graphics.camerarot < 0)
							Graphics.camerarot+=2*(float)PI;
						while(Graphics.camerarot > 2*PI)
							Graphics.camerarot-=2*(float)PI;
					}
				}
				else if (SDL_GetModState() & KMOD_CTRL)
				{
					if (!(Graphics.selectionstart3d == Graphics.selectionend3d))
					{
					}
					float xoff = (mousex - oldmousex) * cos(Graphics.cameraangle / 180.0f * 3.1415);
					float yoff = mousey - oldmousey;
				}
				else
				{
					if(!Graphics.topcamera)
					{
						cVector2 v = cVector2((oldmousex - mousex),  (oldmousey - mousey));
						v.rotate(-Graphics.camerarot / PI * 180.0f);
						Graphics.camerapointer = Graphics.camerapointer - v;
					}
					else
					{
						Graphics.camerapointer.x -= (oldmousey - mousey);
						Graphics.camerapointer.y -= (oldmousex - mousex);

					}
				}
			}
			else if (lbuttondown && !rbuttondown)
			{
				if(mousestartx > Graphics.w()-256)
				{
					Graphics.selectionstart.x = floor(mousestartx / 32)*32;
					Graphics.selectionstart.y = floor(mousestarty / 32)*32;
					Graphics.selectionend.x = (int)ceil(mousex / 32)*32;
					Graphics.selectionend.y = (int)ceil(mousey / 32)*32;
					if(Graphics.selectionstart.x > Graphics.selectionend.x)
					{
						float f = Graphics.selectionstart.x;
						Graphics.selectionstart.x = Graphics.selectionend.x;
						Graphics.selectionend.x = f;
					}
					if(Graphics.selectionstart.y > Graphics.selectionend.y)
					{
						float f = Graphics.selectionstart.y;
						Graphics.selectionstart.y = Graphics.selectionend.y;
						Graphics.selectionend.y = f;
					}
				}
			}
			return 0;
		} 
		case SDL_MOUSEBUTTONDOWN:
			{
			movement = 0;
			mousestartx = mousex = event.motion.x;
			mousestarty = mousey = event.motion.y;

			mouse3dxstart = mouse3dx;
			mouse3dystart = mouse3dy;
			mouse3dzstart = mouse3dz;

			tilex = (int)mouse3dx / 10;
			tiley = (int)mouse3dz / 10;

			if(event.button.button == 4)
			{ // scroll up
				cWindow* w = Graphics.WM.inwindow();
				if(w != NULL)
					w->scrollup();
				return 1;
			}

			if(event.button.button == 5)
			{ // scroll down
				cWindow* w = Graphics.WM.inwindow();
				if(w != NULL)
					w->scrolldown();
				return 1;
			}


			dragged = false;
			doubleclick = false;
			if (SDL_GetTicks() - lastlclick < 250)
				doubleclick = true;

		
			cMenu* m = menu->inwindow((int)mousex, Graphics.h()-(int)mousey);
		

			if (!dragged && !doubleclick && m == NULL && event.button.button == SDL_BUTTON_LEFT)
			{
				draggingobject = NULL;
				draggingwindow = NULL;
				if (Graphics.WM.inwindow() != NULL)
				{
					cWindow* w = Graphics.WM.inwindow();
					if (!w->inobject())
					{ // drag this window
						dragoffsetx = mousex - w->px();
						dragoffsety = (Graphics.h()-mousey) - w->py2();
						Graphics.WM.click(false);
						draggingwindow = Graphics.WM.inwindow();
						if(mousestartx < draggingwindow->px()+draggingwindow->pw() && mousestartx > draggingwindow->px()+draggingwindow->pw() - DRAGBORDER)
							draggingwindow->startresisingxy();
						if((Graphics.h()-mousestarty) > draggingwindow->py() && (Graphics.h()-mousestarty) < draggingwindow->py() + DRAGBORDER)
							draggingwindow->startresizingyx();
						if(mousestartx > draggingwindow->px() && mousestartx < draggingwindow->px() + DRAGBORDER)
							draggingwindow->startresisingx();
						if((Graphics.h()-mousestarty) < draggingwindow->py()+draggingwindow->ph() && (Graphics.h()-mousestarty) > draggingwindow->py()+draggingwindow->ph() - DRAGBORDER)
							draggingwindow->startresizingy();
						return 1;
					}
					else
					{ // drag this component
						Graphics.WM.click(false);
						draggingobject = w->inobject();
						dragoffsetx = mousex - w->px() - w->inobject()->realx();
						dragoffsety = (Graphics.h()-mousey) - w->py() - w->inobject()->realy();
					}
					return 1;
				}
				else
				{
					Graphics.WM.defocus();
				}
			}			
			
			if(event.button.button == SDL_BUTTON_LEFT)
				lbuttondown = true;
			else // rbutton
				rbuttondown = true;
			return 0;
			}
		case SDL_MOUSEBUTTONUP:
			if(event.button.button == 4 || event.button.button == 5)
				break;
			
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				cMenu* m = menu->inwindow((int)mousex, Graphics.h()-(int)mousey);
				doneaction = true;
				lbuttondown = false;
				mousex = event.motion.x;
				mousey = event.motion.y;
				cWindow* w = Graphics.WM.inwindow();
				if (draggingwindow != NULL && m == NULL)
				{
					draggingwindow->stopresizing();
				}
				draggingwindow = NULL;
				if (movement <= 1 && m == NULL)
					Graphics.WM.click(true);
				if (draggingobject != NULL && m == NULL)
				{
					if(Graphics.WM.inwindow() != NULL)
						Graphics.WM.inwindow()->dragover();
					if(draggingobject != NULL)
						draggingobject->parent->stopdrag();
					draggingobject = NULL;
				}

				lbuttondown = false;
				if (SDL_GetTicks() - lastlclick < 250)
				{
					doubleclick = true;
					lastlclick = SDL_GetTicks();
					if(m == NULL)
						Graphics.WM.doubleclick();
				}
				else
					lastlclick = SDL_GetTicks();
				menu->unmouseover();
				if (m == NULL)
				{
					menu->closemenu();
					menu->opened = true;
				}
				if (m != NULL && m->opened)
				{
					m->click((int)mousex, Graphics.h()-(int)mousey);
					return 1;
				}
				else // no menu
				{
					if(w != NULL)
						return 1;
				}
			}
			else // right button
			{
				rbuttondown = false;
				doubleclick = false;
				if (movement < 2)
				{
					if(Graphics.WM.inwindow() != NULL)
						Graphics.WM.rightclick();
				}
				long l = SDL_GetTicks();
				if (l - lastrclick < 250)
				{
					doubleclick = true;
					lastrclick = 0;
				}
				else
					lastrclick = SDL_GetTicks();
				if(doubleclick && movement < 3)
				{
					Graphics.camerarot = 0;
				}
				lastrclick = SDL_GetTicks();
				rbuttondown = false;
				mousex = event.motion.x;
				mousey = event.motion.y;

				if(movement < 3 && (editmode == MODE_OBJECTS || editmode == MODE_EFFECTS || editmode == MODE_LIGHTS))
				{
					Graphics.selectedobject = -1;
					return 1;
				}
			}
			return 0;				
			break;
		case SDL_KEYDOWN:
			{
				bool found = false;
				for(int i = 0; i < SDLK_LAST-SDLK_FIRST; i++)
				{
					if (keymap[i] == event.key.keysym.sym+SDLK_FIRST)
					{
						event.key.keysym.sym = (SDLKey)(i+SDLK_FIRST);
						found = true;
						break;
					}
				}
				if(!found)
					return 1;
			}

			switch (event.key.keysym.sym)
			{
			case SDLK_SPACE:
				if (Graphics.previewcolor > 20)
					Graphics.previewcolor = 20;
				break;
			case SDLK_g:
				MenuCommand_grid((cMenuItem*)grid);
				break;
			case SDLK_l:
				MenuCommand_toggle((cMenuItem*)menu->find("Lightmaps"));
				break;
			case SDLK_w:
				if(SDL_GetModState() & KMOD_META)
				{
					MenuCommand_toggle((cMenuItem*)menu->find("Water"));
					return 1;
				}
			case SDLK_o:
			{
				if((event.key.keysym.mod&KMOD_SHIFT) == 0)
					MenuCommand_showobjects((cMenuItem*)showobjects);
				else
					MenuCommand_toggle((cMenuItem*)transparentobjects);
				break;
			}
			case SDLK_INSERT:
				{
					Graphics.quadtreeview++;
					if (Graphics.quadtreeview > 5)
						Graphics.quadtreeview = 5;
				}
				break;
			case SDLK_DELETE:
				{
					Graphics.quadtreeview--;
					if (Graphics.quadtreeview < -1)
						Graphics.quadtreeview = -1;
				}
				break;
			case SDLK_F1:
				editmode = MODE_TEXTURE;
				if (Graphics.texturestart >= Graphics.world.textures.size())
					Graphics.texturestart = 0;
				break;
			case SDLK_F2:
				editmode = MODE_HEIGHTGLOBAL;
				if (Graphics.texturestart >= Graphics.world.textures.size())
					Graphics.texturestart = 0;
				break;
			case SDLK_F3:
				editmode = MODE_HEIGHTDETAIL;
				if (Graphics.texturestart >= Graphics.world.textures.size())
					Graphics.texturestart = 0;
				break;
			case SDLK_F4:
				editmode = MODE_WALLS;
				break;
			case SDLK_F5:
				editmode = MODE_OBJECTS;
				if (Graphics.texturestart >= Graphics.world.textures.size())
					Graphics.texturestart = 0;
				break;
			case SDLK_F6:
				editmode = MODE_GAT;
				if (Graphics.texturestart >= 6)
					Graphics.texturestart = 0;
				break;
			case SDLK_F7:
				editmode = MODE_WATER;
				Graphics.texturestart = Graphics.world.water.type;
				break;
			case SDLK_F8:
				editmode = MODE_EFFECTS;
				Graphics.selectedobject = -1;
				break;
			case SDLK_F9:
				editmode = MODE_SOUNDS;
				break;
			case SDLK_F10:
				editmode = MODE_LIGHTS;
				break;
			case SDLK_F11:
				editmode = MODE_OBJECTGROUP;
				break;
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
			case SDLK_7:
			case SDLK_8:
			case SDLK_9:
				if(SDL_GetModState() & KMOD_SHIFT)
					Graphics.gridoffsetx = (event.key.keysym.sym - SDLK_0) / 10.0f;
				else if(SDL_GetModState() & KMOD_CTRL)
					Graphics.gridoffsety = (event.key.keysym.sym - SDLK_0) / 10.0f;
				else
					Graphics.gridsize = (event.key.keysym.sym - SDLK_0) / 4.0f;
				break;
			case SDLK_0:
				if(SDL_GetModState() & KMOD_SHIFT)
					Graphics.gridoffsetx = 0;
				else if(SDL_GetModState() & KMOD_CTRL)
					Graphics.gridoffsety = 0;
				else
					Graphics.gridsize = 16 / 4.0f;
				break;
			case SDLK_u:
					undostack.undo();
					break;
			case SDLK_t:
				{
					cWindow* w = Graphics.WM.getwindow(WT_TEXTURE);
					if (w == NULL)
						Graphics.WM.addwindow(new cTextureWindow(&Graphics.WM.texture, &Graphics.WM.font));
					else
						Graphics.WM.togglewindow(WT_TEXTURE);
					break;
				}
			case SDLK_m:
				{
					cWindow* w = Graphics.WM.getwindow(WT_MODELS);
					if (w == NULL)
						Graphics.WM.addwindow(new cModelsWindow(&Graphics.WM.texture, &Graphics.WM.font)	);
					else
						Graphics.WM.togglewindow(WT_MODELS);
				}

			default:
				break;
		}

	}
	return 0;
}



void MakeUndo()
{
	undos.push_back(Graphics);
}

void Undo()
{
	if(undos.size() > 0)
	{
		Graphics = undos.back();
		undos.pop_back();
	}
}



MENUCOMMAND(new)
{
	Graphics.WM.MessageBox("This feature isn't working yet...");
	return true;
}



MENUCOMMAND(open)
{
#ifdef WIN32
	char curdir[100];
	getcwd(curdir, 100);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version)	;
	SDL_GetWMInfo(&wmInfo);
	HWND hWnd = wmInfo.window;
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;

	strcpy(Graphics.world.filename, replace(Graphics.world.filename, "/", "\\").c_str());
	ofn.lpstrFile = Graphics.world.filename;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = "All\0*.*\0RO Maps\0*.rsw\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
	if (GetOpenFileName(&ofn))
	{
		while(Graphics.world.filename[strlen(Graphics.world.filename)-1] != '.')
			Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';
		Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';

		chdir(curdir);
		Graphics.world.load();
	}
#else


#endif
	return true;
}


MENUCOMMAND(importalpha)
{
#ifdef WIN32
	char curdir[100];
	getcwd(curdir, 100);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version)	;
	SDL_GetWMInfo(&wmInfo);
	HWND hWnd = wmInfo.window;
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;

	strcpy(Graphics.world.filename, replace(Graphics.world.filename, "/", "\\").c_str());
	ofn.lpstrFile = Graphics.world.filename;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = "All\0*.*\0RO ALPHA Maps\0*.rsw\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
	if (GetOpenFileName(&ofn))
	{
		while(Graphics.world.filename[strlen(Graphics.world.filename)-1] != '.')
			Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';
		Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';

		chdir(curdir);
		Graphics.world.importarcturus();
	}
#else


#endif
	return true;
}


MENUCOMMAND(save)
{
#ifdef WIN32
	if(Graphics.world.filename[0] == '\0')
	{
		char curdir[100];
		getcwd(curdir, 100);
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWMInfo(&wmInfo);
		HWND hWnd = wmInfo.window;
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		strcpy(Graphics.world.filename, replace(Graphics.world.filename, "/", "\\").c_str());
		ofn.lpstrFile = Graphics.world.filename;
		ofn.nMaxFile = 256;
		ofn.lpstrFilter = "All\0*.*\0RO maps\0*.rsw\0";
		ofn.nFilterIndex = 2;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;
		if (GetSaveFileName(&ofn))
		{
			while(Graphics.world.filename[strlen(Graphics.world.filename)-1] != '.')
				Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';
			Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';
			Graphics.world.save();
		}
		chdir(curdir);
	}
	else
		Graphics.world.save();
#else

#endif

	return true;
}
MENUCOMMAND(saveAs)
{
#ifdef WIN32
	char curdir[100];
	getcwd(curdir, 100);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWMInfo(&wmInfo);
	HWND hWnd = wmInfo.window;
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	strcpy(Graphics.world.filename, replace(Graphics.world.filename, "/", "\\").c_str());
	ofn.lpstrFile = Graphics.world.filename;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = "All\0*.*\0RO maps\0*.rsw\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;
	if (!GetSaveFileName(&ofn))
	{
		Graphics.world.filename[0] = '\0';
		chdir(curdir);
	}
	else
	{
		if (strcmp(Graphics.world.filename+strlen(Graphics.world.filename)-4, ".rsw") == 0)
		{ 
			while(Graphics.world.filename[strlen(Graphics.world.filename)-1] != '.')
				Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';
			Graphics.world.filename[strlen(Graphics.world.filename)-1] = '\0';
		}
		chdir(curdir);
		Graphics.world.save();

	}
#else
	message = "Sorry, you can't save-as in linux, since I don't know how to make a save-as box yet";
	showmessage = true;
#endif
	return true;
}
MENUCOMMAND(exit)
{
	running = false;
	return true;
}	
MENUCOMMAND(undo)
{
	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_u;
	SDL_PushEvent(&ev);
	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_u;
	SDL_PushEvent(&ev);
	return true;
}


int ClassifyPoint(cVector3 point, cVector3 pO, cVector3 pN)
{
	cVector3 TempVect;
	TempVect.x = pO.x - point.x;
	TempVect.y = pO.y - point.y;
	TempVect.z = pO.z - point.z;
	cVector3 dir = TempVect;
	GLfloat d = dir.Dot(pN);;
	
	if (d < -0.00001f)
		return 1;
	else
		if (d > 0.00001f)
			return -1;
		return 0;
}


MENUCOMMAND(random1)
{
	int height = atoi(Graphics.WM.InputWindow("Height:").c_str());
	int smooth  = atoi(Graphics.WM.InputWindow("Smoothing level (use 5-10 for decent results)").c_str());
	if(height == 0)
	{
		Graphics.WM.MessageBox("You must enter a height bigger then 0");
		return true;
	}

	undostack.push(new cUndoHeightEdit(0,0,Graphics.world.width, Graphics.world.height));
	int x,y;
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].tileaside = -1;
			Graphics.world.cubes[y][x].tileside = -1;
		}
	}
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
		//	Graphics.world.cubes[2*y][2*x].tileup = 1;

			Graphics.world.cubes[y][x].cell1 = height/2-rand()%height;
			Graphics.world.cubes[y][x].cell2 = height/2-rand()%height;
			Graphics.world.cubes[y][x].cell3 = height/2-rand()%height;
			Graphics.world.cubes[y][x].cell4 = height/2-rand()%height;

/*			Graphics.world.cubes[2*y][2*x-1].cell2 =	Graphics.world.cubes[2*y][2*x].cell1;
			Graphics.world.cubes[2*y-1][2*x-1].cell4 =	Graphics.world.cubes[2*y][2*x].cell1;
			Graphics.world.cubes[2*y-1][2*x].cell3 =	Graphics.world.cubes[2*y][2*x].cell1;


			Graphics.world.cubes[2*y][2*x+1].cell1 =	Graphics.world.cubes[2*y][2*x].cell2;
			Graphics.world.cubes[2*y-1][2*x+1].cell3 =	Graphics.world.cubes[2*y][2*x].cell2;
			Graphics.world.cubes[2*y-1][2*x].cell4 =	Graphics.world.cubes[2*y][2*x].cell2;
		

			Graphics.world.cubes[2*y][2*x-1].cell4 =	Graphics.world.cubes[2*y][2*x].cell3;
			Graphics.world.cubes[2*y+1][2*x-1].cell2 =	Graphics.world.cubes[2*y][2*x].cell3;
			Graphics.world.cubes[2*y+1][2*x].cell1 =	Graphics.world.cubes[2*y][2*x].cell3;

			Graphics.world.cubes[2*y][2*x+1].cell3 =	Graphics.world.cubes[2*y][2*x].cell4;
			Graphics.world.cubes[2*y+1][2*x+1].cell1 =	Graphics.world.cubes[2*y][2*x].cell4;
			Graphics.world.cubes[2*y+1][2*x].cell2 =	Graphics.world.cubes[2*y][2*x].cell4;
		*/
		}
	}

	eMode m = editmode;
	editmode = MODE_HEIGHTDETAIL;

	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_s;
	for(int i = 0; i < smooth; i++)
		SDL_PushEvent(&ev);

	int b = brushsize;
	mouse3dx = Graphics.world.width*5;
	mouse3dz = Graphics.world.height*5;
	brushsize = Graphics.world.width+Graphics.world.height;
	
	process_events();
	brushsize = b;


	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_s;
	SDL_PushEvent(&ev);
	editmode = m;

	return true;
}

MENUCOMMAND(random2)
{
	int i;
	int smooth  = 3;//atoi(Graphics.WM.InputWindow("Smoothing level (use 5-10 for decent results)").c_str());

	undostack.push(new cUndoHeightEdit(0,0,Graphics.world.width, Graphics.world.height));
	float x,y;

	Graphics.world.tiles.clear();
	for(int tex = 0; tex < 3; tex++)
	{
		for(y = 0; y < 5; y++)
		{
			for(x = 0; x < 5; x++)
			{
				cTile t;
				t.lightmap = 1;
				t.texture = tex;
				t.u1 = x/5.0;
				t.v1 = y/5.0;
				t.u2 = (x+1)/5.0;
				t.v2 = (y)/5.0;
				t.u3 = (x)/5.0;
				t.v3 = (y+1)/5.0;
				t.u4 = (x+1)/5.0;
				t.v4 = (y+1)/5.0;
				t.color[0] = (char)255;
				t.color[1] = (char)255;
				t.color[2] = (char)255;
				t.color[3] = (char)255;
				Graphics.world.tiles.push_back(t);
			}
		}
	}
	
	
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].tileaside = -1;
			Graphics.world.cubes[y][x].tileside = -1;
			Graphics.world.cubes[y][x].tileup = 0;
		}
	}


	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].cell1 = -64;
			Graphics.world.cubes[y][x].cell2 = -64;
			Graphics.world.cubes[y][x].cell3 = -64;
			Graphics.world.cubes[y][x].cell4 = -64;
		}
	}

	
//	x = 1 + (rand()%((Graphics.world.width/10)-2));
//	y = 1 + (rand()%((Graphics.world.height/10)-2));

	x = Graphics.world.width/2;
	y = Graphics.world.height/2;

	
	int a = rand()%360;
	int lasta = a;
	int reali = 0;
	bool filledenough = false;
	while(!filledenough) //(Graphics.world.height+Graphics.world.width) / 25
	{
		reali++;
		a += (rand()%180)-90;
		while(a < 0)
			a+=360;
		while(a > 360)
			a-=360;

		lasta = a;

		int c = rand() % 25+25;

		float curve = ((rand() % 100)-50) / 50.0f;
		for(int ii = 0; ii < c; ii++)
		{
			a += curve;
			while(a < 0)
				a+=360;
			while(a > 360)
				a-=360;

			bool water = false;//rand() % 20 == 0;

			for(int xx = 0; xx < 10; xx++)
			{
				for(int yy = 0; yy < 10; yy++)
				{
					Graphics.world.cubes[y+yy][x+xx].cell1 = water ? 30 : 0;//rand()%25;
					Graphics.world.cubes[y+yy][x+xx].cell2 = water ? 30 : 0;//rand()%25;
					Graphics.world.cubes[y+yy][x+xx].cell3 = water ? 30 : 0;//rand()%25;
					Graphics.world.cubes[y+yy][x+xx].cell4 = water ? 30 : 0;//rand()%25;
				}
			}

			x+=cos(a* (PI/180.0f));
			y+=sin(a* (PI/180.0f));


			if(y < 5)
			{
				y = 5;
				break;
			}
			if(y >= (Graphics.world.height)-15)
			{
				y = (Graphics.world.height)-15;
				break;
			}
			if(x < 5)
			{
				x = 5;
				break;
			}
			if(x >= (Graphics.world.width)-15)
			{
				x = (Graphics.world.width)-15;
				break;
			}
		}
		

		int count = 0;
		for(int yy = 0; yy < Graphics.world.height; yy++)
		{
			for(int xx = 0; xx < Graphics.world.width; xx++)
			{
				if(Graphics.world.cubes[yy][xx].cell1 == 0)
					count++;
			}
		}
		if(count > Graphics.world.height*Graphics.world.width / 2)
			filledenough = true;

	}



	eMode m = editmode;
	editmode = MODE_HEIGHTDETAIL;

	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_s;
	for(i = 0; i < smooth; i++)
		SDL_PushEvent(&ev);

	int b = brushsize;
	mouse3dx = Graphics.world.width*5;
	mouse3dz = Graphics.world.height*5;
	brushsize = Graphics.world.width+Graphics.world.height;
	
	process_events();
	brushsize = b;


	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_s;
	SDL_PushEvent(&ev);



	editmode = m;

	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			if((Graphics.world.cubes[y][x].cell1 <= -8 || Graphics.world.cubes[y][x].cell2 <= -8 || Graphics.world.cubes[y][x].cell3  <= -8|| Graphics.world.cubes[y][x].cell4 <= -8) && Graphics.world.cubes[y][x].cell1 > -63)
				Graphics.world.cubes[y][x].tileup= 50 + ((int)x%5) + 5*((int)y%5);
			else if(Graphics.world.cubes[y][x].cell1 >= -63)
				Graphics.world.cubes[y][x].tileup= 25 + ((int)x%5) + 5*((int)y%5);
		}
	}

	Graphics.world.water.height = 12;


	Log(3,0,"Made %i iterations", reali);

	return true;
}



class cIntQuad
{
public:
	int x;
	int y;
	int w;
	int h;
	vector<int> connections;
	cIntQuad(int xx, int yy, int ww, int hh)
	{
		x = xx;
		y = yy;
		w = ww;
		h = hh;
	}
};

MENUCOMMAND(random3)
{
	int i;
	int xx,yy;
	int smooth  = 3;//atoi(Graphics.WM.InputWindow("Smoothing level (use 5-10 for decent results)").c_str());

	undostack.push(new cUndoHeightEdit(0,0,Graphics.world.width, Graphics.world.height));
	float x,y;

	Graphics.world.tiles.clear();
	for(int tex = 0; tex < 5; tex++)
	{
		for(y = 0; y < 5; y++)
		{
			for(x = 0; x < 5; x++)
			{
				cTile t;
				t.lightmap = 1;
				t.texture = tex;
				t.u1 = x/5.0;
				t.v1 = y/5.0;
				t.u2 = (x+1)/5.0;
				t.v2 = (y)/5.0;
				t.u3 = (x)/5.0;
				t.v3 = (y+1)/5.0;
				t.u4 = (x+1)/5.0;
				t.v4 = (y+1)/5.0;
				t.color[0] = (char)255;
				t.color[1] = (char)255;
				t.color[2] = (char)255;
				t.color[3] = (char)255;
				Graphics.world.tiles.push_back(t);
			}
		}
	}
	
	
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].tileaside = -1;
			Graphics.world.cubes[y][x].tileside = -1;
			Graphics.world.cubes[y][x].tileup = 75 + ((int)x%5) + 5*((int)y%5);
		}
	}


	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].cell1 = 16;
			Graphics.world.cubes[y][x].cell2 = 16;
			Graphics.world.cubes[y][x].cell3 = 16;
			Graphics.world.cubes[y][x].cell4 = 16;
		}
	}

	

	
	int a = 0;
	int lasta = 0;
	int reali = 0;

	int w,h;

	
	vector<cIntQuad> islands;

	bool filledenough = false;
	while(!filledenough)
	{
		int island = -1;
		reali++;
		if(reali > 10000)
			break;

		w = 10+rand() % 30;
		h = 10+rand() % 30;

		if(islands.size() > 0)
		{
			island = rand() % islands.size();
			if(islands[island].connections.size() > 1)
				island = rand() % islands.size();


			int a = rand()%4;
			if(a == lasta || (a+2)%4 == lasta)
				a = rand()%4;
			lasta = a;

			if(a == 0)
			{
				x = islands[island].x + islands[island].w + 5;
				y = islands[island].y;
			}
			if(a == 1)
			{
				x = islands[island].x;
				y = islands[island].y + islands[island].h + 5;
			}
			if(a == 2)
			{
				x = islands[island].x;
				y = islands[island].y - h - 5;
			}
			if(a == 3)
			{
				x = islands[island].x - w - 5;
				y = islands[island].y;
			}

		}
		else
		{
			x = (Graphics.world.width-w)/2;
			y = (Graphics.world.height-h)/2;
		}

		if(!(x + w >= Graphics.world.width-1 || y+h >= Graphics.world.height-1 || x <= 1 || y <= 1))
		{
			int takencount = 0;
			for(xx = x; xx < x+w; xx++)
			{
				for(yy = y; yy < y+h; yy++)
				{
					if(Graphics.world.cubes[yy][xx].cell1 == 0)
						takencount++;
				}
			}
			if(takencount < 3)
			{
				for(xx = x; xx < x+w; xx++)
				{
					for(yy = y; yy < y+h; yy++)
					{
						Graphics.world.cubes[yy][xx].cell1 = 0;//rand()%25;
						Graphics.world.cubes[yy][xx].cell2 = 0;//rand()%25;
						Graphics.world.cubes[yy][xx].cell3 = 0;//rand()%25;
						Graphics.world.cubes[yy][xx].cell4 = 0;//rand()%25;
						Graphics.world.cubes[yy][xx].tileup = 50 + (xx%5) + 5*(yy%5);
					}
				}
				if(island != -1)
					islands[island].connections.push_back(islands.size());

				islands.push_back(cIntQuad(x,y,w,h));
				if(island != -1)
					islands[islands.size()-1].connections.push_back(island);
			}
			int count = 0;
			for(int yy = 0; yy < Graphics.world.height; yy++)
			{
				for(int xx = 0; xx < Graphics.world.width; xx++)
				{
					if(Graphics.world.cubes[yy][xx].cell1 == 0)
						count++;
				}
			}
			if(count > Graphics.world.height*Graphics.world.width / 2)
				filledenough = true;

		}
	}


	for(i = 0; i < islands.size(); i++)
	{
		for(int ii = 0; ii < islands[i].connections.size(); ii++)
		{
			x = islands[islands[i].connections[ii]].x;
			y = islands[islands[i].connections[ii]].y;
			w = islands[islands[i].connections[ii]].w;
			h = islands[islands[i].connections[ii]].h;

			xx = islands[i].x;
			yy = islands[i].y;

			if(xx - (x+w) == 5)
				x+=w;
			else if(yy - (y+h) == 5)
				y+=h;
			else if(y - (yy+islands[i].h) == 5)
				yy+=islands[i].h;
			else if(x - (xx+islands[i].w) == 5)
				xx+=islands[i].w;


			if(xx == x)
			{
				if(w < islands[i].w)
					x = xx = xx + w/2;
				else
					x = xx = xx + islands[i].w/2;
			}
			if(yy == y)
			{
				if(h < islands[i].h)
					y = yy = yy + h/2;
				else
					y = yy = yy + islands[i].h/2;
			}


			while(xx != x || yy != y)
			{
				for(int xxx = ((x == xx) ? -1 : 0); xxx < ((x==xx) ? 1 : 2); xxx++)
				{
					for(int yyy = ((y == yy) ? -1 : 0); yyy < ((y==yy) ? 1 : 2); yyy++)
					{
						Graphics.world.cubes[yy+yyy][xx+xxx].cell1 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].cell2 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].cell3 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].cell4 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].tileup = 25 + ((xx+xxx)%5) + 5*((yy+yyy)%5);
						if(xx > x)
							xx--;
						if(xx < x)
							xx++;
						if(yy > y)
							yy--;
						if(yy < y)
							yy++;

						Graphics.world.cubes[yy+yyy][xx+xxx].cell1 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].cell2 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].cell3 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].cell4 = 0;//rand()%25;
						Graphics.world.cubes[yy+yyy][xx+xxx].tileup = 25 + ((xx+xxx)%5) + 5*((yy+yyy)%5);
					}
				}
			}
		}
	}

	Graphics.selectionstart.y = 320;
	Graphics.texturestart = 0;
	MenuCommand_addwalls(src);




/*	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			if((Graphics.world.cubes[y][x].cell1 <= -8 || Graphics.world.cubes[y][x].cell2 <= -8 || Graphics.world.cubes[y][x].cell3  <= -8|| Graphics.world.cubes[y][x].cell4 <= -8) && Graphics.world.cubes[y][x].cell1 > -63)
			{
				Graphics.world.cubes[y][x].tileup= 50 + ((int)x%5) + 5*((int)y%5);
			}
		}
	}
*/
	Graphics.world.water.height = 8;


	return true;
}



MENUCOMMAND(random4)
{
	int i,x,y;
	int smooth  = 3;//atoi(Graphics.WM.InputWindow("Smoothing level (use 5-10 for decent results)").c_str());

	undostack.push(new cUndoHeightEdit(0,0,Graphics.world.width, Graphics.world.height));


	Graphics.world.tiles.clear();
	for(int tex = 0; tex < 5; tex++)
	{
		for(y = 0; y < 5; y++)
		{
			for(x = 0; x < 5; x++)
			{
				cTile t;
				t.lightmap = 1;
				t.texture = tex;
				t.u1 = x/5.0;
				t.v1 = y/5.0;
				t.u2 = (x+1)/5.0;
				t.v2 = (y)/5.0;
				t.u3 = (x)/5.0;
				t.v3 = (y+1)/5.0;
				t.u4 = (x+1)/5.0;
				t.v4 = (y+1)/5.0;
				t.color[0] = (char)255;
				t.color[1] = (char)255;
				t.color[2] = (char)255;
				t.color[3] = (char)255;
				Graphics.world.tiles.push_back(t);
			}
		}
	}

	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].tileaside = -1;
			Graphics.world.cubes[y][x].tileside = -1;
			Graphics.world.cubes[y][x].tileup = 25 + ((int)x%5) + 5*((int)y%5);
		}
	}

	float xding = rand() % 50 + 20;
	float yding = rand() % 50 + 20;
	float zding = rand() % 50 + 20;

	float xding2 = rand() % 50 + 50;
	float yding2 = rand() % 50 + 50;
	float zding2 = rand() % 50 + 50;

	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{

			float f = xding2*sin(y/xding)+yding2*cos(x/yding) + zding*cos(x/zding+y/zding);
			f = floor(f / 32.0f)*32;

			Graphics.world.cubes[y][x].cell1 = f;
			Graphics.world.cubes[y][x].cell2 = f;
			Graphics.world.cubes[y][x].cell3 = f;
			Graphics.world.cubes[y][x].cell4 = f;
		}
	}
	eMode m = editmode;
	editmode = MODE_HEIGHTDETAIL;

	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_s;
	for(i = 0; i < 3; i++)
		SDL_PushEvent(&ev);

	int b = brushsize;
	mouse3dx = Graphics.world.width*5;
	mouse3dz = Graphics.world.height*5;
	brushsize = Graphics.world.width+Graphics.world.height;
	
	process_events();
	brushsize = b;


	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_s;
	SDL_PushEvent(&ev);
	editmode = m;




	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			if(fabs(Graphics.world.cubes[y][x].cell1 - Graphics.world.cubes[y][x].cell2) > 5 ||
				fabs(Graphics.world.cubes[y][x].cell1 - Graphics.world.cubes[y][x].cell3) > 5 ||
				fabs(Graphics.world.cubes[y][x].cell1 - Graphics.world.cubes[y][x].cell4) > 5 ||
				fabs(Graphics.world.cubes[y][x].cell2 - Graphics.world.cubes[y][x].cell3) > 5 ||
				fabs(Graphics.world.cubes[y][x].cell2 - Graphics.world.cubes[y][x].cell4) > 5 ||
				fabs(Graphics.world.cubes[y][x].cell3 - Graphics.world.cubes[y][x].cell4) > 5)
				Graphics.world.cubes[y][x].tileup = 50 + ((int)x%5) + 5*((int)y%5);
		}
	}


	vector<string>	randommodels;
	for(i = 0; i < objectfiles.size(); i++)
	{
		cFile* pFile = fs.open(objectfiles[i]);
		while(!pFile->eof())
		{
			string line = pFile->readline();
			if (line == "")
				continue;
			string pre = line.substr(0, line.find("|"));
			string filename = line.substr(line.find("|")+1);

			string cat = pre.substr(0, pre.rfind("/"));
			string name = pre.substr(pre.rfind("/")+1);

			if(cat == "randomtrees")
				randommodels.push_back(filename);
		}
	}



	for(i = 0; i < 1000; i++)
	{
		cRSMModel* model = new cRSMModel();
		model->load(rodir +  randommodels[rand() % randommodels.size()]);

		model->pos = cVector3(rand()%(Graphics.world.width*2), 0, rand()%(Graphics.world.height*2));

		while(Graphics.world.cubes[model->pos.z/2][model->pos.x/2].tileup > 50)
			model->pos = cVector3(rand()%(Graphics.world.width*2), 0, rand()%(Graphics.world.height*2));


		model->pos.y = Graphics.world.cubes[model->pos.z/2][model->pos.x/2].cell1;
		model->scale = cVector3(1,1,1);
		model->rot = cVector3(0,0,0);
		Graphics.world.models.push_back(model);
	}


	return true;
}



MENUCOMMAND(mode)
{
	string title = src->title;
	for(int i = 0; i < mode->items.size(); i++)
		mode->items[i]->ticked = false;
	src->ticked = true;

	if(title == GetMsg("menu/editmode/GLOBALHEIGHTEDIT"))
	{
		editmode = MODE_HEIGHTGLOBAL;
		if (Graphics.texturestart >= Graphics.world.textures.size())
			Graphics.texturestart = 0;
	}
	else if (title == GetMsg("menu/editmode/DETAILTERRAINEDIT"))
	{
		editmode = MODE_HEIGHTDETAIL;
		if (Graphics.texturestart >= Graphics.world.textures.size())
			Graphics.texturestart = 0;
	}
	else if (title == GetMsg("menu/editmode/TEXTUREEDIT"))
	{
		editmode = MODE_TEXTURE;
		if (Graphics.texturestart >= Graphics.world.textures.size())
			Graphics.texturestart = 0;
	}
	else if (title == GetMsg("menu/editmode/WALLEDIT"))
	{
		editmode = MODE_WALLS;
	}
	else if (title == GetMsg("menu/editmode/OBJECTEDIT"))
	{
		editmode = MODE_OBJECTS;
		if (Graphics.texturestart >= Graphics.world.textures.size())
			Graphics.texturestart = 0;
	}
	else if (title == GetMsg("menu/editmode/GATEDIT"))
	{
		editmode = MODE_GAT;
		if (Graphics.texturestart >= 6)
			Graphics.texturestart = 0;
	}
	else if (title == GetMsg("menu/editmode/WATEREDIT"))
	{
		editmode = MODE_WATER;
		Graphics.texturestart = Graphics.world.water.type;
	}
	else if (title == GetMsg("menu/editmode/EFFECTSEDIT"))
	{
		editmode = MODE_EFFECTS;
		Graphics.selectedobject = -1;
	}
	else if (title == GetMsg("menu/editmode/SOUNDSEDIT"))
	{
		editmode = MODE_SOUNDS;
	}
	else if (title == GetMsg("menu/editmode/LIGHTSEDIT"))
	{
		editmode = MODE_LIGHTS;
	}
	else if (title == GetMsg("menu/editmode/OBJECTGROUPEDIT"))
	{
		editmode = MODE_OBJECTGROUP;
	}
	else if (title == GetMsg("menu/editmode/SPRITEEDIT"))
	{
		editmode = MODE_SPRITE;
	}
	return true;
}

MENUCOMMAND(flatten)
{
	undostack.push(new cUndoHeightEdit(0,0,Graphics.world.width, Graphics.world.height));
	for(int y = 0; y < Graphics.world.height; y++)
	{
		for(int x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].tileaside = -1;
			Graphics.world.cubes[y][x].tileside = -1;
			Graphics.world.cubes[y][x].cell1 = 0;
			Graphics.world.cubes[y][x].cell2 = 0;
			Graphics.world.cubes[y][x].cell3 = 0;
			Graphics.world.cubes[y][x].cell4 = 0;
			Graphics.world.cubes[y][x].calcnormal();
		}
	}
	return true;	
}

MENUCOMMAND(grid)
{
	src->ticked = !src->ticked;
	Graphics.showgrid = src->ticked;
	return true;
}

MENUCOMMAND(mode_detail)
{
	int i;
	for(i = 0; i < mode->items.size(); i++)
		mode->items[i]->ticked = (mode->items[i]->title == "Detail Terrain Edit" ? true : false);
	for(i = 0; i < editdetail->items.size(); i++)
		editdetail->items[i]->ticked = false;
	src->ticked = true;
	editmode = MODE_HEIGHTDETAIL;
	brushsize = atoi(src->title.c_str());

	return true;
}
MENUCOMMAND(speed)
{
	for(int i =0 ; i < speed->items.size(); i++)
		speed->items[i]->ticked = false;
	src->ticked = true;
	paintspeed = atof(src->title.c_str());
	return true;
}

MENUCOMMAND(fill)
{
	int x,y,i;
	map<int, bool, less<int> > used;

	Graphics.world.tiles.clear();
	Graphics.world.lightmaps.clear();
	cLightmap* map = new cLightmap();
	for(i = 0; i < 256; i++)
		map->buf[i] = i < 64 ? 255 : 0;
	Graphics.world.lightmaps.push_back(map);
	map = new cLightmap();
	for(i = 0; i < 256; i++)
		map->buf[i] = i < 64 ? 255 : 0;
	Graphics.world.lightmaps.push_back(map);
	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 4; x++)
		{
			cTile t;
			t.lightmap = 1;
			t.texture = Graphics.texturestart + ((int)Graphics.selectionstart.y - 32) / 288;
			t.u1 = x/4.0;
			t.v1 = y/4.0;
			t.u2 = (x+1)/4.0;
			t.v2 = (y)/4.0;
			t.u3 = (x)/4.0;
			t.v3 = (y+1)/4.0;
			t.u4 = (x+1)/4.0;
			t.v4 = (y+1)/4.0;
			t.color[0] = (char)255;
			t.color[1] = (char)255;
			t.color[2] = (char)255;
			t.color[3] = (char)255;
			Graphics.world.tiles.push_back(t);
		}
	}


	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			Graphics.world.cubes[y][x].tileup = (x%4) + 4*(y % 4);
			Graphics.world.cubes[y][x].tileside = -1;
			Graphics.world.cubes[y][x].tileaside = -1;
		}
	}


	for(x = 0; x < Graphics.world.width; x++)
	{
		for(y = 0; y < Graphics.world.height; y++)
		{
			int tile = Graphics.world.cubes[y][x].tileup;
			if(used.find(tile) != used.end())
			{
				cTile t = Graphics.world.tiles[tile];
				tile = Graphics.world.tiles.size();
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileup = tile;
			}
			used[tile] = 1;
///////////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileside;
			if (tile != -1)
			{
				if(used.find(tile) != used.end())
				{
					cTile t = Graphics.world.tiles[tile];
					tile = Graphics.world.tiles.size();
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileside = tile;
				}
				used[tile] = 1;
			}
/////////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileaside;
			if (tile!= -1)
			{
				if(used.find(tile) != used.end())
				{
					cTile t = Graphics.world.tiles[tile];
					tile = Graphics.world.tiles.size();
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileaside = tile;
				}
				used[tile] = 1;
			}
		}
	}


	

	return true;
}

MENUCOMMAND(showobjects)
{
	src->ticked = !src->ticked;
	Graphics.showobjects = src->ticked;
	return true;
}


MENUCOMMAND(model)
{
	delete Graphics.previewmodel;
	Graphics.previewmodel = new cRSMModel();
	Graphics.previewmodel->load(rodir + src->data);
	Graphics.previewmodel->rot = cVector3(0,0,0);
	Graphics.previewmodel->scale = cVector3(4,4,4);

	Graphics.previewmodel->pos = cVector3(40,-40,-40);

	if (editmode != MODE_OBJECTS)
		Graphics.previewcolor = 200;
	currentobject = src;
	return true;
}


MENUCOMMAND(slope)
{
	src->ticked = !src->ticked;
	Graphics.slope = src->ticked;
	return true;
}

MENUCOMMAND(quadtree)
{
	int x,y;
	for(x = 0; x < Graphics.world.width; x++)
		for(y = 0; y < Graphics.world.height; y++)
		{
			Graphics.world.cubes[y][x].maxh = -99999;
			Graphics.world.cubes[y][x].minh = 99999;
		}

	for(int i = 0; i < Graphics.world.models.size(); i++)
	{
		Log(3,0,GetMsg("CALCMODEL"), i, Graphics.world.models.size(), (i/(float)Graphics.world.models.size())*100);
		Graphics.world.models[i]->draw(false,false,true);
	}


	Graphics.world.root->recalculate();
	return true;
}

MENUCOMMAND(gatheight)
{
	undostack.push(new cUndoGatHeightEdit(0,0,Graphics.world.gattiles[0].size(), Graphics.world.gattiles.size()));
	int x,y;
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			cCube* c = &Graphics.world.cubes[y][x];
			Graphics.world.gattiles[2*y][2*x].cell1 = (c->cell1+c->cell1) / 2.0f;
			Graphics.world.gattiles[2*y][2*x].cell2 = (c->cell1+c->cell2) / 2.0f;
			Graphics.world.gattiles[2*y][2*x].cell3 = (c->cell1+c->cell3) / 2.0f;
			Graphics.world.gattiles[2*y][2*x].cell4 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;

			Graphics.world.gattiles[2*y][2*x+1].cell1 = (c->cell1+c->cell2) / 2.0f;
			Graphics.world.gattiles[2*y][2*x+1].cell2 = (c->cell2+c->cell2) / 2.0f;
			Graphics.world.gattiles[2*y][2*x+1].cell3 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
			Graphics.world.gattiles[2*y][2*x+1].cell4 = (c->cell4+c->cell2) / 2.0f;

			Graphics.world.gattiles[2*y+1][2*x+1].cell1 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
			Graphics.world.gattiles[2*y+1][2*x+1].cell2 = (c->cell4 + c->cell2) / 2.0f;
			Graphics.world.gattiles[2*y+1][2*x+1].cell3 = (c->cell4 + c->cell3) / 2.0f;
			Graphics.world.gattiles[2*y+1][2*x+1].cell4 = (c->cell4 + c->cell4) / 2.0f;

			Graphics.world.gattiles[2*y+1][2*x].cell1 = (c->cell3 + c->cell1) / 2.0f;
			Graphics.world.gattiles[2*y+1][2*x].cell2 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
			Graphics.world.gattiles[2*y+1][2*x].cell3 = (c->cell3 + c->cell3) / 2.0f;
			Graphics.world.gattiles[2*y+1][2*x].cell4 = (c->cell3 + c->cell4) / 2.0f;
		}

	}
	
	return true;
}




cVector3 lightpos = cVector3(-20000,20000,-20000);
bool selectonly;
bool lightonly;

MENUCOMMAND(dolightmaps)
{
	selectonly = true;
	lightonly = false;
	return MenuCommand_dolightmaps2(src);
}
MENUCOMMAND(dolightmapsall)
{
	selectonly = false;
	lightonly = false;
	return MenuCommand_dolightmaps2(src);
}
MENUCOMMAND(dolightmapslights)
{
	selectonly = false;
	lightonly = true;
	return MenuCommand_dolightmaps2(src);
}



MENUCOMMAND(dolightmaps2)
{
	int x,y,i;

	map<int, bool, less<int> > used;

	for(x = 0; x < Graphics.world.width; x++)
	{
		for(y = 0; y < Graphics.world.height; y++)
		{
			if(selectonly && !Graphics.world.cubes[y][x].selected)
				continue;
			int tile = Graphics.world.cubes[y][x].tileup;
			if(used.find(tile) != used.end())
			{
				cTile t = Graphics.world.tiles[tile];
				tile = Graphics.world.tiles.size();
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileup = tile;
			}
			used[tile] = 1;
			if(tile != -1)
			{
				cLightmap* map = new cLightmap();
				for(int i = 0; i < 256; i++)
					map->buf[i] = i < 64 ? 0 : 0;
				Graphics.world.tiles[tile].lightmap = Graphics.world.lightmaps.size();
				Graphics.world.lightmaps.push_back(map);
			}
///////////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileside;
			if (tile != -1)
			{
				if(used.find(tile) != used.end())
				{
					cTile t = Graphics.world.tiles[tile];
					tile = Graphics.world.tiles.size();
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileside = tile;
				}
				used[tile] = 1;
				if(tile != -1)
				{
					cLightmap* map = new cLightmap();
					for(int i = 0; i < 256; i++)
						map->buf[i] = i < 64 ? 255 : 0;
					Graphics.world.tiles[tile].lightmap = Graphics.world.lightmaps.size();
					Graphics.world.lightmaps.push_back(map);
				}
			}
/////////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileaside;
			if (tile!= -1)
			{
				if(used.find(tile) != used.end())
				{
					cTile t = Graphics.world.tiles[tile];
					tile = Graphics.world.tiles.size();
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileaside = tile;
				}
				used[tile] = 1;
				if(tile != -1)
				{
					cLightmap* map = new cLightmap();
					for(int i = 0; i < 256; i++)
						map->buf[i] = i < 64 ? 255 : 0;
					Graphics.world.tiles[tile].lightmap = Graphics.world.lightmaps.size();
					Graphics.world.lightmaps.push_back(map);
				}
			}
		}
	}

	int ww = Graphics.w();
	ww -= 256;
	int hh = Graphics.h()-20;

	glEnable(GL_DEPTH_TEST);
	glViewport(0,0,ww,hh);						// Reset The Current Viewport

	float camrad = 10;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f,(GLfloat)Graphics.w()/(GLfloat)Graphics.h(),10.0f,10000.0f);
	gluLookAt(  -Graphics.camerapointer.x + Graphics.cameraheight*sin(Graphics.camerarot),
				camrad+Graphics.cameraheight,
				-Graphics.camerapointer.y + Graphics.cameraheight*cos(Graphics.camerarot),
				-Graphics.camerapointer.x,camrad + Graphics.cameraheight * (Graphics.cameraangle/10.0f),-Graphics.camerapointer.y,
				0,1,0);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
//	glTranslatef(0,0,Graphics.world.height*10);
//	glScalef(1,1,-1);

	for(i = 0; i < Graphics.world.models.size(); i++)
		Graphics.world.models[i]->precollides();

	for(y = 0; y < Graphics.world.height; y++)
//	for(y = 40; y < 60; y++)
	{
		if(y == 99)
			Sleep(0);
		for(x = 0; x < Graphics.world.width; x++)
//		for(x = 40; x < 60; x++)
		{
			cCube* c = &Graphics.world.cubes[y][x];
			if(selectonly && !c->selected)
				continue;
			Log(3,0,GetMsg("PERCENTAGE"), (y*Graphics.world.width+x) / (float)(Graphics.world.height * Graphics.world.width)*100); // %f %%
			if(Graphics.world.cubes[y][x].tileup == -1)
				continue;
			Graphics.world.reallightmaps[y][x]->reset();
			BYTE* buf = (BYTE*)Graphics.world.lightmaps[Graphics.world.tiles[Graphics.world.cubes[y][x].tileup].lightmap]->buf;

			for(int yy = 0; yy < 6; yy++)
			{
				for(int xx = 0; xx < 6; xx++)
				{
					cVector3 worldpos = cVector3(	10*x+(10/6.0)*xx, 
													-((Graphics.world.cubes[y][x].cell1+Graphics.world.cubes[y][x].cell2+Graphics.world.cubes[y][x].cell3+Graphics.world.cubes[y][x].cell4)/4),
													10*y+(10/6.0)*yy);
					
					int from = 0;
					int to = Graphics.world.lights.size();
					if(lightonly)
					{
						from = Graphics.selectedobject;
						to = from+1;
					}
					for(i = from; i < to; i++)
					{
						cLight* l = &Graphics.world.lights[i];
						cVector3 diff = worldpos - cVector3(l->pos.x*5, l->pos.y, l->pos.z*5);
						float bla = diff.Magnitude();
						bool obstructed = false;
						if(l->todo2-(0.1*bla*bla) < 0)
							continue;
						if(buf[yy*8 + xx + 9] == 255)
							continue;

						for(int ii = 0; ii < Graphics.world.models.size() && !obstructed; ii++)
						{
							if(Graphics.world.models[ii]->collides(worldpos, cVector3(l->pos.x*5, l->pos.y, l->pos.z*5)))
								obstructed = true;
						}

						if(!obstructed)
						{
							buf[yy*8 + xx + 9] = min(255, buf[yy*8 + xx + 9] + max(0, (int)(l->todo2 - (0.1*bla*bla))));

							buf[64 + 3*(yy*8 + xx + 9)+0] = min(255, buf[64 + 3*(yy*8 + xx + 9)+0] + max(0, (int)ceil((l->todo2 - bla)*l->color.x)));
							buf[64 + 3*(yy*8 + xx + 9)+1] = min(255, buf[64 + 3*(yy*8 + xx + 9)+1] + max(0, (int)ceil((l->todo2 - bla)*l->color.y)));
							buf[64 + 3*(yy*8 + xx + 9)+2] = min(255, buf[64 + 3*(yy*8 + xx + 9)+2] + max(0, (int)ceil((l->todo2 - bla)*l->color.z)));
						}
						else
						{
						}
					}
				}
			}
		}
	}



/*
	for(i = 0; i < Graphics.world.models.size(); i++)
	{
		Log(3,0,"Doing model %i out of %i (%.2f%%)", i, Graphics.world.models.size(), (i/(float)Graphics.world.models.size())*100);
		Graphics.world.models[i]->draw(false,false,false, true);
	}*/

/*	float t;
	for(x = 0; x < Graphics.world.width; x++)
	{
		Log(3,0,"%f%%", (x/(float)Graphics.world.width)*100.0f);
		for(y = 0; y < Graphics.world.height; y++)
		{
			int tile = Graphics.world.cubes[y][x].tileup;
			if (tile != -1)
			{
				float cellheight = -Graphics.world.cubes[y][x].cell1;
				cLightmap* l = Graphics.world.lightmaps[Graphics.world.tiles[tile].lightmap];
				for(int xx = 0; xx < 6; xx++)
				{
					for(int yy = 0; yy < 6; yy++)
					{
						cVector3 pos = cVector3(10*x+10*(xx/6.0),cellheight, 10*y+10*(yy/6.0));
						char* lightmappos = &l->buf[xx + (8*yy)+1+8];

						for(int xxx = max(0,x - 1); xxx <= min(Graphics.world.width-1,x+1); xxx++)
						{
							for(int yyy = max(0,y - 1); yyy <= min(Graphics.world.height-1,y+1); yyy++)
							{
								if(*lightmappos == 127)
									break;
								if (xxx == x && yyy == y)
									continue;
								cCube* c = &Graphics.world.cubes[yyy][xxx];
								cVector3 triangle[6];
								triangle[2] = cVector3(xxx*10+10, -c->cell2, yyy*10);
								triangle[1] = cVector3(xxx*10, -c->cell3, yyy*10-10);
								triangle[0] = cVector3(xxx*10+10, -c->cell4, yyy*10-10);

								triangle[5] = cVector3(xxx*10, -c->cell4, yyy*10-10);
								triangle[4] = cVector3(xxx*10+10, -c->cell2, yyy*10);
								triangle[3] = cVector3(xxx*10, -c->cell1, yyy*10);

								if (LineIntersectPolygon(triangle, 3, lightpos, pos, t))
								{
									if (t < 1)
										*lightmappos = 127;//((BYTE)l->buf[xx + (8*yy)+1+8]) / 1.6;
								}
								else if (LineIntersectPolygon(triangle+3, 3, lightpos, pos, t))
								{
									if (t < 1)
										*lightmappos = 127;//((BYTE)l->buf[xx + (8*yy)+1+8]) / 1.6;
								}

								if (c->tileaside != -1)
								{
									triangle[2] = cVector3(xxx*10+10,-c->cell4,yyy*10);
									triangle[1] = cVector3(xxx*10+10,-(c+1)->cell1,yyy*10);
									triangle[0] = cVector3(xxx*10+10,-(c+1)->cell3,yyy*10+10);

									triangle[3] = cVector3(xxx*10+10,-(c+1)->cell1,yyy*10+10);
									triangle[4] = cVector3(xxx*10+10,-c->cell4,yyy*10+10);
									triangle[5] = cVector3(xxx*10+10,-c->cell2,yyy*10);

									if (LineIntersectPolygon(triangle, 3, lightpos, pos, t))
									{
										if (t < 1)
											*lightmappos = 127;//((BYTE)l->buf[xx + (8*yy)+1+8]) / 1.6;
									}
									else if (LineIntersectPolygon(triangle+3, 3, lightpos, pos, t))
									{
										if (t < 1)
											*lightmappos = 127;//((BYTE)l->buf[xx + (8*yy)+1+8]) / 1.6;
									}
								}
								if (c->tileside != -1 && y < Graphics.world.width - 1)
								{

									triangle[0] = cVector3(xxx*10,-c->cell3,yyy*10+10);
									triangle[1] = cVector3(xxx*10+10,-c->cell4,yyy*10+10);
									triangle[2] = cVector3(xxx*10,-Graphics.world.cubes[y+1][x].cell1,yyy*10+10);
									
									
									triangle[3] = cVector3(xxx*10+10,-Graphics.world.cubes[y+1][x].cell2,yyy*10+10);
									triangle[4] = cVector3(xxx*10,-Graphics.world.cubes[y+1][x].cell1,yyy*10+10);
									triangle[5] = cVector3(xxx*10+10,-c->cell4,yyy*10+10);

									if (LineIntersectPolygon(triangle, 3, lightpos, pos, t))
									{
										if(t < 1)
											*lightmappos = 127;//((BYTE)l->buf[xx + (8*yy)+1+8]) / 1.6;
									}
									else if (LineIntersectPolygon(triangle+3, 3, lightpos, pos, t))
									{
										if(t < 1)
											*lightmappos = 127;//((BYTE)l->buf[xx + (8*yy)+1+8]) / 1.6;
									}
								}
							
							}
						}
					}
				}

			}
				
		}
	}
*/

	for(x = 1; x < (Graphics.world.width*6)-1; x++)
	{
		for(y = 1; y < (Graphics.world.height*6)-1; y++)
		{

		}
	}

	int lightmap,lightmapleft,lightmaptop,lightmapright,lightmapbottom;
	cLightmap* map;
	cLightmap* mapleft;
	cLightmap* maptop;
	cLightmap* mapright;
	cLightmap* mapbottom;


	for(x = 1; x < Graphics.world.width-1; x++)
	{
		for(y = 1; y < Graphics.world.height-1; y++)
		{
			int tile = Graphics.world.cubes[y][x].tileup;
			int tileleft = Graphics.world.cubes[y][x-1].tileup;
			int tiletop = Graphics.world.cubes[y-1][x].tileup;
			int tileright = Graphics.world.cubes[y][x+1].tileup;
			int tilebottom = Graphics.world.cubes[y+1][x].tileup;
			if (tile != -1)
			{
				if(tile != -1)
					lightmap = Graphics.world.tiles[tile].lightmap;
				if(tileleft != -1)
					lightmapleft = Graphics.world.tiles[tileleft].lightmap;
				if(tiletop != -1)
					lightmaptop = Graphics.world.tiles[tiletop].lightmap;
				if(tileright != -1)
					lightmapright = Graphics.world.tiles[tileright].lightmap;
				if(tilebottom != -1)
					lightmapbottom = Graphics.world.tiles[tilebottom].lightmap;

				if(tile != -1)
					map = Graphics.world.lightmaps[lightmap];
				if(tileleft != -1)
					mapleft = Graphics.world.lightmaps[lightmapleft];
				if(tiletop != -1)
					maptop = Graphics.world.lightmaps[lightmaptop];
				if(tileright != -1)
					mapright = Graphics.world.lightmaps[lightmapright];
				if(tilebottom != -1)
					mapbottom = Graphics.world.lightmaps[lightmapbottom];

				for(i = 0; i < 8; i++)
				{
					if(tileleft != -1)
						mapleft->buf[8*i+7] = map->buf[8*i+1];
					if(tiletop != -1)
						maptop->buf[7*8+i] = map->buf[i+8];
					if(tileright != -1)
						mapright->buf[8*i] = map->buf[8*i+6];
					if(tilebottom != -1)
						mapbottom->buf[i] = map->buf[6*8+i];
				}
			}
				
		}
	}
		

	


	return true;
}

MENUCOMMAND(fixcolors)
{
	int x,y;
	for(x = 0; x < Graphics.world.width; x++)
		for(y = 0; y < Graphics.world.height; y++)
		{
			int tile = Graphics.world.cubes[y][x].tileup;
			if(tile != -1)
			{
				Graphics.world.tiles[tile].color[0] = '\255';
				Graphics.world.tiles[tile].color[1] = '\255';
				Graphics.world.tiles[tile].color[2] = '\255';
				Graphics.world.tiles[tile].color[3] = '\255';
			}			
		}

	return true;
}


MENUCOMMAND(savelightmaps)
{
	Graphics.world.savelightmap();
	return true;
}

MENUCOMMAND(loadlightmaps)
{
	int x,y,i;

	map<int, bool, less<int> > used;
	for(x = 0; x < Graphics.world.width; x++)
	{
		for(y = 0; y < Graphics.world.height; y++)
		{
			Graphics.world.reallightmaps[y][x]->reset();
			int tile = Graphics.world.cubes[y][x].tileup;
			if(used.find(tile) != used.end())
			{
				cTile t = Graphics.world.tiles[tile];
				tile = Graphics.world.tiles.size();
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileup = tile;
			}
			used[tile] = 1;
			if(tile != -1)
			{
				cLightmap* map = new cLightmap();
				for(int i = 0; i < 256; i++)
					map->buf[i] = i < 64 ? 255 : 0;
				Graphics.world.tiles[tile].lightmap = Graphics.world.lightmaps.size();
				Graphics.world.lightmaps.push_back(map);
			}
//////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileside;
			if(used.find(tile) != used.end() && tile != -1)
			{
				cTile t = Graphics.world.tiles[tile];
				tile = Graphics.world.tiles.size();
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileside = tile;
			}
			used[tile] = 1;
			if(tile != -1)
			{
				cLightmap* map = new cLightmap();
				for(int i = 0; i < 256; i++)
					map->buf[i] = i < 64 ? 255 : 0;
				Graphics.world.tiles[tile].lightmap = Graphics.world.lightmaps.size();
				Graphics.world.lightmaps.push_back(map);
			}
////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileaside;
			if(used.find(tile) != used.end() && tile != -1)
			{
				cTile t = Graphics.world.tiles[tile];
				tile = Graphics.world.tiles.size();
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileaside = tile;
			}
			used[tile] = 1;
			if(tile != -1)
			{
				cLightmap* map = new cLightmap();
				for(int i = 0; i < 256; i++)
					map->buf[i] = i < 64 ? 255 : 0;
				Graphics.world.tiles[tile].lightmap = Graphics.world.lightmaps.size();
				Graphics.world.lightmaps.push_back(map);
			}
		}
	}


	Graphics.world.loadlightmap();

		int lightmap,lightmapleft,lightmaptop,lightmapright,lightmapbottom;
	cLightmap* map;
	cLightmap* mapleft;
	cLightmap* maptop;
	cLightmap* mapright;
	cLightmap* mapbottom;


	for(x = 1; x < Graphics.world.width-1; x++)
	{
		for(y = 1; y < Graphics.world.height-1; y++)
		{
			int tile = Graphics.world.cubes[y][x].tileup;
			int tileleft = Graphics.world.cubes[y][x-1].tileup;
			int tiletop = Graphics.world.cubes[y-1][x].tileup;
			int tileright = Graphics.world.cubes[y][x+1].tileup;
			int tilebottom = Graphics.world.cubes[y+1][x].tileup;
			if (tile != -1)
			{
				if(tile != -1)
					lightmap = Graphics.world.tiles[tile].lightmap;
				if(tileleft != -1)
					lightmapleft = Graphics.world.tiles[tileleft].lightmap;
				if(tiletop != -1)
					lightmaptop = Graphics.world.tiles[tiletop].lightmap;
				if(tileright != -1)
					lightmapright = Graphics.world.tiles[tileright].lightmap;
				if(tilebottom != -1)
					lightmapbottom = Graphics.world.tiles[tilebottom].lightmap;

				if(tile != -1)
					map = Graphics.world.lightmaps[lightmap];
				if(tileleft != -1)
					mapleft = Graphics.world.lightmaps[lightmapleft];
				if(tiletop != -1)
					maptop = Graphics.world.lightmaps[lightmaptop];
				if(tileright != -1)
					mapright = Graphics.world.lightmaps[lightmapright];
				if(tilebottom != -1)
					mapbottom = Graphics.world.lightmaps[lightmapbottom];

				for(i = 0; i < 8; i++)
				{
					if(tileleft != -1)
						mapleft->buf[8*i+7] = map->buf[8*i+1];
					if(tiletop != -1)
						maptop->buf[7*8+i] = map->buf[i+8];
					if(tileright != -1)
						mapright->buf[8*i] = map->buf[8*i+6];
					if(tilebottom != -1)
						mapbottom->buf[i] = map->buf[6*8+i];
				}

				for(i = 0; i < 8; i++)
				{
					if(tileleft != -1)
					{
						mapleft->buf[64+3*(8*i+7)] = map->buf[64+3*(8*i+1)];
						mapleft->buf[64+3*(8*i+7)+1] = map->buf[64+3*(8*i+1)+1];
						mapleft->buf[64+3*(8*i+7)+2] = map->buf[64+3*(8*i+1)+2];
					}
					if(tiletop != -1)
					{
						maptop->buf[64+3*(7*8+i)] = map->buf[64+3*(i+8)];
						maptop->buf[64+3*(7*8+i)+1] = map->buf[64+3*(i+8)+1];
						maptop->buf[64+3*(7*8+i)+2] = map->buf[64+3*(i+8)+2];
					}
					if(tileright != -1)
					{
						mapright->buf[64+3*(8*i)] = map->buf[64+3*(8*i+6)];
						mapright->buf[64+3*(8*i)+1] = map->buf[64+3*(8*i+6)+1];
						mapright->buf[64+3*(8*i)+2] = map->buf[64+3*(8*i+6)+2];
					}
					if(tilebottom != -1)
					{
						mapbottom->buf[64+3*(i)] = map->buf[64+3*(6*8+i)];
						mapbottom->buf[64+3*(i)+1] = map->buf[64+3*(6*8+i)+1];
						mapbottom->buf[64+3*(i)+2] = map->buf[64+3*(6*8+i)+2];
					}
				}
			
			
			}
				
		}
	}

	return true;
}


MENUCOMMAND(clearobjects)
{
	int i;
	vector<cUndoObjectsDelete::cObject> objectsdeleted;
	for(i = 0; i < Graphics.world.models.size(); i++)
	{
		cUndoObjectsDelete::cObject object;
		object.filename = Graphics.world.models[i]->filename;
		object.pos = Graphics.world.models[i]->pos;
		object.rot = Graphics.world.models[i]->rot;
		object.scale = Graphics.world.models[i]->scale;
		object.id = i;
		objectsdeleted.push_back(object);
	}
	undostack.push(new cUndoObjectsDelete(objectsdeleted));
	for(i = 0; i < Graphics.world.models.size(); i++)
		delete Graphics.world.models[i];
	Graphics.world.models.clear();
	return true;
}

MENUCOMMAND(addwalls)
{
	int x,y;
	for(x = 0; x < Graphics.world.width-1; x++)
	{
		for(y = 0; y < Graphics.world.height-1; y++)
		{
			cCube* c = &Graphics.world.cubes[y][x];
			if (c->tileaside == -1)
			{
				if (c->cell4 != (c+1)->cell1 && c->cell2 != (c+1)->cell3)
				{
					cTile t;
					t.color[0] = (char)255;
					t.color[1] = (char)255;
					t.color[2] = (char)255;
					t.color[3] = (char)255;
					t.texture = Graphics.texturestart + ((int)Graphics.selectionstart.y - 32) / 288;
					t.lightmap = 0;
					t.u1 = 0;
					t.v1 = 0;

					t.u2 = 1;
					t.v2 = 0;
					
					t.u3 = 0;
					t.v3 = 1;
					
					t.u4 = 1;
					t.v4 = 1;
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileaside = Graphics.world.tiles.size()-1;
				}
			}
			if (c->tileside == -1)
			{
				if (c->cell4 != Graphics.world.cubes[y+1][x].cell1 && c->cell3 != Graphics.world.cubes[y+1][x].cell2)
				{
					cTile t;
					t.color[0] = (char)255;
					t.color[1] = (char)255;
					t.color[2] = (char)255;
					t.color[3] = (char)255;
					t.texture = Graphics.texturestart + ((int)Graphics.selectionstart.y - 32) / 288;
					t.lightmap = 0;
					t.u1 = 0;
					t.v1 = 0;

					t.u2 = 1;
					t.v2 = 0;
					
					t.u3 = 0;
					t.v3 = 1;
					
					t.u4 = 1;
					t.v4 = 1;
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileside = Graphics.world.tiles.size()-1;
				}
			}
		}
	}
	return true;
}

MENUCOMMAND(gatcollision)
{
	int x,y;
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			cCube* c = &Graphics.world.cubes[y][x];
			Graphics.world.gattiles[2*y][2*x].type = (c->cell1+c->cell2+c->cell3+c->cell4) != 0 ? '\1' : '\0';
			Graphics.world.gattiles[2*y][2*x+1].type = (c->cell1+c->cell2+c->cell3+c->cell4) != 0 ? '\1' : '\0';
			Graphics.world.gattiles[2*y+1][2*x].type = (c->cell1+c->cell2+c->cell3+c->cell4) != 0 ? '\1' : '\0';
			Graphics.world.gattiles[2*y+1][2*x+1].type = (c->cell1+c->cell2+c->cell3+c->cell4) != 0 ? '\1' : '\0';
		}

	}
	return true;
}

MENUCOMMAND(clearlightmaps)
{
	unsigned int i;
	for(i = 0; i < Graphics.world.lightmaps.size(); i++)
		delete 	Graphics.world.lightmaps[i];
	Graphics.world.lightmaps.clear();
	cLightmap* m = new cLightmap();
	for(i = 0; i < 256; i++)
		m->buf[i] = i < 64 ? 255 : 0;
	Graphics.world.lightmaps.push_back(m);

	for(i = 0; i < Graphics.world.tiles.size(); i++)
		Graphics.world.tiles[i].lightmap = 0;

	return true;
}


MENUCOMMAND(cleanuplightmaps)
{
	unsigned int i;
	vector<int> newvalue;
	map<int, bool, less<int> > used;
	for(i = 0; i < Graphics.world.lightmaps.size(); i++)
	{
		for(int ii = 0; ii < i; ii++)
		{
			if(memcmp(Graphics.world.lightmaps[i]->buf, Graphics.world.lightmaps[ii]->buf, 256) == 0)
			{
				newvalue.push_back(ii);
				break;
			}
		}
		if (newvalue.size() <= i)
			newvalue.push_back(i);
	}

	for(i = 0; i < Graphics.world.tiles.size(); i++)
	{
		Graphics.world.tiles[i].lightmap = newvalue[Graphics.world.tiles[i].lightmap];
	}
	return true;
}


MENUCOMMAND(tempfunc)
{
	glColor4f(1,1,1,0.7f);
	glEnable(GL_BLEND);

	cTile t;
	t.color[0] = (char)255;
	t.color[1] = (char)255;
	t.color[2] = (char)255;
	t.color[3] = (char)255;
	t.texture = Graphics.texturestart + ((int)Graphics.selectionstart.y - 32) / 288;
	t.lightmap = 0;
	t.u1 = 0;
	t.v1 = 0;

	t.u2 = 1;
	t.v2 = 0;
	
	t.u3 = 0;
	t.v3 = 1;
	
	t.u4 = 1;
	t.v4 = 1;
	Graphics.world.tiles.push_back(t);
	Graphics.world.tiles.push_back(t);
	int x,y;
	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			cCube* c = &Graphics.world.cubes[y][x];
			if(Graphics.world.lightmaps[Graphics.world.tiles[c->tileup].lightmap]->buf[10] == 0)
			{
				c->cell1 = -20;
				c->cell2 = -20;
				c->cell3 = -20;
				c->cell4 = -20;

				c->tileup = Graphics.world.tiles.size()-2;
				c->tileside = -1;
				c->tileaside = -1;
			}
			else
			{
				c->tileside = -1;
				c->tileaside = -1;
			}

		}

	}
	return true;

}

MENUCOMMAND(snaptofloor)
{
	src->ticked = !src->ticked;
	return true;
}


bool mouseovertexture(cMenu* src)
{
	if (Graphics.texturepreview == NULL || Graphics.texturepreview->getfilename() != rodir + "data\\texture\\" + ((cMenuItem*)src)->data)
	{
		Graphics.texturepreview = new cTexture();
		Graphics.texturepreview->Load(rodir + "data\\texture\\" + ((cMenuItem*)src)->data);
		return false;
	}
	else
	{
		Graphics.texturepreview->unLoad();
		delete Graphics.texturepreview;
		Graphics.texturepreview = NULL;
		return true;
	}
}
bool mouseouttexture(cMenu* src)
{
	if (Graphics.texturepreview != NULL)
	{
		Graphics.texturepreview->unLoad();
		delete Graphics.texturepreview;
		Graphics.texturepreview = NULL;
	}
	return true;
}

MENUCOMMAND(clearstuff)
{
	MenuCommand_flatten(src);
	MenuCommand_clearobjects(src);
	MenuCommand_clearlightmaps(src);
	MenuCommand_fill(src);
	MenuCommand_fixcolors(src);
	Graphics.world.effects.clear();
	Graphics.world.lights.clear();
	Graphics.world.sounds.clear();
	return true;
}

MENUCOMMAND(effect)
{
	if (selectedeffect != NULL)
		selectedeffect->ticked = false;
	src->ticked = true;
	if (Graphics.selectedobject != -1)
	{
		Graphics.world.effects[Graphics.selectedobject].type = atoi(src->data.c_str());
		Graphics.world.effects[Graphics.selectedobject].readablename = src->title;
	}
	selectedeffect = src;
	return true;
}

MENUCOMMAND(toggle)
{
	src->ticked = !src->ticked;
	*((bool*)src->pdata) = src->ticked;

	return true;
}

MENUCOMMAND(water)
{
	char buf[100];
	cWindow* w = new cWaterWindow(&Graphics.WM.texture, &Graphics.WM.font);
	sprintf(buf, "%f", Graphics.world.water.amplitude);		w->objects["amplitude"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.water.height);		w->objects["height"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.water.phase);			w->objects["phase"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.water.surfacecurve);	w->objects["surfacecurve"]->SetText(0,buf);
	sprintf(buf, "%i", Graphics.world.water.type);			w->objects["type"]->SetText(0,buf);
	Graphics.WM.addwindow(w);
	return true;
}

MENUCOMMAND(cleantextures)
{
	Graphics.world.clean();
	vector<bool> used;
	int i;
	used.resize(Graphics.world.textures.size(), false);
	for(i = 0; i < Graphics.world.tiles.size(); i++)
	{
		used[Graphics.world.tiles[i].texture] = true;
	}
	
	for(i = used.size()-1; i > -1; i--)
	{
		if (!used[i])
		{
			for(int ii = 0; ii < Graphics.world.tiles.size(); ii++)
			{
				if(Graphics.world.tiles[i].texture > i)
					Graphics.world.tiles[i].texture--;
			}
			TextureCache.unload(Graphics.world.textures[i]->texture);
			delete Graphics.world.textures[i]->texture;
			delete Graphics.world.textures[i];
			Graphics.world.textures.erase(Graphics.world.textures.begin() + i);
		}
	}

	return true;
}

MENUCOMMAND(ambientlight)
{
	char buf[100];
	cWindow* w = new cAmbientLightWindow(&Graphics.WM.texture, &Graphics.WM.font);
	sprintf(buf, "%i", Graphics.world.ambientlight.ambientr);		w->objects["ambientr"]->SetText(0,buf);
	sprintf(buf, "%i", Graphics.world.ambientlight.ambientg);		w->objects["ambientg"]->SetText(0,buf);
	sprintf(buf, "%i", Graphics.world.ambientlight.ambientb);		w->objects["ambientb"]->SetText(0,buf);

	sprintf(buf, "%f", Graphics.world.ambientlight.diffuse.x);		w->objects["diffuser"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.ambientlight.diffuse.y);		w->objects["diffuseg"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.ambientlight.diffuse.z);		w->objects["diffuseb"]->SetText(0,buf);

	sprintf(buf, "%f", Graphics.world.ambientlight.shadow.x);		w->objects["shadowr"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.ambientlight.shadow.y);		w->objects["shadowg"]->SetText(0,buf);
	sprintf(buf, "%f", Graphics.world.ambientlight.shadow.z);		w->objects["shadowb"]->SetText(0,buf);
	
	sprintf(buf, "%f", Graphics.world.ambientlight.alpha);			w->objects["alpha"]->SetText(0,buf);

	Graphics.WM.addwindow(w);
	return true;
}


MENUCOMMAND(cleareffects)
{
	int i;
	vector<int> objectsdeleted;
	for(i = 0; i < Graphics.world.effects.size(); i++)
		objectsdeleted.push_back(i);
	undostack.push(new cUndoEffectsDelete(objectsdeleted));

	Graphics.world.effects.clear();
	return true;
}

MENUCOMMAND(clearlights)
{
	Graphics.world.lights.clear();
	return true;
}

MENUCOMMAND(texturewindow)
{
	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_t;
	SDL_PushEvent(&ev);
	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_t;
	SDL_PushEvent(&ev);
	return true;
}

MENUCOMMAND(modelwindow)
{
	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_m;
	SDL_PushEvent(&ev);
	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_m;
	SDL_PushEvent(&ev);
	return true;
}

MENUCOMMAND(properties)
{
	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.keysym.sym = SDLK_RETURN;
	SDL_PushEvent(&ev);
	ev.type = SDL_KEYUP;
	ev.key.keysym.sym = SDLK_RETURN;
	SDL_PushEvent(&ev);
	return true;
}


MENUCOMMAND(preferences)
{
	Graphics.WM.addwindow(new cKeyBindWindow(&Graphics.WM.texture, &Graphics.WM.font));
	return true;
}



MENUCOMMAND(fillarea)
{
	int x,y,i;
	map<int, bool, less<int> > used;

	cLightmap* map = new cLightmap();
	for(i = 0; i < 256; i++)
		map->buf[i] = i < 64 ? 255 : 0;
	Graphics.world.lightmaps.push_back(map);
	map = new cLightmap();
	for(i = 0; i < 256; i++)
		map->buf[i] = i < 64 ? 255 : 0;
	Graphics.world.lightmaps.push_back(map);


	for(y = 0; y < Graphics.world.height; y++)
	{
		for(x = 0; x < Graphics.world.width; x++)
		{
			if(Graphics.world.cubes[y][x].selected)
			{
				int xx = x % 4;
				int yy = y % 4;
				cTile t;
				t.lightmap = Graphics.world.textures.size()-1;
				t.texture = Graphics.texturestart + ((int)Graphics.selectionstart.y - 32) / 288;
				t.u1 = xx/4.0;
				t.v1 = yy/4.0;
				t.u2 = (xx+1)/4.0;
				t.v2 = (yy)/4.0;
				t.u3 = (xx)/4.0;
				t.v3 = (yy+1)/4.0;
				t.u4 = (xx+1)/4.0;
				t.v4 = (yy+1)/4.0;
				t.color[0] = (char)255;
				t.color[1] = (char)255;
				t.color[2] = (char)255;
				t.color[3] = (char)255;
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileup = Graphics.world.tiles.size()-1;;
			}
		}
	}


	for(x = 0; x < Graphics.world.width; x++)
	{
		for(y = 0; y < Graphics.world.height; y++)
		{
			int tile = Graphics.world.cubes[y][x].tileup;
			if(used.find(tile) != used.end())
			{
				cTile t = Graphics.world.tiles[tile];
				tile = Graphics.world.tiles.size();
				Graphics.world.tiles.push_back(t);
				Graphics.world.cubes[y][x].tileup = tile;
			}
			used[tile] = 1;
///////////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileside;
			if (tile != -1)
			{
				if(used.find(tile) != used.end())
				{
					cTile t = Graphics.world.tiles[tile];
					tile = Graphics.world.tiles.size();
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileside = tile;
				}
				used[tile] = 1;
			}
/////////////////////////////////////
			tile = Graphics.world.cubes[y][x].tileaside;
			if (tile!= -1)
			{
				if(used.find(tile) != used.end())
				{
					cTile t = Graphics.world.tiles[tile];
					tile = Graphics.world.tiles.size();
					Graphics.world.tiles.push_back(t);
					Graphics.world.cubes[y][x].tileaside = tile;
				}
				used[tile] = 1;
			}
		}
	}


	

	return true;
}



MENUCOMMAND(rsmedit)
{
	Graphics.WM.addwindow(new cRSMEditWindow(&Graphics.WM.texture, &Graphics.WM.font));
	return true;
}

MENUCOMMAND(exportmapfiles)
{
	int i;
	ofstream pFile((string(Graphics.world.filename) + ".txt").c_str());
	for(i = 0; i < Graphics.world.textures.size(); i++)
	{
		cFile* pF = fs.open(rodir + "data\\texture\\" + Graphics.world.textures[i]->RoFilename);
		if(pF->location != -1)
		{
			pF->close();
			continue;
		}
		pF->close();


		
		pFile.write("texture\\", 8);
		pFile.write(Graphics.world.textures[i]->RoFilename.c_str(), Graphics.world.textures[i]->RoFilename.length());
		pFile.put('\r');
		pFile.put('\n');
	}

	map<string, bool, less<string> > usedmodels;

	for(i = 0; i < Graphics.world.models.size(); i++)
	{
		if(usedmodels.find(Graphics.world.models[i]->rofilename) != usedmodels.end())
			continue;

		cFile* pF = fs.open(Graphics.world.models[i]->filename);
		if(pF->location != -1)
		{
			pF->close();
			continue;
		}
		pF->close();



		usedmodels[Graphics.world.models[i]->rofilename] = true;
		pFile.write("model\\", 6);
		pFile.write(Graphics.world.models[i]->rofilename.c_str(), Graphics.world.models[i]->rofilename.length());
		pFile.put('\r');
		pFile.put('\n');

		for(int ii = 0; ii < Graphics.world.models[i]->textures.size(); ii++)
		{
			string file = Graphics.world.models[i]->textures[ii]->getfilename();
			cFile* pF = fs.open(file);
			if(pF->location != -1)
			{
				pF->close();
				continue;
			}
			pF->close();

			file = file.substr(rodir.length()+5);
			pFile.write(file.c_str(), file.length());
			pFile.put('\r');
			pFile.put('\n');
		}
		
	}


	pFile.close();
	ShellExecute(NULL,"open",(string(Graphics.world.filename) + ".txt").c_str(),NULL,"c:\\",SW_SHOW);
	return true;
}