#pragma once
namespace Sdl{
    template <typename TEvent, Uint32 type> TEvent& convertEvent(SDL_Event &e);

    class GameLoop{
        bool _stop = false;
        std::map<Uint32, std::function<void(SDL_Event&)>> _handlers;
        std::vector<Uint32> _indices;
        std::vector<std::function<void(SDL_Event&)>> _hs;

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
            /*_handlers.emplace(type, [=](SDL_Event &e){
                f(convertEvent<TEvent, type>(e));
            });*/
            _indices.push_back(type);
            _hs.push_back([=](SDL_Event &e){
                f(convertEvent<TEvent, type>(e));
            });
            /*_handlers.push_back(std::make_tuple(type, [=](SDL_Event &e){
                f(convertEvent<TEvent, type>(e));
            }));*/
        }
        template <typename TFunc> void Run(TFunc render){
            while(true){
                if (_stop) break;
                render();
           		SDL_Event e;
           		if (SDL_PollEvent(&e)){
                    /*auto h = _handlers.find(e.type);
                    if (h != _handlers.end()){
                        std::get<1>(*h)(e);
                    }*/
                    for(auto i = 0; i < _indices.size(); i++){
                        if (_indices[i] == e.type){
                            _hs[i](e);
                        }
                    }
                    /*for(auto &h : _handlers){
                        if (std::get<0>(h) == e.type){
                            std::get<1>(h)(e);
                        }
                    }*/
           		}
            }
        }

