
# Test program for **tiger_gl** library.

### Features:

	1. No X11 required.
	2. Simple to use and understand.
	3. All C library.
	4. Developed for Raspberry Pi.
	5. Current widgets are:

		- Image.
		- Label.
		- Checkbox.
		- Radio Button.
		- Progress bar.
		- Spinner
		- Button with icon support.
		- Touch screen support.
		- Many simple font sizes.
		- Support for video.

### Building the program.

You will need the **tiger_gl** and **utils** repositories and of course this repository.

See **[tiger_gl](https://github.com/tigerkelly/tiger_gl)** repository for complile instructions.

See **[utils](https://github.com/tigerkelly/utils)** repository for complile instructions.

For the **tiger_gl-test** repository just type **make** after **tiger_gl** has been compiled.

This example program is using the 5" touch monitor from Elecrow, model QDtech MPI5001. Which has  800x480 resolution but I increased the resolution to 1024x768.  With this higher resolution can cause some lose of quality.

See my /boot/config.txt file in the code.

Note, this has only been compiled on a RPI 3B+ with the *Linux raspberrypi 4.19.75-v7+* version.
