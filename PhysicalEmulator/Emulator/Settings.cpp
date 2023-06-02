// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/LogStream.hpp"
    #include "../DesktopInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "Settings.hpp"
    #include "VirconEmulator.hpp"
    #include "GUI.hpp"
    #include "FilePaths.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <set>              // [ C++ STL ] Sets
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // include external headers
    #include <tinyxml2.h>
    
    // declare used namespaces
    using namespace std;
    using namespace tinyxml2;
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR INPUT MAPPINGS
// =============================================================================


JoystickControl::JoystickControl()
{
    IsAxis = false;
    IsHat = false;
    ButtonIndex = -1;
    AxisIndex = -1;
    HatIndex = -1;
    AxisPositive = true;
    HatDirection = SDL_HAT_CENTERED;
}


// =============================================================================
//     OPERATION WITH GUIDS
// =============================================================================


string GUIDToString( SDL_JoystickGUID GUID )
{
    char GUIDString[ 35 ];
    SDL_JoystickGetGUIDString( GUID, GUIDString, 34 );  
    return GUIDString;  
}

// -----------------------------------------------------------------------------

bool GUIDStringIsValid( const string& GUIDString )
{
    // length must be even and no greater than 32 characters
    if( GUIDString.size() > 32 ) return false;
    if( GUIDString.size() &  1 ) return false;
    
    // characters must be hexadecimal and lowercase
    for( char c: GUIDString )
    {
        if( isdigit( c ) ) continue;
        if( isupper( c ) ) return false;
        if( c < 'a' && c > 'f' ) return false;
    }
    
    return true;
}


// =============================================================================
//      GLOBALS FOR INPUT PROFILES
// =============================================================================


// SDL joystick instances --> SDL joystick
map< Sint32, SDL_Joystick* > ConnectedJoysticks;

// a single mapping, applicable for all V32 gamepads
// (since they are all considered identical)
JoystickMapping Vircon32GamepadMapping;

// maps each of the 4 gamepad ports to an SDL joystick instance
// (an invalid instance of -1 means no gamepad is connected)
Sint32 GamepadInstanceIDs[ Constants::MaximumGamepads ];

// stores the expected path string for each of the 4 gamepad ports
std::string GamepadPaths[ Constants::MaximumGamepads ];


// =============================================================================
//      XML HELPER FUNCTIONS
// =============================================================================


// automation for child elements in XML
XMLElement* GetRequiredElement( XMLElement* Parent, const string& ChildName )
{
    if( !Parent )
      THROW( "Parent element NULL" );
    
    XMLElement* Child = Parent->FirstChildElement( ChildName.c_str() );
    
    if( !Child )
      THROW( "Cannot find element <" + ChildName + "> inside <" + Parent->Name() + ">" );
    
    return Child;
}

// -----------------------------------------------------------------------------

// automation for string attributes in XML
string GetRequiredStringAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      THROW( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      THROW( "Cannot find attribute '" + AtributeName + "' inside <" + Element->Name() + ">" );
    
    return Attribute->Value();
}

// -----------------------------------------------------------------------------

// automation for integer attributes in XML
int GetRequiredIntegerAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      THROW( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      THROW( "Cannot find attribute '" + AtributeName + "' inside <" + Element->Name() + ">" );
    
    // attempt integer conversion
    int Number = 0;
    XMLError ErrorCode = Element->QueryIntAttribute( AtributeName.c_str(), &Number );
    
    if( ErrorCode != XML_SUCCESS )
      THROW( "Attribute '" + AtributeName + "' inside <" + Element->Name() + "> must be an integer number" );
    
    return Number;
}

// -----------------------------------------------------------------------------

// automation for yes/no attributes in XML
bool GetRequiredYesNoAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      THROW( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      THROW( "Cannot find attribute '" + AtributeName + "' inside <" + Element->Name() + ">" );
    
    if( ToLowerCase( Attribute->Value() ) == "yes" )
      return true;
    
    if( ToLowerCase( Attribute->Value() ) == "no" )
      return false;
    
    THROW( "Attribute '" + AtributeName + "' inside <" + Element->Name() + "> must be either 'yes' or 'no'" );
}


