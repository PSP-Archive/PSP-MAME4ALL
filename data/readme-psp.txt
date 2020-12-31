PSP MAME4ALL v4.9r2 Hires by TTYman, 16/11/2007
-------------------------------------------------------


Install directions:

- Copy in /PSP/GAME (firmware 1.5) or /PSP/GAME150 (firmware 1.5 compatibility for 3.xx OE release);
  There are four different builds in archive:
  1) 1.50 Kernel mode (psp_mame4all_v4.9r2);
  2) 1.50 Kernel mode without neogeo and cps1 support (psp_mame4all_v4.9r2-noneogeo-nocps1);
  3) 2.00 User mode (psp_mame4all_v4.9r2-usermode);
  4) 2.00 User mode without neogeo and cps1 support (psp_mame4all_v4.9r2-usermode-noneogeo-nocps1);
  Use psp_mame4all_v4.9r2 as installation base, then replace EBOOT.PBP with your needed/preferred build;
- Recreate zipped roms with included ClrMame compatible .DAT files (v0.34-v0.36 mixed roms);
- Add created zipped roms files in ROMs subfolder and samples in samples subfolder;
  Samples must be in .sam old Mame pre-v0.35 format, downloadable from http://www.mameworld.net/samples/sam.htm
  or from 


Specific Features:

- Based on latest MAME GP2X release v4.9;
- 480x272 full PSP Hires Support, now games with >320 orizz. and >240 vert. pixels are fully playable (Loderun, Galaga etc.);
- Overclock support, from 133 to 333 MHz;
- Fixed, Fixed Divided by 2, Software Scaled and Software Full Screen Stretched resolutions (all maximum 480x272);
- Frameskip as GP2X version (fixed 0-5, auto 1-5);
- Vsync on/off;
- Save configuration support, default and per game;
- 22KHz with 16 voices predefined sounds, 33KHz and 44KHz frequencies with 4, 8 and 16 voices combinations added;
- Preliminary analog support;
- Autocentering fix for all resolutions;
- Optimized compiled code as derived from GP2X version, but for PSP cpu (fastest graphics core!);
- Stripped & Packed executable, only 1.5MB;
- Original graphics theme from Gold edition (user mode version only);
- Music in menu (filename music.psp in data folder), in these supported formats: .MOD, .S3M, .IT, .XM and .OGG;
- Added, as bonus, 1st classified OldSkool at Assembly 2007 Music Competition (Commodore 64 based), in .OGG format;
- Graphic theme 480x272 BMP skins support, both pspmenu480.bmp and pspsplash480.bmp in folder skins (user mode version only);
- Dipswitches settings;

Specific Controls:

- Button SELECT: Insert credits.
- Button START: Play.
- Button HOME: after Pause exit game and return to menu.
- Buttons L+R: Activate/deactivate screen fixed/fixed div2/scale/stretch.
- Buttons L+R pressed during one second: Pause.
- Buttons L+HOME simultaneously: Reset emulation.



Fixed from 4.9r1

- Usermode builds now works.
- Dipswitches settings added.
- Some Z80 core speedups.
- Vector based games like Asteroids now run at native resolution of 480x272 pixels.
- Preliminary memory management to clean-up things in certain romset (due to original MAME bugs) before starting a new emulation.
- In case of memory fragmentation errors emulator now quits without lockups.
- Better sound support in certain games, like Teenage Mutant Hero Turtles etc.
- Graphics themes disabled in 1.50 kernel mode to gain more memory available, used a simple blue gradient instead.
- Graphics mode saved with settings.

Fixed from 4.7r3 (same as 4.8r1 unreleased)

- Crash when sampled used.
- Unified builds, thanks to new Z80 core, Mame 0.53 based, compatible with system16, neogeo and classic roms.
- Implemented PSP-2000 (slim) specific support, so double memory available to load roms, thanks to keiich-san code.

Fixed from 4.7r2:

- Sounds now are near arcade perfect, with minor gliches in some sound chips emulations;
- Gold (CPS1, System16 and Neogeo) specific emulations are now stand-alone, as require different z80 cpu implementation;
- Improved performance with sounds, oldies with 0 frame skip and vsync on, others with auto-1 and auto-2; 

Fixed from 4.7:

- 133 MHz in menu and while paused, for battery saving; 
- VSync really works now;
- Corrected build configuration and Makefile, so sounds in various games are now back;
- Correct fixed, div2, sw scaled and sw stretched resolutions, with autocentering;


Known Problems:

- General audio with some gliches due to original unoptimized threaded sound core;
- Preliminary Analog stick support doesn't recognize very fast movements;
- No rotate support;


TTYman
http://ttyman.free.fr

