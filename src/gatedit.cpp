#include <common.h>
#include <wm/window.h>
#include "graphics.h"
#include "undo.h"

extern long mousestartx, mousestarty;
extern double mouse3dx, mouse3dy, mouse3dz;
extern cUndoStack undostack;
extern bool lbuttondown, rbuttondown;
extern std::vector<std::vector<std::vector<float> > > clipboard;
std::vector<std::vector<int > > clipboardgat;


int cProcessManagement::gatedit_process_events(SDL_Event &event)
{
	switch(event.type)
	{
		case SDL_MOUSEMOTION:
			if(lbuttondown && !rbuttondown)
			{
				int posx = (int)mouse3dx / 5;
				int posy = (int)mouse3dz / 5;

				int f = (int)ceil(cGraphics::brushsize);

			//	if (posx >= floor(f/2.0f) && posx < 2*cGraphics::world->width-(int)ceil(f/2.0f) && posy >= floor(f/2.0f) && posy< 2*cGraphics::world->height-(int)ceil(f/2.0f))
				{
					undostack.push(new cUndoGatTileEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));

					glColor4f(1,0,0,1);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
					for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
					{
						for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
						{
							if (y < 0 || y >= cGraphics::world->height*2 || x < 0 || x >= cGraphics::world->width*2)
								continue;
							cGatTile* c = &cGraphics::world->gattiles[y][x];
							c->type = cGraphics::gatTiles[cGraphics::texturestart];
						}
					}
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		{
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				int posx = (int)mouse3dx / 5;
				int posy = (int)mouse3dz / 5;

				int f = (int)ceil(cGraphics::brushsize);

			//	if (posx >= floor(f/2.0f) && posx < 2*cGraphics::world->width-(int)ceil(f/2.0f) && posy >= floor(f/2.0f) && posy< 2*cGraphics::world->height-(int)ceil(f/2.0f))
				{
					undostack.push(new cUndoGatTileEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));

					glColor4f(1,0,0,1);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
					for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
					{
						for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
						{
							if (y < 0 || y >= cGraphics::world->height*2 || x < 0 || x >= cGraphics::world->width*2)
								continue;
							cGatTile* c = &cGraphics::world->gattiles[y][x];
							c->type = cGraphics::gatTiles[cGraphics::texturestart];
						}
					}
				}
			}
			break;
		}
		case SDL_KEYDOWN:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_EQUALS:
				cGraphics::brushsize*=2;
				break;
			case SDLK_MINUS:
				if (cGraphics::brushsize > 1)
					cGraphics::brushsize/=2;
				break;
			case SDLK_SPACE:
				{
					int posx = (int)mouse3dx / 5;
					int posy = (int)mouse3dz / 5;

					float f = (int)ceil(cGraphics::brushsize);


					if (posx >= (int)floor(f/2.0f) && posx < 2*cGraphics::world->width-(int)ceil(f/2.0f) && posy >= (int)floor(f/2.0f) && posy< 2*cGraphics::world->height-(int)ceil(f/2.0f))
					{
						undostack.push(new cUndoGatHeightEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));
						for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
						{
							for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
							{
								cCube* c = &cGraphics::world->cubes[y/2][x/2];

								if (y%2 == 0 && x%2 == 0)
								{
									cGraphics::world->gattiles[y][x].cell1 = (c->cell1+c->cell1) / 2.0f;
									cGraphics::world->gattiles[y][x].cell2 = (c->cell1+c->cell2) / 2.0f;
									cGraphics::world->gattiles[y][x].cell3 = (c->cell1+c->cell3) / 2.0f;
									cGraphics::world->gattiles[y][x].cell4 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
								}
								if (y%2 == 0 && x%2 == 1)
								{
									cGraphics::world->gattiles[y][x].cell1 = (c->cell1+c->cell2) / 2.0f;
									cGraphics::world->gattiles[y][x].cell2 = (c->cell2+c->cell2) / 2.0f;
									cGraphics::world->gattiles[y][x].cell3 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
									cGraphics::world->gattiles[y][x].cell4 = (c->cell4+c->cell2) / 2.0f;
								}
								if (y%2 == 1 && x%2 == 1)
								{
									cGraphics::world->gattiles[y][x].cell1 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
									cGraphics::world->gattiles[y][x].cell2 = (c->cell4 + c->cell2) / 2.0f;
									cGraphics::world->gattiles[y][x].cell3 = (c->cell4 + c->cell3) / 2.0f;
									cGraphics::world->gattiles[y][x].cell4 = (c->cell4 + c->cell4) / 2.0f;
								}
								if (y%2 == 1 && x%2 == 0)
								{
									cGraphics::world->gattiles[y][x].cell1 = (c->cell3 + c->cell1) / 2.0f;
									cGraphics::world->gattiles[y][x].cell2 = (c->cell1+c->cell4+c->cell2+c->cell3) / 4.0f;
									cGraphics::world->gattiles[y][x].cell3 = (c->cell3 + c->cell3) / 2.0f;
									cGraphics::world->gattiles[y][x].cell4 = (c->cell3 + c->cell4) / 2.0f;
								}
							
							}
						}
					}
				}
				break;
			case SDLK_PAGEDOWN:
				{
					int posx = (int)mouse3dx / 5;
					int posy = (int)mouse3dz / 5;

					float f = ceil(cGraphics::brushsize);


					if (posx >= (int)floor(f/2.0f) && posx < 2*cGraphics::world->width-(int)ceil(f/2.0f) && posy >= (int)floor(f/2.0f) && posy< 2*cGraphics::world->height-(int)ceil(f/2.0f))
					{
						undostack.push(new cUndoGatHeightEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));
						for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
						{
							for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
							{
								cGatTile* c = &cGraphics::world->gattiles[y][x];
								if (!cGraphics::slope || ((x > posx-(int)floor(f/2.0f)) && y > posy-(int)floor(f/2.0f)))
									c->cell1+=1;
								if (!cGraphics::slope || ((x < posx+(int)ceil(f/2.0f)-1) && y > posy-(int)floor(f/2.0f)))
									c->cell2+=1;
								if (!cGraphics::slope || ((x > posx-(int)floor(f/2.0f)) && y < posy+(int)ceil(f/2.0f)-1))
									c->cell3+=1;
								if (!cGraphics::slope || ((x < posx+(int)ceil(f/2.0f)-1) && y < posy+(int)ceil(f/2.0f)-1))
									c->cell4+=1;
							}
						}
					}
				}
				break;
			case SDLK_PAGEUP:
				{
					int posx = (int)mouse3dx / 5;
					int posy = (int)mouse3dz / 5;

					float f = (int)ceil(cGraphics::brushsize);

					undostack.push(new cUndoGatHeightEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));
					glColor4f(1,0,0,1);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
					for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
					{
						for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
						{
							if(x >= 0 && y >= 0 && y < (int)cGraphics::world->gattiles.size() && x < (int)cGraphics::world->gattiles[0].size())
							{
								cGatTile* c = &cGraphics::world->gattiles[y][x];
								if (!cGraphics::slope || ((x > posx-(int)floor(f/2.0f)) && y > posy-(int)floor(f/2.0f)))
									c->cell1-=1;
								if (!cGraphics::slope || ((x < posx+(int)ceil(f/2.0f)-1) && y > posy-(int)floor(f/2.0f)))
									c->cell2-=1;
								if (!cGraphics::slope || ((x > posx-(int)floor(f/2.0f)) && y < posy+(int)ceil(f/2.0f)-1))
									c->cell3-=1;
								if (!cGraphics::slope || ((x < posx+(int)ceil(f/2.0f)-1) && y < posy+(int)ceil(f/2.0f)-1))
									c->cell4-=1;
							}
						}
					}
				}
				break;
			case SDLK_c:
				{
					int posx = (int)mouse3dx / 5;
					int posy = (int)mouse3dz / 5;

					int f = (int)ceil(cGraphics::brushsize);

					if (posx >= (int)floor(f/2.0f) && posx < 2*cGraphics::world->width-(int)ceil(f/2.0f) && posy >= f && posy< 2*cGraphics::world->height-f)
					{
						clipboard.clear();
						clipboardgat.clear();
						for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
						{
							std::vector<std::vector<float> > row;
							std::vector<int > row2;
							for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
							{
								std::vector<float> c;
								c.push_back(cGraphics::world->gattiles[y][x].cell1);
								c.push_back(cGraphics::world->gattiles[y][x].cell2);
								c.push_back(cGraphics::world->gattiles[y][x].cell3);
								c.push_back(cGraphics::world->gattiles[y][x].cell4);
								row.push_back(c);
								row2.push_back(cGraphics::world->gattiles[y][x].type);
							}
							clipboard.push_back(row);
							clipboardgat.push_back(row2);
						}
					}
				}
				break;
			case SDLK_p:
				{
					int posx = (int)mouse3dx / 5;
					int posy = (int)mouse3dz / 5;

					int f = (int)ceil(cGraphics::brushsize);

					if ((int)clipboard.size() != f)
						break;

					undostack.push(new cUndoGatHeightEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));
					undostack.push(new cUndoGatTileEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));

					if (posx >= (int)floor(f/2.0f) && posx < 2*cGraphics::world->width-(int)ceil(f/2.0f) && posy >= f && posy< 2*cGraphics::world->height-f)
					{
						int yy = 0;
						for(int y = posy-(int)floor(f/2.0f); y < posy+(int)ceil(f/2.0f); y++)
						{
							int xx = 0;
							for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f); x++)
							{
								cGraphics::world->gattiles[y][x].cell1 = clipboard[yy][xx][0];
								cGraphics::world->gattiles[y][x].cell2 = clipboard[yy][xx][1];
								cGraphics::world->gattiles[y][x].cell3 = clipboard[yy][xx][2];
								cGraphics::world->gattiles[y][x].cell4 = clipboard[yy][xx][3];
								cGraphics::world->gattiles[y][x].type = clipboardgat[yy][xx];
								xx++;
							}
							yy++;
						}
					}
				}
				break;
			case SDLK_f:
				{
					int posx = (int)mouse3dx / 5;
					int posy = (int)mouse3dz / 5;

					float f = (int)ceil(cGraphics::brushsize);


					undostack.push(new cUndoGatHeightEdit(posx-(int)floor(f/2.0f), posy-(int)floor(f/2.0f), posx+(int)ceil(f/2.0f), posy+(int)ceil(f/2.0f)));
					for(int x = posx-(int)floor(f/2.0f); x < posx+(int)ceil(f/2.0f)-1; x++)
					{
						for(int y = posy-(int)floor(f/2.0f)+1; y < posy+(int)ceil(f/2.0f); y++)
						{
							if (x >= 0 && x < cGraphics::world->width*2 && y > 0 && y <= cGraphics::world->height*2)
							{
								float to = cGraphics::world->gattiles[y][x].cell2;
								cGraphics::world->gattiles[y][x].cell2 = to;
								cGraphics::world->gattiles[y][x+1].cell1 = to;
								cGraphics::world->gattiles[y-1][x+1].cell3 = to;
								cGraphics::world->gattiles[y-1][x].cell4 = to;
							}
						}
					}

				}
				break;
			case SDLK_LEFTBRACKET:
				cGraphics::texturestart--;
				if (cGraphics::texturestart < 0)
					cGraphics::texturestart = 0;
				break;
			case SDLK_RIGHTBRACKET:
				cGraphics::texturestart++;
				if (cGraphics::texturestart > 8)
					cGraphics::texturestart--;
				break;
			case SDLK_i:
				{
				int posx = (int)mouse3dx / 5;
				int posy = (int)mouse3dz / 5;
				char buf[100];
				sprintf(buf, "Your cursor is at %i,%i", posx, posy);
				if(cWM::getWindow(WT_MESSAGE))
					cWM::getWindow(WT_MESSAGE)->close();
				cWM::showMessage(buf);
				break;
				}



			default:
				break;
			}

			break;
		}
		
			
	}



	return true;


}