// =============================================================================
//      RESETTING INPUT MAPPINGS
// =============================================================================


void SetDefaultControls()
{
    // set the default joystick profile
    memset( (void*)(&Vircon32GamepadMapping), 0, sizeof(Vircon32GamepadMapping) );
    
    Vircon32GamepadMapping.Left.IsAxis = true;
    Vircon32GamepadMapping.Left.AxisIndex = 0;
    Vircon32GamepadMapping.Left.AxisPositive = false;
    
    Vircon32GamepadMapping.Right.IsAxis = true;
    Vircon32GamepadMapping.Right.AxisIndex = 0;
    Vircon32GamepadMapping.Right.AxisPositive = true;
    
    Vircon32GamepadMapping.Up.IsAxis = true;
    Vircon32GamepadMapping.Up.AxisIndex = 1;
    Vircon32GamepadMapping.Up.AxisPositive = false;
    
    Vircon32GamepadMapping.Down.IsAxis = true;
    Vircon32GamepadMapping.Down.AxisIndex = 1;
    Vircon32GamepadMapping.Down.AxisPositive = true;
    
    Vircon32GamepadMapping.ButtonA.ButtonIndex = 0;
    Vircon32GamepadMapping.ButtonB.ButtonIndex = 1;
    Vircon32GamepadMapping.ButtonX.ButtonIndex = 2;
    Vircon32GamepadMapping.ButtonY.ButtonIndex = 3;
    Vircon32GamepadMapping.ButtonL.ButtonIndex = 4;
    Vircon32GamepadMapping.ButtonR.ButtonIndex = 5;
    Vircon32GamepadMapping.ButtonStart.ButtonIndex = 6;
}


// =============================================================================
//      XML LOAD FUNCTIONS
// =============================================================================


void LoadJoystickControl( JoystickControl* LoadedControl, XMLElement* Parent, const string& ControlName )
{
    XMLElement* ControlElement = Parent->FirstChildElement( ControlName.c_str() );
    
    // controls may be unmapped
    if( !ControlElement )
    {
        LoadedControl->IsAxis = false;
        LoadedControl->IsHat = false;
        LoadedControl->ButtonIndex = -1;
        return;
    }
    
    // when mapped, must be either a button or an axis
    const XMLAttribute* ButtonAttribute = ControlElement->FindAttribute( "button" );
    const XMLAttribute* AxisAttribute = ControlElement->FindAttribute( "axis" );
    const XMLAttribute* HatAttribute = ControlElement->FindAttribute( "hat" );
    
    if( ButtonAttribute )
    {
        LoadedControl->IsAxis = false;
        XMLError ErrorCode = ControlElement->QueryIntAttribute( "button", &LoadedControl->ButtonIndex );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( string("Attribute 'button' in <") + ControlElement->Name() + "> must be a number" );
    }
    
    else if( AxisAttribute )
    {
        LoadedControl->IsAxis = true;
        XMLError ErrorCode = ControlElement->QueryIntAttribute( "axis", &LoadedControl->AxisIndex );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( string("Attribute 'axis' in <") + ControlElement->Name() + "> must be a number" );
        
        // for an axis, it is mandatory to indicate a direction
        string AxisDirection = GetRequiredStringAttribute( ControlElement, "direction" );
        
        if( ToLowerCase( AxisDirection ) == "minus" )
          LoadedControl->AxisPositive = false;
        
        else if( ToLowerCase( AxisDirection ) == "plus" )
          LoadedControl->AxisPositive = true;
        
        else
          THROW( "Axis direction must be either 'plus' or 'minus'" );
    }
    
    else if( HatAttribute )
    {
        LoadedControl->IsHat = true;
        XMLError ErrorCode = ControlElement->QueryIntAttribute( "hat", &LoadedControl->HatIndex );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( string("Attribute 'hat' in <") + ControlElement->Name() + "> must be a number" );
        
        // for a hat, it is mandatory to indicate a direction
        string HatDirection = GetRequiredStringAttribute( ControlElement, "direction" );
        HatDirection = ToLowerCase( HatDirection );
        
        if( HatDirection == "left" )
          LoadedControl->HatDirection = SDL_HAT_LEFT;
        
        else if( HatDirection == "right" )
          LoadedControl->HatDirection = SDL_HAT_RIGHT;
        
        else if( HatDirection == "up" )
          LoadedControl->HatDirection = SDL_HAT_UP;
        
        else if( HatDirection == "down" )
          LoadedControl->HatDirection = SDL_HAT_DOWN;
        
        else
          THROW( "Hat direction must be one of: 'left', 'right', 'up' or 'down'" );
    }
    
    else
      THROW( string("For a joystick, element <") + ControlElement->Name() + "> must include 'button', 'axis' or 'hat' attribute" );
}

