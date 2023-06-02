// *****************************************************************************
    // start include guard
    #ifndef SETTINGS_HPP
    #define SETTINGS_HPP
    
    // include project headers
    #include "VirconGamepadController.hpp"
    
    // include C/C++ headers
    #include <string>               // [ C++ STL ] Strings
    #include <map>                  // [ C++ STL ] Maps
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR INPUT MAPPINGS
// =============================================================================


// identification of a single control from a given joystick
class JoystickControl
{
    public:
        
        // control type
        bool IsAxis;
        bool IsHat;
        
        // button info
        int ButtonIndex;
        
        // axis info
        int AxisIndex;
        bool AxisPositive;
        
        // hat info
        int HatIndex;
        int HatDirection;
    
    public:
        
        // constructor to leave all controls unmapped
        JoystickControl();
};

// -----------------------------------------------------------------------------

// control mapping for a joystick
class JoystickMapping
{
    public:
        
        // static identification
        SDL_JoystickGUID GUID;
        
        // human-readable names
        std::string ProfileName;
        std::string JoystickName;
        
        // d-pad directions
        JoystickControl Left, Right, Up, Down;
        
        // buttons
        JoystickControl ButtonA, ButtonB, ButtonX, ButtonY;
        JoystickControl ButtonL, ButtonR, ButtonStart;    
};


// =============================================================================
//      OPERATION WITH GUIDS
// =============================================================================


// GUID <-> string conversions
std::string GUIDToString( SDL_JoystickGUID GUID );
bool GUIDStringIsValid( const std::string& GUIDString );


// =============================================================================
//      GLOBALS FOR INPUT DEVICES
// =============================================================================


// SDL joystick instances --> SDL joystick
extern std::map< Sint32, SDL_Joystick* > ConnectedJoysticks;

// a single mapping, applicable for all V32 gamepads
// (since they are all considered identical)
extern JoystickMapping Vircon32GamepadMapping;

// maps each of the 4 gamepad ports to an SDL joystick instance
// (an invalid instance of -1 means no gamepad is connected)
extern Sint32 GamepadInstanceIDs[ Constants::MaximumGamepads ];

// stores the expected path string for each of the 4 gamepad ports
extern std::string GamepadPaths[ Constants::MaximumGamepads ];


// =============================================================================
//      LOAD & SAVE INPUT DEVICES FROM XML FILE
// =============================================================================


void SetDefaultControls();
void LoadControls( const std::string& FilePath );


// =============================================================================
//      LOAD & SAVE SETTINGS FROM XML FILES
// =============================================================================


void SetDefaultSettings();
void LoadSettings( const std::string& FilePath );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

