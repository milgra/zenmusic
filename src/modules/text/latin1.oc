#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_ttf.h"

#define RENDER_MODE 1 //0=solid 1=shaded 2=blended

SDL_Surface *screen=0,
			*text[128];

TTF_Font *font=0;

int start_glyph=0,
	style=TTF_STYLE_NORMAL,
	kerning=1,
	hinting=TTF_HINTING_NORMAL,
	outline=0,
	font_size=32;

typedef struct {
	int minx,
		maxx,
		miny,
		maxy,
		advance;
} GlyphMetrics;

GlyphMetrics gm[128];

#define SPACING 0

void draw_table(int x0, int y0)
{
	int i,w=0,h;
	SDL_Rect r, r2;
	Uint32 color[6];

	SDL_Color fg={0,0,0,255};
#if RENDER_MODE==1
	SDL_Color bg={255,255,255,255};
#endif
	color[0]=SDL_MapRGB(screen->format,255,0,0);
	color[1]=SDL_MapRGB(screen->format,255,128,0);
	color[2]=SDL_MapRGB(screen->format,255,255,0);
	color[3]=SDL_MapRGB(screen->format,0,255,0);
	color[4]=SDL_MapRGB(screen->format,0,255,255);
	color[5]=SDL_MapRGB(screen->format,0,0,255);
	color[6]=SDL_MapRGB(screen->format,128,0,128);
	
	/* detemine max font advance for this page */
	for(i=0;i<128;i++)
		if(w<gm[i].advance)
			w=gm[i].advance;
	
	h=TTF_FontLineSkip(font);
	
	for(i=0; i<17; i++)
	{
		/* draw max font advance grid (for this page) */
		r.x=x0+i*(w+SPACING)-SPACING/2-1;
		r.y=y0-SPACING/2-1;
		r.w=1;
		r.h=(h+SPACING)*8+1;
		SDL_FillRect(screen,&r,color[5]);
	}
	
	for(i=0; i<9; i++)
	{
		if(i<8)
		{
			/* draw font ascent line */
			r.x=x0-SPACING/2-1;
			r.y=y0+i*(h+SPACING)-SPACING/2+TTF_FontAscent(font);
			r.w=(w+SPACING)*16;
			r.h=1;
			SDL_FillRect(screen,&r,color[3]);
			/* draw font height line */
			r.x=x0-SPACING/2-1;
			r.y=y0+i*(h+SPACING)-SPACING/2+TTF_FontHeight(font);
			r.w=(w+SPACING)*16;
			r.h=1;
			SDL_FillRect(screen,&r,color[2]);
		}
		/* draw max height grid */
		r.x=x0-SPACING/2-1;
		r.y=y0+i*(h+SPACING)-SPACING/2-1;
		r.w=(w+SPACING)*16;
		r.h=1;
		SDL_FillRect(screen,&r,color[5]);
	}

	r.x=x0;
	r.y=y0-h;
	for(i=0;i<128;i++)
	{
		/* relocate for next line... */
		if(!(i%16))
		{
			r.x=x0;
			r.y+=h+SPACING;
		}

		/* draw the bounding box */
#if RENDER_MODE==1
		r2.x=r.x+gm[i].minx-1;
		r2.y=r.y+TTF_FontAscent(font)-gm[i].maxy-1;
		r2.w=text[i]->w+2;
		r2.h=text[i]->h+2;
		SDL_FillRect(screen,&r2,color[4]);
#else
		r2.x=r.x+gm[i].minx-1;
		r2.y=r.y+TTF_FontAscent(font)-gm[i].maxy-1;
		r2.w=text[i]->w+2;
		r2.h=1;
		SDL_FillRect(screen,&r2,color[4]);
		r2.x=r.x+gm[i].minx-1;
		r2.y=r.y+TTF_FontAscent(font)-gm[i].maxy-1;
		r2.w=1;
		r2.h=text[i]->h+2;
		SDL_FillRect(screen,&r2,color[4]);
		r2.x=r.x+gm[i].minx-1;
		r2.y=r.y+TTF_FontAscent(font)-gm[i].maxy+text[i]->h;
		r2.w=text[i]->w+2;
		r2.h=1;
		SDL_FillRect(screen,&r2,color[4]);
		r2.x=r.x+gm[i].minx+text[i]->w;
		r2.y=r.y+TTF_FontAscent(font)-gm[i].maxy-1;
		r2.w=1;
		r2.h=text[i]->h+2;
		SDL_FillRect(screen,&r2,color[4]);
#endif

		/* draw advance line */
		r2.x=r.x+gm[i].advance;
		r2.w=1;
		r2.y=r.y;
		r2.h=h;
		SDL_FillRect(screen,&r2,color[0]);

		/* draw the glyph (from cache) */
		r2.x=r.x+gm[i].minx-TTF_GetFontOutline(font);
		r2.y=r.y+TTF_FontAscent(font)-gm[i].maxy-TTF_GetFontOutline(font);
		SDL_BlitSurface(text[i],0,screen,&r2);

		r.x+=w+SPACING;
	}
	r.x=x0-TTF_GetFontOutline(font);
	r.y+=h+SPACING-TTF_GetFontOutline(font);
	{
		/* pick a panagram... */
		const char *msg="Toth Milan vagyok, viragokat locsolgatok, megbaszhatod+!!!";
			/*"The quick brown fox jumps over the lazy dog";*/
		SDL_Surface *surf=
#if RENDER_MODE==0
				TTF_RenderText_Solid(font,msg,fg);
#elif RENDER_MODE==1
				TTF_RenderText_Shaded(font,msg,fg,bg);
#elif RENDER_MODE==2
				TTF_RenderText_Blended(font,msg,fg);
#endif
		if(surf)
		{
			SDL_BlitSurface(surf,0,screen,&r);
			SDL_FreeSurface(surf);
		}
	}
}