// -----------------------------------------------------------------------------

void LoadControls( const std::string& FilePath )
{
    LOG( "Loading controls from \"" + FilePath + "\"" );
    
    try
    {
        // load file and parse it as XML
        XMLDocument FileDoc;
        XMLError ErrorCode = FileDoc.LoadFile( FilePath.c_str() );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( "Cannot read XML from file path " + FilePath );
          
        // obtain XML root
        XMLElement* ControlsRoot = FileDoc.FirstChildElement( "controls" );
        
        if( !ControlsRoot )
          THROW( "Cannot find <controls> root element" );
        
        // check document version number
        int Version = GetRequiredIntegerAttribute( ControlsRoot, "version" );
        
        if( Version < 1 || Version > 2 )
          THROW( "Document version number is" + to_string( Version ) + ", only versions 1 and 2 are supported" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // read the joystick profile
        // (it must exist and be unique)
        XMLElement* JoystickRoot = ControlsRoot->FirstChildElement( "joystick" );
        
        if( JoystickRoot->NextSiblingElement( "joystick" ) )
          THROW( "There can only be 1 joystick mapping" );
        
        // read joystick profile nickname
        string ProfileName = GetRequiredStringAttribute( JoystickRoot, "nickname" );
        
        if( ProfileName == "")
          THROW( "Profile name cannot be empty" );
        
        // read joystick device name
        XMLElement* JoystickNameElement = GetRequiredElement( JoystickRoot, "name" );
        string JoystickName = "";
        
        if( JoystickNameElement->GetText() )
          JoystickName = JoystickNameElement->GetText();
        
        // read joystick GUID
        XMLElement* GUIDElement = GetRequiredElement( JoystickRoot, "guid" );
        
        if( !GUIDElement->GetText() )
          THROW( "Joystick GUID cannot be empty" );
        
        string GUIDString = GUIDElement->GetText();
        
        // validate and convert GUID
        if( !GUIDStringIsValid( GUIDString ) )
          THROW( "Joystick GUID is not valid" );
        
        SDL_JoystickGUID GUID = SDL_JoystickGetGUIDFromString( GUIDString.c_str() );
        
        // fill in basic profile info
        Vircon32GamepadMapping.GUID = GUID;
        Vircon32GamepadMapping.ProfileName = ProfileName;
        Vircon32GamepadMapping.JoystickName = JoystickName;
        
        // load directions
        LoadJoystickControl( &Vircon32GamepadMapping.Left , JoystickRoot, "left"  );
        LoadJoystickControl( &Vircon32GamepadMapping.Right, JoystickRoot, "right" );
        LoadJoystickControl( &Vircon32GamepadMapping.Up   , JoystickRoot, "up"    );
        LoadJoystickControl( &Vircon32GamepadMapping.Down , JoystickRoot, "down"  );
        
        // load buttons
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonA, JoystickRoot, "button-a" );
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonB, JoystickRoot, "button-b" );
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonX, JoystickRoot, "button-x" );
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonY, JoystickRoot, "button-y" );
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonL, JoystickRoot, "button-l" );
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonR, JoystickRoot, "button-r" );
        LoadJoystickControl( &Vircon32GamepadMapping.ButtonStart, JoystickRoot, "button-start" );
    }
    
    // as backup, set our default configuration
    catch( exception& e )
    {
        LOG( "Cannot load controls file: " << e.what() );
        SetDefaultControls();
    }
}


