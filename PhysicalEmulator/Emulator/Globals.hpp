// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/Definitions.hpp"
    #include "../DesktopInfrastructure/Texture.hpp"
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    
    // forward declarations for all needed classes
    class VirconEmulator;
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


// program state
extern bool GlobalLoopActive;
extern bool MouseIsOnWindow;
extern std::string EmulatorFolder;

// used file paths
extern std::string BiosFileName;
extern std::string CartridgePath;
extern std::string MemoryCardPath;


// =============================================================================
//      VIDEO OBJECTS
// =============================================================================


extern OpenGL2DContext OpenGL2D;
extern std::string VertexShader;
extern std::string FragmentShader;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// instance of the Vircon virtual machine
extern VirconEmulator Vircon;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
