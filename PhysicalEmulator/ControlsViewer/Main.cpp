// *****************************************************************************
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
    #include <SDL2/SDL_joystick.h>  // [ SDL2 ] Joystick functions
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <list>         // [ C++ STL ] Lists
    #include <map>          // [ C++ STL ] Maps
    
    // bug fix needed for SDL2 headers
    #undef main
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void LogJoystick( Sint32 InstanceID )
{
    SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
    cout << "Joystick instance " << InstanceID << endl;
    cout << " --> name = \"" << SDL_JoystickName( Joystick ) << endl;
    cout << " --> path = \"" << SDL_JoystickPath( Joystick ) << endl;
    cout << "----------------------------------" << endl;
}


// =============================================================================
//      PROCESSING JOYSTICKS
// =============================================================================


// maps instance ids to joystick pointers
map< Sint32, SDL_Joystick* > ConnectedJoysticks;

// -----------------------------------------------------------------------------

void ProcessJoystickAdded( SDL_JoyDeviceEvent Event )
{
    SDL_Joystick* NewJoystick = SDL_JoystickOpen( Event.which );
    if( !NewJoystick )  return;
    
    Sint32 AddedInstanceID = SDL_JoystickInstanceID( NewJoystick );
    cout << "Joystick added:" << endl;
    LogJoystick( AddedInstanceID );
}

// -----------------------------------------------------------------------------


void ProcessJoystickRemoved( SDL_JoyDeviceEvent Event )
{
    Sint32 RemovedInstanceID = Event.which;
    
    for( auto it = ConnectedJoysticks.begin(); it != ConnectedJoysticks.end(); it++ )
      if( it->first == RemovedInstanceID )
      {
          ConnectedJoysticks.erase( it );
    
          cout << "Joystick removed: ";
          LogJoystick( RemovedInstanceID );
          return;
      }
}

// -----------------------------------------------------------------------------

void ProcessJoystickButtonDown( SDL_JoyButtonEvent Event )
{
    Sint32 InstanceID = Event.which;
    cout << "Pressed button " << (int)Event.button << " on:" << endl;
    LogJoystick( InstanceID );
}

// -----------------------------------------------------------------------------

void ProcessJoystickAxisMotion( SDL_JoyAxisEvent Event )
{
    Sint32 InstanceID = Event.which;
    cout << "Moved axis " << (int)Event.axis << " on:" << endl;
    LogJoystick( InstanceID );
}

// -----------------------------------------------------------------------------

void ProcessJoystickHatMotion( SDL_JoyHatEvent Event )
{
    Sint32 InstanceID = Event.which;
    cout << "Moved hat " << (int)Event.hat << " on:" << endl;
    LogJoystick( InstanceID );
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main()
{
	try
	{
        // log SDL2 version
        SDL_version Version;
        SDL_GetVersion( &Version );
        cout << "SDL2 Version " << (int)Version.major << "." << (int)Version.minor << "." << (int)Version.patch << endl;
        
        // init SDL
        cout << "Initializing SDL" << endl;
        
        Uint32 SDLSubsystems =
        (
            SDL_INIT_VIDEO      |
            SDL_INIT_EVENTS     |
            SDL_INIT_JOYSTICK
        );
        
        if( SDL_Init( SDLSubsystems ) != 0 )
          throw runtime_error( string("Cannot initialize SDL: ") + SDL_GetError() );
        
        // enable SDL joystick events
        SDL_JoystickEventState( SDL_ENABLE );
        
        // we need to create a window for SDL to receive any events
        cout << "Creating SDL window" << endl;
        
        SDL_Window* Window = SDL_CreateWindow
        (
           "Joystick test",
           SDL_WINDOWPOS_UNDEFINED,
           SDL_WINDOWPOS_UNDEFINED,
           300,
           300,
           SDL_WINDOW_SHOWN
        );
        
        if( !Window )
          throw runtime_error( string("Window cannot be created: ") + SDL_GetError() );
        
        // open all connected joysticks
        int NumberOfJoysticks = SDL_NumJoysticks();
        cout << "Active joysticks: " << NumberOfJoysticks << endl;
        cout << "----------------------------------" << endl;
        
        for( int Joystick = 0; Joystick < NumberOfJoysticks; Joystick++ )
        {
            SDL_Joystick* NewJoystick = SDL_JoystickOpen( Joystick );
            
            if( NewJoystick )
            {
                Sint32 AddedInstanceID = SDL_JoystickInstanceID( NewJoystick );
                ConnectedJoysticks[ AddedInstanceID ] = NewJoystick;
                
                LogJoystick( AddedInstanceID );
            }
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Main loop 
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        cout << "Starting main loop" << endl;
        cout << "----------------------------------" << endl;
        bool GlobalLoopActive = true;
        
        // begin message loop
        while( GlobalLoopActive )
        {
            // process window events
            SDL_Event Event;
            
            while( SDL_PollEvent( &Event ) )
            {
                // respond to the quit event
                if( Event.type == SDL_QUIT )
                  GlobalLoopActive = false;
                
                // respond to keys being pressed
                if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
                {
                    SDL_Keycode Key = Event.key.keysym.sym;
                    
                    // Escape key toggles showing GUI
                    if( Key == SDLK_ESCAPE )
                      GlobalLoopActive = false;
                }
                
                // respond to joysticks being added or removed
                if( Event.type == SDL_JOYDEVICEADDED )
                  ProcessJoystickAdded( Event.jdevice );
                
                if( Event.type == SDL_JOYDEVICEREMOVED )
                  ProcessJoystickRemoved( Event.jdevice );
                
                // process joystick events
                if( Event.type == SDL_JOYBUTTONDOWN )
                  ProcessJoystickButtonDown( Event.jbutton );
                
                if( Event.type == SDL_JOYAXISMOTION )
                  ProcessJoystickAxisMotion( Event.jaxis );
                
                if( Event.type == SDL_JOYHATMOTION )
                  ProcessJoystickHatMotion( Event.jhat );
            }
        }
        
        // close all connected joysticks
        for( auto Pair: ConnectedJoysticks )
          SDL_JoystickClose( Pair.second );
        
        // clean-up in reverse order
        cout << "Exiting" << endl;
        SDL_DestroyWindow( Window );
        SDL_Quit();
	}
    
    catch( exception& Error )
    {
        cout << "ERROR: " << Error.what() << endl;
    }
}
