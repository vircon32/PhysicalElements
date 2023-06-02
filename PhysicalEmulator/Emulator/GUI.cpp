// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    #include "../DesktopInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "GUI.hpp"
    #include "VirconEmulator.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


void SetFullScreen()
{
    // pause emulation at window events to
    // ensure sound is restored after them
    bool WasRunning = Vircon.PowerIsOn && !Vircon.Paused;
    
    if( WasRunning )
      Vircon.Pause();
    
    // set full screen
    OpenGL2D.SetFullScreen();
    
    // resume emulation if needed
    if( WasRunning )
      Vircon.Resume();
}

// -----------------------------------------------------------------------------

// renders the emulator's framebuffer onto the main program's
// window to make it visible. Since the console implementation
// can change OpenGL's render properties, we need to wrap this
// to ensure the framebuffer is rendered correctly
void ShowEmulatorWindow()
{
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    
    // to do the actual drawing on the screen
    // correctly we have to temporarily 
    // override render settings in OpenGL
    OpenGL2D.SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    OpenGL2D.SetTranslation( 0, 0 );
    OpenGL2D.ComposeTransform( false, false );
    OpenGL2D.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // if the emulator is on, draw its display
    // on our window; otherwise just show a "no
    // signal" indicator on a black screen
    OpenGL2D.RenderToScreen();
    
    if( Vircon.PowerIsOn )
      OpenGL2D.DrawFramebufferOnScreen();
    
    // now restore the Vircon render parameters
    VirconWord BlendValue;
    BlendValue.AsInteger = Vircon.GPU.ActiveBlending;
    Vircon.GPU.WritePort( (int32_t)GPU_LocalPorts::ActiveBlending, BlendValue );
    OpenGL2D.MultiplyColor = Vircon.GPU.MultiplyColor;
}
