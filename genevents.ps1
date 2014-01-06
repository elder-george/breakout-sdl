$IDLE = 1;
$WAIT_LBR = 2;
$WAIT_TYPE = 3;
$READ_FIELDS = 4;

$mode = $IDLE;
$current_event = "";
$types = @();
$arguments = @();

@'
#pragma once
namespace Sdl{
    template <typename TEvent, Uint32 type> TEvent& convertEvent(SDL_Event &e);

    class GameLoop{
        bool _stop = false;
        std::map<Uint32, std::function<void(SDL_Event&)> > _handlers;

    template <typename T> struct handler_traits: 
       public handler_traits<decltype(&T::operator())>
    { };

    template <typename ClassT, typename ReturnT, typename EventT> 
    struct handler_traits<ReturnT(ClassT::*)(EventT&) const>{
        typedef EventT event_type;
    };
    public:
       
        template <Uint32 type, typename TFunc> void On(TFunc f){
            typedef typename handler_traits<TFunc>::event_type TEvent;

            _handlers.emplace(type, [=](SDL_Event &e){
                f(convertEvent<TEvent, type>(e));
            });
        }
        template <typename TFunc> void Run(TFunc render){
            while(true){
                if (_stop) break;
                render();
           		SDL_Event e;
           		if (SDL_PollEvent(&e)){
                    auto h = _handlers.find(e.type);
                    if (h != _handlers.end()){
                        std::get<1>(*h)(e);
                    }
           		}
            }
        }

        void Stop(){
            _stop = true;
        }
    };
'@;


cat "C:\tools\gcc\include\SDL2\sdl_events.h" |% {
	$ln = $_;
	switch ($mode) {
		$IDLE { if ($ln -imatch "typedef struct (SDL_.*Event)") {
					$current_event = $matches[1];
					$mode = $WAIT_LBR;
				} 
			}
		$WAIT_LBR {
				if ($ln -match "\{"){
					$mode = $WAIT_TYPE;
				}
			}
	    $WAIT_TYPE {
	    	if ($ln -match "Uint32 type;(?:\s*\/\*\*\<\s(.*)\*\/)"){
	    		$mode = $READ_FIELDS;
	    		$ms = [regex]::matches($matches[1], "(?:\:\:)([\w\d_]+)", 'ExplicitCapture')
	   			for ($i = 0; $i -lt $ms.Count; $i++){
	    			$types += $ms[$i].Value.split(":")[2];
	    		}
	    	}
	    }
	    $READ_FIELDS {
	    	if ($ln -match "([\w\d]+)\s+([\w\d]+);"){
	    		if (-not ($matches[2] -match "padding.*" -or $matches[2] -match "timestamp.*")){
		    		$arguments += ("$($matches[1]) $($matches[2])");
		    	}
	    	}elseif ($ln -match "\}"){
                foreach($type in $types){
                    Write-Host "	template<> $current_event& convertEvent<$current_event, $type>(SDL_Event& e) { return reinterpret_cast<$current_event&>(e); }";
                }
	    		$current_event = "";
	    		$arguments = @()
	    		$types = @();
	    		$mode = $IDLE;

	    	}
	    }
	}
}

"}"