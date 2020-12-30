
MAME GP2X 5.1 - Source Code (30/Nov/2007)
-----------------------------------------

http://www.talfi.net/gp32_franxis/

Development Environment:
- GCC 4.0.2: DevKitGP2X rc2 (http://sourceforge.net/project/showfiles.php?group_id=114505)

Optionally it can be compiled with GCC 4.1.0 (DevKitARM r18) and linked with GCC 4.0.2 (DevKitGP2X rc2).
In Makefile.gp2x change:
CC     = arm-linux-g++ 		to arm-elf-g++
CXX    = arm-linux-g++ 		to arm-elf-g++

This port is based on MAME 0.34.

Thanks and regards.

- Franxis (fjmar@hotmail.com)


MAME LICENSE
------------

http://www.mame.net
http://www.mamedev.com

Copyright © 1997-2005, Nicola Salmoria and the MAME team. All rights reserved. 

Redistribution and use of this code or any derivative works are permitted provided
that the following conditions are met: 

* Redistributions may not be sold, nor may they be used in a commercial product or activity. 

* Redistributions that are modified from the original source must include the complete source
code, including the source code for all components used by a binary built from the modified
sources. However, as a special exception, the source code distributed need not include 
anything that is normally distributed (in either source or binary form) with the major 
components (compiler, kernel, and so on) of the operating system on which the executable
runs, unless that component itself accompanies the executable. 

* Redistributions must reproduce the above copyright notice, this list of conditions and the
following disclaimer in the documentation and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
