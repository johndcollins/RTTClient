RTTClient for Raspberry Pi 4
----

This is a RTTClient for Raspberry Pi 4 that I wrote for my MFDs.

I didn't want to run another two cables to the front of my pit and I can't add anymore outputs to my main computer anyway as it's running 3 projectors. Adding more outputs with different resolutions messes up the Flyelise and nvidia warping.

Current Features:
Display support:
  * `HUD`
  * `PFL`
  * `DED`
  * `RWR`
  * `MFDLEFT`
  * `MFDRIGHT`
  * `HMS`
  
Additional Features:
  * `Ability to Flip in both Horizontal and Vertical` (e.g HUD_FLIPPED_VERTICALLY = 1 or HUD_FLIPPED_HORIZONTICALLY = 1)
  * `Move and Resize the windows` (WINDOWS_MOVABLE = 1 - This doesn't actually save the values to the ini file, but help you find the values to put in the ini file)
  * `Show Position on screen` (SHOW_POSITION_INFO = 1 to help get the current values after moving and resizing)
  * `Set Background Images for each display instead of the standard orange helpers.` (MFDLEFT_BACKGROUND_IMAGE = <path to png or jpg image file) 
  
Possible future enhancements:
  * [ ] `ONTOP`
  * [ ] `RWR_GRID`
  * [ ] `SHAREDMEMORY`
  * [ ] `RENDERER` (It just uses the default right now, I think Rpi supports OpenGL, OpenGL ES and OpenGL ES2)
  * [ ] `Cross platform to this can be used on Windows or Mac`
  
 
  
