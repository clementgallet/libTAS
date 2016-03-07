#include "events.h"
#include "logging.h"

/* Return if the SDL event must be passed to the game or be filtered */
int filterSDL2Event(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return 1;
        default:
            return 0;
    }
}

/* Return if the SDL event must be passed to the game or be filtered */
int filterSDL1Event(SDL1_Event *event)
{
    switch(event->type) {
        case SDL1_KEYDOWN:
        case SDL1_KEYUP:
            return 1;
        default:
            return 0;
    }
}

/* Print which event type is it */
void logEvent(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving KEYUP/KEYDOWN event.");
            break;

        case SDL_QUIT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving QUIT event.");
            break;

        case SDL_WINDOWEVENT:
            switch (event->window.event) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window %d gained keyboard focus.", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window %d lost keyboard focus.", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window %d closed.", event->window.windowID);
                    break;
                default:
                    break;
            }
            break;

        case SDL_SYSWMEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a system specific event.");
            switch (event->syswm.msg->subsystem) {
                case SDL_SYSWM_UNKNOWN:
                    debuglog(LCF_SDL | LCF_EVENTS, "Unknown subsystem.");
                    break;
                case SDL_SYSWM_WINDOWS:
                    debuglog(LCF_SDL | LCF_EVENTS, "Windows subsystem.");
                    break;
                case SDL_SYSWM_X11:
                    debuglog(LCF_SDL | LCF_EVENTS, "X subsystem.");
                    debuglog(LCF_SDL | LCF_EVENTS, "Getting an X event of type %d", event->syswm.msg->msg.x11.event.type);
                    break;
                case SDL_SYSWM_DIRECTFB:
                    debuglog(LCF_SDL | LCF_EVENTS, "DirectFB subsystem.");
                    break;
                case SDL_SYSWM_COCOA:
                    debuglog(LCF_SDL | LCF_EVENTS, "OSX subsystem.");
                    break;
                case SDL_SYSWM_UIKIT:
                    debuglog(LCF_SDL | LCF_EVENTS, "iOS subsystem.");
                    break;
                default:
                    debuglog(LCF_SDL | LCF_EVENTS, "Another subsystem.");
                    break;
            }
            break;

        case SDL_TEXTEDITING:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a keyboard text editing event.");
            break;

        case SDL_TEXTINPUT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a keyboard text input event.");
            break;
            /*
               case SDL_KEYMAPCHANGED:
               debuglog(LCF_SDL | LCF_EVENTS, "Receiving a keymap change event.");
               break;
               */
        case SDL_MOUSEMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse move event.");
            break;

        case SDL_MOUSEBUTTONDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse button press event.");
            break;

        case SDL_MOUSEBUTTONUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse button release event.");
            break;

        case SDL_MOUSEWHEEL:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse wheel event.");
            break;

        case SDL_JOYAXISMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick axis motion event.");
            break;

        case SDL_JOYBALLMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick trackball event.");
            break;

        case SDL_JOYHATMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick hat position event.");
            break;

        case SDL_JOYBUTTONDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick button press event.");
            break;

        case SDL_JOYBUTTONUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick button release event.");
            break;

        case SDL_JOYDEVICEADDED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick connected event.");
            break;

        case SDL_JOYDEVICEREMOVED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick disconnected event.");
            break;

        case SDL_CONTROLLERAXISMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller axis motion event.");
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller button press event.");
            break;

        case SDL_CONTROLLERBUTTONUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller button release event.");
            break;

        case SDL_CONTROLLERDEVICEADDED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller connected event.");
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller disconnected event.");
            break;

        case SDL_CONTROLLERDEVICEREMAPPED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller mapping update event.");
            break;

        case SDL_FINGERDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an input device touch event.");
            break;

        case SDL_FINGERUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an input device release event.");
            break;

        case SDL_FINGERMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an input device drag event.");
            break;

        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:
        case SDL_MULTIGESTURE:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a gesture event.");
            break;

        case SDL_CLIPBOARDUPDATE:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a clipboard update event.");
            break;

        case SDL_DROPFILE:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a drag and drop event.");
            break;
            /*
               case SDL_AUDIODEVICEADDED:
               debuglog(LCF_SDL | LCF_EVENTS, "Receiving a new audio device event.");
               break;

               case SDL_AUDIODEVICEREMOVED:
               debuglog(LCF_SDL | LCF_EVENTS, "Receiving a audio device removal event.");
               break;
               */
        case SDL_RENDER_TARGETS_RESET:
            //            case SDL_RENDER_DEVICE_RESET:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a render event.");
            break;

        case SDL_USEREVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a user-specified event.");
            break;

        default:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an unknown event: %d.", event->type);
            break;
    }



}