void free_glyphs()
{
	int i;

	for(i=0; i<128; i++)
	{
		if(text[i])
			SDL_FreeSurface(text[i]);
		text[i]=0;
	}
}

void free_font()
{
	if(font)
		TTF_CloseFont(font);
	font=0;
	free_glyphs();
}

void cache_glyphs()
{
	int i;
	char title[800];
	SDL_Color fg={0,0,0,255};
#if RENDER_MODE==1
	SDL_Color bg={255,255,255,255};
#endif

	free_glyphs();
	if(!font)
		return;
	if(style!=TTF_GetFontStyle(font))
		TTF_SetFontStyle(font,style);		
	TTF_SetFontKerning(font,kerning);
	if(hinting != TTF_GetFontHinting(font))
		TTF_SetFontHinting(font,hinting);
	if(outline != TTF_GetFontOutline(font))
		TTF_SetFontOutline(font,outline);
	for(i=0; i<128; i++)
	{
		/* cache rendered surface */
#if RENDER_MODE==0
		text[i]=TTF_RenderGlyph_Solid(font,i+start_glyph,fg);
#elif RENDER_MODE==1
		text[i]=TTF_RenderGlyph_Shaded(font,i+start_glyph,fg,bg);
#elif RENDER_MODE==2
		text[i]=TTF_RenderGlyph_Blended(font,i+start_glyph,fg);
#endif
		if(!text[i])
		{
			printf("TTF_RenderGlyph_Shaded: %s\n", TTF_GetError());
			exit(4);
		}
		/* cache metrics */
		TTF_GlyphMetrics(font, i+start_glyph,
				&gm[i].minx, &gm[i].maxx,
				&gm[i].miny, &gm[i].maxy,
				&gm[i].advance);
	}

	sprintf(title,"%s-%s:%d+0x%04x",TTF_FontFaceFamilyName(font),
			TTF_FontFaceStyleName(font),font_size,start_glyph);
	SDL_WM_SetCaption(title,"latin1");
}

void load_font(char *fname, int size)
{
	char *p;

	free_font();
	font=TTF_OpenFont(fname, size);
	if(!font)
	{
		printf("TTF_OpenFont: %s\n", TTF_GetError());
		exit(3);
	}

	/* print some metrics and attributes */
	printf("size                    : %d\n",size);
	printf("TTF_FontHeight          : %d\n",TTF_FontHeight(font));
	printf("TTF_FontAscent          : %d\n",TTF_FontAscent(font));
	printf("TTF_FontDescent         : %d\n",TTF_FontDescent(font));
	printf("TTF_FontLineSkip        : %d\n",TTF_FontLineSkip(font));
	printf("TTF_FontFaceIsFixedWidth: %d\n",TTF_FontFaceIsFixedWidth(font));
	{
		char *str=TTF_FontFaceFamilyName(font);
		if(!str)
			str="(null)";
		printf("TTF_FontFaceFamilyName  : \"%s\"\n",str);
	}
	{
		char *str=TTF_FontFaceStyleName(font);
		if(!str)
			str="(null)";
		printf("TTF_FontFaceStyleName   : \"%s\"\n",str);
	}
	if(TTF_GlyphIsProvided(font,'g'))
	{
		int minx, maxx, miny, maxy, advance;
		TTF_GlyphMetrics(font,'g', &minx, &maxx, &miny, &maxy, &advance);
		printf("TTF_GlyphMetrics('g'):\n\tminx=%d\n\tmaxx=%d\n\tminy=%d\n\tmaxy=%d\n\tadvance=%d\n",
				minx, maxx, miny, maxy, advance);
	}
	else
		printf("TTF_GlyphMetrics('g'): unavailable in font!\n");

	/* set window title and icon name, using filename and stuff */
	p=strrchr(fname,'/');
	if(!p)
		p=strrchr(fname,'\\');
	if(!p)
		p=strrchr(fname,':');
	if(!p)
		p=fname;
	else
		p++;
	
	/* cache new glyphs */
	cache_glyphs();
}

