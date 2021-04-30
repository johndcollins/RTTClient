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
  * `Ability to Flip in both Horizontal and Vertical` (e.g HUD_FLIPPED_VERTICALLY = 1 or HUD_FLIPPED_HORIZONTALLY = 1)
  * `Move and Resize the windows` (WINDOWS_MOVABLE = 1 - This doesn't actually save the values to the ini file, but help you find the values to put in the ini file)
  * `Show Position on screen` (SHOW_POSITION_INFO = 1 to help get the current values after moving and resizing)
  * `Set Background Images for each display instead of the standard orange helpers.` (MFDLEFT_BACKGROUND_IMAGE = "path to png or jpg image file") 
  
Possible future enhancements:
  * [ ] `ONTOP`
  * [ ] `RWR_GRID`
  * [ ] `SHAREDMEMORY`
  * [ ] `RENDERER` (It just uses the default right now, I think Rpi supports OpenGL, OpenGL ES and OpenGL ES2)
  * [ ] `Cross platform to this can be used on Windows or Mac`
  
 
# Setup
You will need to build this project.

One way is to follow these steps (can be done in many different ways)

Install the following dependencies:
```c
sudo apt-get install -y build-essential gdb g++ zip cmake
sudo apt-get install -y libsdl2-dev
sudo apt install -y libsdl2-image-dev
sudo apt install -y libsdl2-ttf-dev
sudo apt-get install -y libglew-dev
```

Clone RakNet from here : https://github.com/facebookarchive/RakNet
Put it in ~\RakNet
Compile place the libraries in the correct folders

Clone this project in a folder like ~/projects/rttclient on your Rpi 4.
Go to the folder and run:
```c
cmake .
cmake --build .
```

This will create a RTTClient file in the src folder.
Copy the following files to a new folder here ~/RTTClient
```c
RTTClient
RTTClient.ini
font.ttf
start.sh
```

To autostart the RTTClient when you boot the Rpi create this file:
```c
nano .config/autostart/rtt.desktop
```
and enter this in the file and save:
```c
[Desktop Entry]
Type=Application
Name=RTTClient
Exec=/home/pi/RTTClient/start.sh
```
If you put the RTTClient executable in a different folder, you will need to modify this and the startup.sh script.

You might have to make both the RTTClient and the start.sh executable using the chmod +x command