        void Stop(){
            _stop = true;
        }
    };
	template<> SDL_CommonEvent& convertEvent<SDL_CommonEvent, SDL_WINDOWEVENT>(SDL_Event& e) { return reinterpret_cast<SDL_CommonEvent&>(e); }
	template<> SDL_KeyboardEvent& convertEvent<SDL_KeyboardEvent, SDL_KEYDOWN>(SDL_Event& e) { return reinterpret_cast<SDL_KeyboardEvent&>(e); }
	template<> SDL_KeyboardEvent& convertEvent<SDL_KeyboardEvent, SDL_KEYUP>(SDL_Event& e) { return reinterpret_cast<SDL_KeyboardEvent&>(e); }
	template<> SDL_TextEditingEvent& convertEvent<SDL_TextEditingEvent, SDL_TEXTEDITING>(SDL_Event& e) { return reinterpret_cast<SDL_TextEditingEvent&>(e); }
	template<> SDL_TextInputEvent& convertEvent<SDL_TextInputEvent, SDL_TEXTINPUT>(SDL_Event& e) { return reinterpret_cast<SDL_TextInputEvent&>(e); }
	template<> SDL_MouseMotionEvent& convertEvent<SDL_MouseMotionEvent, SDL_MOUSEMOTION>(SDL_Event& e) { return reinterpret_cast<SDL_MouseMotionEvent&>(e); }
	template<> SDL_MouseButtonEvent& convertEvent<SDL_MouseButtonEvent, SDL_MOUSEBUTTONDOWN>(SDL_Event& e) { return reinterpret_cast<SDL_MouseButtonEvent&>(e); }
	template<> SDL_MouseButtonEvent& convertEvent<SDL_MouseButtonEvent, SDL_MOUSEBUTTONUP>(SDL_Event& e) { return reinterpret_cast<SDL_MouseButtonEvent&>(e); }
	template<> SDL_MouseWheelEvent& convertEvent<SDL_MouseWheelEvent, SDL_MOUSEWHEEL>(SDL_Event& e) { return reinterpret_cast<SDL_MouseWheelEvent&>(e); }
	template<> SDL_JoyAxisEvent& convertEvent<SDL_JoyAxisEvent, SDL_JOYAXISMOTION>(SDL_Event& e) { return reinterpret_cast<SDL_JoyAxisEvent&>(e); }
	template<> SDL_JoyBallEvent& convertEvent<SDL_JoyBallEvent, SDL_JOYBALLMOTION>(SDL_Event& e) { return reinterpret_cast<SDL_JoyBallEvent&>(e); }
	template<> SDL_JoyHatEvent& convertEvent<SDL_JoyHatEvent, SDL_JOYHATMOTION>(SDL_Event& e) { return reinterpret_cast<SDL_JoyHatEvent&>(e); }
	template<> SDL_JoyButtonEvent& convertEvent<SDL_JoyButtonEvent, SDL_JOYBUTTONDOWN>(SDL_Event& e) { return reinterpret_cast<SDL_JoyButtonEvent&>(e); }
	template<> SDL_JoyButtonEvent& convertEvent<SDL_JoyButtonEvent, SDL_JOYBUTTONUP>(SDL_Event& e) { return reinterpret_cast<SDL_JoyButtonEvent&>(e); }
	template<> SDL_JoyDeviceEvent& convertEvent<SDL_JoyDeviceEvent, SDL_JOYDEVICEADDED>(SDL_Event& e) { return reinterpret_cast<SDL_JoyDeviceEvent&>(e); }
	template<> SDL_JoyDeviceEvent& convertEvent<SDL_JoyDeviceEvent, SDL_JOYDEVICEREMOVED>(SDL_Event& e) { return reinterpret_cast<SDL_JoyDeviceEvent&>(e); }
	template<> SDL_ControllerAxisEvent& convertEvent<SDL_ControllerAxisEvent, SDL_CONTROLLERAXISMOTION>(SDL_Event& e) { return reinterpret_cast<SDL_ControllerAxisEvent&>(e); }
	template<> SDL_ControllerButtonEvent& convertEvent<SDL_ControllerButtonEvent, SDL_CONTROLLERBUTTONDOWN>(SDL_Event& e) { return reinterpret_cast<SDL_ControllerButtonEvent&>(e); }
	template<> SDL_ControllerButtonEvent& convertEvent<SDL_ControllerButtonEvent, SDL_CONTROLLERBUTTONUP>(SDL_Event& e) { return reinterpret_cast<SDL_ControllerButtonEvent&>(e); }
	template<> SDL_ControllerDeviceEvent& convertEvent<SDL_ControllerDeviceEvent, SDL_CONTROLLERDEVICEADDED>(SDL_Event& e) { return reinterpret_cast<SDL_ControllerDeviceEvent&>(e); }
	template<> SDL_ControllerDeviceEvent& convertEvent<SDL_ControllerDeviceEvent, SDL_CONTROLLERDEVICEREMOVED>(SDL_Event& e) { return reinterpret_cast<SDL_ControllerDeviceEvent&>(e); }
	template<> SDL_ControllerDeviceEvent& convertEvent<SDL_ControllerDeviceEvent, SDL_CONTROLLERDEVICEREMAPPED>(SDL_Event& e) { return reinterpret_cast<SDL_ControllerDeviceEvent&>(e); }
	template<> SDL_TouchFingerEvent& convertEvent<SDL_TouchFingerEvent, SDL_FINGERMOTION>(SDL_Event& e) { return reinterpret_cast<SDL_TouchFingerEvent&>(e); }
	template<> SDL_TouchFingerEvent& convertEvent<SDL_TouchFingerEvent, SDL_FINGERDOWN>(SDL_Event& e) { return reinterpret_cast<SDL_TouchFingerEvent&>(e); }
	template<> SDL_TouchFingerEvent& convertEvent<SDL_TouchFingerEvent, SDL_FINGERUP>(SDL_Event& e) { return reinterpret_cast<SDL_TouchFingerEvent&>(e); }
	template<> SDL_MultiGestureEvent& convertEvent<SDL_MultiGestureEvent, SDL_MULTIGESTURE>(SDL_Event& e) { return reinterpret_cast<SDL_MultiGestureEvent&>(e); }
	template<> SDL_DollarGestureEvent& convertEvent<SDL_DollarGestureEvent, SDL_DOLLARGESTURE>(SDL_Event& e) { return reinterpret_cast<SDL_DollarGestureEvent&>(e); }
	template<> SDL_DropEvent& convertEvent<SDL_DropEvent, SDL_DROPFILE>(SDL_Event& e) { return reinterpret_cast<SDL_DropEvent&>(e); }
	template<> SDL_QuitEvent& convertEvent<SDL_QuitEvent, SDL_QUIT>(SDL_Event& e) { return reinterpret_cast<SDL_QuitEvent&>(e); }
	template<> SDL_OSEvent& convertEvent<SDL_OSEvent, SDL_QUIT>(SDL_Event& e) { return reinterpret_cast<SDL_OSEvent&>(e); }
	template<> SDL_UserEvent& convertEvent<SDL_UserEvent, SDL_USEREVENT>(SDL_Event& e) { return reinterpret_cast<SDL_UserEvent&>(e); }
	template<> SDL_UserEvent& convertEvent<SDL_UserEvent, SDL_LASTEVENT>(SDL_Event& e) { return reinterpret_cast<SDL_UserEvent&>(e); }
	template<> SDL_SysWMEvent& convertEvent<SDL_SysWMEvent, SDL_SYSWMEVENT>(SDL_Event& e) { return reinterpret_cast<SDL_SysWMEvent&>(e); }
}