int main(int argc, char **argv)
{
	int done=0;

	/* check args */
	if(argc!=2)
	{
		fprintf(stderr,"%s file.ttf\n",argv[0]);
		return 1;
	}
	
	/* initialize the cache to NULL */
	memset(text,0,sizeof(text));
	
	/* start SDL video */
	if(SDL_Init(SDL_INIT_VIDEO)==-1)
	{
		printf("SDL_Init: %s\n",SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit); /* remember to quit SDL */

	/* open the screen */
	if(!(screen=SDL_SetVideoMode(800,600,0,0)))
	{
		printf("SDL_SetVideoMode: %s\n",SDL_GetError());
		return 1;
	}

	/* allow for key repeat (so the user can hold down a key...) */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	
	/* start SDL_ttf */
	if(TTF_Init()==-1)
	{
		printf("TTF_Init: %s\n", TTF_GetError());
		return 2;
	}
	atexit(TTF_Quit); /* remember to quit SDL_ttf */
	atexit(free_font); /* remember to free any loaded font and glyph cache */

	while(!done)
	{
		SDL_Event event;
		static int x=0, y=0, need_draw=1,last_size=0, last_start_glyph=-1;

		/* smartly load font and generate new glyph cache (font_size change) */
		if(last_size!=font_size)
		{
			if(font_size<1)
				font_size=1;
			load_font(argv[1], font_size);
			need_draw=1;
			last_size=font_size;
		}
		/* smartly generate new glyph cache (page change) */
		if(last_start_glyph!=start_glyph)
		{
			cache_glyphs();
			need_draw=1;
			last_start_glyph=start_glyph;
		}
		/* smartly redraw as needed */
		if(need_draw)
		{
			SDL_FillRect(screen,0,~0);
			draw_table(x,y);
			SDL_Flip(screen);
			need_draw=0;
		}
		/* wait for events and handle them */
		/* this waits for one, then handles all that are queued before finishing */
		if(SDL_WaitEvent(&event))
		do {
			switch(event.type)
			{
				case SDL_QUIT:
					done=1;
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case '0':
							start_glyph=0;
							break;
						case SDLK_LEFT:
							start_glyph=(start_glyph+0x10000-0x80)&0xffff;
							break;
						case SDLK_RIGHT:
							start_glyph=(start_glyph+0x80)&0xffff;
							break;
						case SDLK_UP:
							font_size++;
							break;
						case SDLK_DOWN:
							font_size--;
							break;
						case 'n':
							style=TTF_STYLE_NORMAL;
							last_start_glyph=-1;
							break;
						case 'b':
							style^=TTF_STYLE_BOLD;
							last_start_glyph=-1;
							break;
						case 'i':
							style^=TTF_STYLE_ITALIC;
							last_start_glyph=-1;
							break;
						case 'u':
							style^=TTF_STYLE_UNDERLINE;
							last_start_glyph=-1;
							break;
						case 's':
							style^=TTF_STYLE_STRIKETHROUGH;
							last_start_glyph=-1;
							break;
						case 'k':
							kerning=!kerning;
							printf("kerning=%d\n",kerning);
							last_start_glyph=-1;
							break;
						case 'h':
							hinting=(hinting+1)%4;
							printf("hinting=%s\n",
									hinting==0?"Normal":
									hinting==1?"Light":
									hinting==2?"Mono":
									hinting==3?"None":
									"Unknonwn");
							last_start_glyph=-1;
							break;
						case '=':
							++outline;
							printf("outline=%d\n",outline);
							last_start_glyph=-1;
							break;
						case '-':
							if(outline>0)
								--outline;
							printf("outline=%d\n",outline);
							last_start_glyph=-1;
							break;
						case 'q':
						case SDLK_ESCAPE:
							done=1;
							break;
						default:
							break;
					}
					break;
				case SDL_MOUSEMOTION:
					if(event.motion.state)
					{
						x=event.motion.x;
						y=event.motion.y;
						need_draw=1;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					x=event.button.x;
					y=event.button.y;
					need_draw=1;
					break;
			}
		} while(SDL_PollEvent(&event));
	} /* main loop */
	
	return 0;
}
