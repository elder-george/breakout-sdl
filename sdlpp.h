#pragma once
namespace Sdl{
	class sdl_error: public std::runtime_error{
	public:
	    sdl_error(): 
	        std::runtime_error(SDL_GetError())
	    {        
	    }    
	};

	class Sdl{
	public:
    	Sdl(){
	        if (SDL_Init(SDL_INIT_EVERYTHING) == -1){
    	        throw sdl_error();
	        }
            SDL_GameControllerEventState(SDL_ENABLE);
    	}

	    ~Sdl(){
    		SDL_Quit();
	    }
	};

	class SdlWindow{
	    SDL_Window *win;
	public:
	    SdlWindow(const char* title, int x, int y, int w, int h){
		  win = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
		  if (!win){
		    throw sdl_error();
		  }
	    }

	    ~SdlWindow(){
		  if (win != nullptr){
			SDL_DestroyWindow(win);
			win = nullptr;
		  }
	    }

	    friend class SdlRenderer;
	};

	class SdlTexture{
		SDL_Texture *tex;
        SDL_Surface *surface;
	public:
		SdlTexture(SDL_Texture *tex, SDL_Surface *surface): tex(tex), surface(surface){
			if (!tex || !surface){
				throw sdl_error();
			}		
		}

		void Query(unsigned int &format, int &access, int &w, int &h){
			if (SDL_QueryTexture(tex, &format, &access, &w, &h) < 0){
				throw sdl_error();
			}
		}

		SDL_Point GetSize(){
			SDL_Point result;
			unsigned int format; int access;
			Query(format, access, result.x, result.y);
			return result;
		}

        void Update(){
            SDL_UpdateTexture(tex, NULL, surface->pixels, surface->pitch);
        }

		~SdlTexture(){
			if (tex != nullptr){
				SDL_DestroyTexture(this->tex);
				this->tex = nullptr;
			}
		}

		friend class SdlRenderer;
	};

	class SdlRenderer{
	    SDL_Renderer *ren;
	public:
	    SdlRenderer(const SdlWindow &win){
		ren = SDL_CreateRenderer(win.win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
		if (!ren){
		    throw sdl_error();
		}
	    }

	    ~SdlRenderer(){
		  if (ren != nullptr){
			SDL_DestroyRenderer(ren);
			ren = nullptr;
		  }
	    }

	    void Clear(){
		  SDL_RenderClear(ren);
	    }

	    void Copy(const SdlTexture& texture, const SDL_Rect *src, const SDL_Rect *dst){
		  SDL_RenderCopy(ren, texture.tex, src, dst);
	    }

	    void Present(){
		  SDL_RenderPresent(ren);
	    }

	    friend class SdlSurface;
	};


	class SdlSurface{
	    SDL_Surface *surface;
	protected:
	    SdlSurface(SDL_Surface *surface){
		  this->surface = surface;
		  if (!surface){
		    throw sdl_error();
		  }
	    }

	public:
	    static SdlSurface LoadBMP(const char * path){
		return SdlSurface(SDL_LoadBMP(path));
	    }

	    static SdlSurface Load(const char * path){
		return SdlSurface(IMG_Load(path));
	    }
	    
	    ~SdlSurface(){
		if (this->surface != nullptr){
			SDL_FreeSurface(this->surface);
		    this->surface = nullptr;
		}
	    }

	    SdlTexture SurfaceToTexture(const SdlRenderer& ren){
		return SdlTexture(SDL_CreateTextureFromSurface(ren.ren, this->surface), this->surface);
	    }


	    friend class SdlRenderer;
	    friend class SdlTexture;
        friend class SdlTtfFont;
	};

    class SdlTtf{
    public:
        SdlTtf(){
            TTF_Init();
        }

        ~SdlTtf(){
            TTF_Quit();
        }
    };

    #define PROP(type, name) \
        void Set##name(type value) { TTF_SetFont##name(_font, value); } \
        type Get##name() { return TTF_GetFont##name(_font); }
    class SdlTtfFont{
        TTF_Font *_font;

    public:
        SdlTtfFont(const char *path, int ptSize){
            _font = TTF_OpenFont(path, ptSize);
        }

        PROP(int, Style)
        PROP(int, Outline)
        PROP(int, Hinting)

        SdlSurface RenderTextSolid(char *text, SDL_Color fg){
            auto surface = TTF_RenderText_Solid(_font, text, fg);
            return SdlSurface(surface);
        }

        ~SdlTtfFont(){
            TTF_CloseFont(_font);
        }
    };
    #undef PROP

    class SdlGameController{
        SDL_GameController* _ctrl;
    public:

        SdlGameController(int idx){ 
            _ctrl = SDL_GameControllerOpen(idx);
        }

        bool IsAttached(){
            return SDL_GameControllerGetAttached(_ctrl);
        }

        virtual ~SdlGameController(){
            SDL_GameControllerClose(_ctrl);
        }

        static const char* NameForIndex(int idx){
            return SDL_GameControllerNameForIndex(idx);
        }
    };
}
