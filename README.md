# PSP MAME4ALL
PSP MAME4ALL is port created by TTYman, and based on the MAME GP2x emulator developed by Franxis. 

MAME GP2x saw its last release (version 5.1) in November 2007. The same developer would later start the 'MAME4ALL for GP2X' project - which supported a wider romset, and had its last release in March 2010 (version 2.5). This PSP port, despite its name, is unrelated to this later project.

The compatibility list for the GP2X version (on which this port is based on) can be found here:

http://wiki.gp2x.org/articles/m/a/m/MAME_GP2X.html#Supported_Games_1139_romset

## Supported games

There are 1204 different supported romsets. For more details, see the [gamelist.txt](https://github.com/PSP-Archive/PSP-MAME4ALL/blob/main/data/gamelist.txt) file within the release. Games have to be copied into the /roms/ folder.

### ROM names

Folder names or ZIP file names are listed on "gamelist.txt" file.

Romsets have to be MAME 0.34 final version ones (December 1998), but there are also some new added romsets from later MAME versions, as well as modified romsets.

Please use the [clrmame_roms.dat](https://github.com/PSP-Archive/PSP-MAME4ALL/blob/main/data/clrmame_roms.dat) file to convert romsets from other MAME versions, by using the ClrMAME Pro utility. The process is best explained by [this YouTube video](https://www.youtube.com/watch?v=_lssz2pAba8) - the only difference is that you will be using clrmame_roms.dat rather than other files.

NOTE: File and directory names are case sensitive. Put all file and directory names in lower case.

## Controls

- SELECT: insert credits. (SELECT+Up=credits 2P, SELECT+Right=credits 3P, SELECT+Down=credits 4P)
- START: play. (START+Up=Start 2P, START+Right=Start 3P, START+Down=Start 4P).
- L+R: change screen orientation and activate/deactivate screen scale/stretch.
- Hold L+R for one second: pause. Press Home from this screen to exit to the MAME4ALL menu.
- Hold L and press HOME: reset emulation.

## Compiling
1. Tweaks to the PSP toolchain:
	a. in newlib.h, undefine `_LDBL_EQ_DBL`
	b. in std_abs.h, replace the `#ifndef __CORRECT_ISO_CPP_MATH_H_PROTO` block with this:
		```
		#ifndef __CORRECT_ISO_CPP_MATH_H_PROTO
		  //inline _GLIBCXX_CONSTEXPR double
		  //abs(double __x)
		  //{ return __builtin_fabs(__x); }

		  inline _GLIBCXX_CONSTEXPR float
		  abs(float __x)
		  { return __builtin_fabsf(__x); }

		  //inline _GLIBCXX_CONSTEXPR long double
		  //abs(long double __x)
		  //{ return __builtin_fabsl(__x); }
		#endif
		```
2. from the PSP-MAME4ALL folder:
	`make -f Makefile.psp clean all; make -f Makefile.psp`