// =============================================================================
//      HANDLING SETTINGS FROM XML FILE
// =============================================================================


// we need to always have this as backup
// if the XML does not exist or has errors
void SetDefaultSettings()
{
    LOG( "Applying default settings" );
    
    // video configuration
    SetFullScreen();
    
    // audio configuration
    Vircon.SetMute( false );
    Vircon.SetOutputVolume( 1.0 );
    
    // unloaded cartridge
    Vircon.UnloadCartridge();
    
    // unloaded memory card
    Vircon.UnloadMemoryCard();
    
    // set keyboard for first gamepad
    // set no device for the rest of gamepads
    for( int i = 0; i < Constants::MaximumGamepads; i++ )
    {
        Vircon.GamepadController.ProcessConnectionChange( i, false );
        GamepadInstanceIDs[ i ] = -1;
    }
    
    // set default load paths
    CartridgePath  = EmulatorFolder + "Cartridge" + PathSeparator + "CartridgeROM.v32";
    MemoryCardPath = EmulatorFolder + "Card" + PathSeparator + "MemoryCardRAM.memc";
}

// -----------------------------------------------------------------------------

void LoadSettings( const string& FilePath )
{
    LOG( "Loading settings from \"" + FilePath + "\"" );
    
    try
    {
        // load file and parse it as XML
        XMLDocument FileDoc;
        XMLError ErrorCode = FileDoc.LoadFile( FilePath.c_str() );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( "Cannot read XML from file path " + FilePath );
          
        // obtain XML root
        XMLElement* SettingsRoot = FileDoc.FirstChildElement( "settings" );
        
        if( !SettingsRoot )
          THROW( "Cannot find <settings> root element" );
        
        // check document version number
        int Version = GetRequiredIntegerAttribute( SettingsRoot, "version" );
        
        if( Version < 1 || Version > 4 )
          THROW( "Document version number is" + to_string( Version ) + ", only versions 1 through 4 are supported" );
        
        // load BIOS location (optional)
        XMLElement* BiosElement = SettingsRoot->FirstChildElement( "bios" );
        
        if( BiosElement )
          BiosFileName = GetRequiredStringAttribute( BiosElement, "file" );
        
        // load video settings (omitted)
        SetFullScreen();
        
        // load audio settings (omitted)
        Vircon.SetOutputVolume( 1.0 );
        
        // load audio buffers settings
        XMLElement* AudioBuffersElement = GetRequiredElement( SettingsRoot, "audio-buffers" );
        int NumberOfBuffers = GetRequiredIntegerAttribute( AudioBuffersElement, "number" );
        Clamp( NumberOfBuffers, MIN_BUFFERS, MAX_BUFFERS );
        
        // apply audio buffers settings
        Vircon.SPU.NumberOfBuffers = NumberOfBuffers;
        
        // configure gamepads
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // access the Nth gamepad element
            string GamepadElementName = string("gamepad-") + to_string( Gamepad+1 );
            XMLElement* GamepadRoot = GetRequiredElement( SettingsRoot, GamepadElementName.c_str() );
            
            // initially, leave the gamepad unmapped disconnected
            Vircon.GamepadController.ProcessConnectionChange( Gamepad, false );
            GamepadInstanceIDs[ Gamepad ] = -1;
            
            // read gamepad path
            GamepadPaths[ Gamepad ] = GetRequiredStringAttribute( GamepadRoot, "path" );
        }
        
        // read load paths for cartridge and memory card
        XMLElement* LoadPathsRoot = GetRequiredElement( SettingsRoot, "load-paths" );
        XMLElement* MemoryCardElement = GetRequiredElement( LoadPathsRoot, "memory-card" );
        XMLElement* CartridgeElement = GetRequiredElement( LoadPathsRoot, "cartridge" );
        
        MemoryCardPath = GetRequiredStringAttribute( MemoryCardElement, "path" );
        CartridgePath  = GetRequiredStringAttribute( CartridgeElement, "path" );
    }
    
    // as backup, set our default configuration
    catch( exception& e )
    {
        LOG( "WARNING: Cannot load settings file: " << e.what() );
        SetDefaultSettings();
    }
}
