
#-----------
# SDL port :
#-----------
OBJS = \
	src/mame.o \
	src/common.o \
	src/driver.o \
	src/cpuintrf.o \
	src/memory.o \
	src/timer.o \
	src/palette.o \
	src/inptport.o \
	src/unzip.o \
	src/inflate.o \
	src/tilemap.o \
	src/sprite.o \
	src/sndhrdw/irem.o \
	src/sndhrdw/3812intf.o \
	src/sndhrdw/tms5220.o \
	src/sndhrdw/5220intf.o \
	src/sndhrdw/vlm5030.o \
	src/sndhrdw/pokey.o \
	src/sndhrdw/sn76496.o \
	src/sndhrdw/nes.o \
	src/sndhrdw/nesintf.o \
	src/sndhrdw/astrocde.o \
	src/sndhrdw/votrax.o \
	src/sndhrdw/dac.o \
	src/sndhrdw/samples.o \
	src/sndhrdw/streams.o \
	src/machine/z80fmly.o \
	src/machine/6821pia.o \
	src/vidhrdw/generic.o \
	src/sndhrdw/generic.o \
	src/vidhrdw/vector.o \
	src/vidhrdw/avgdvg.o \
	src/machine/mathbox.o \
	src/sndhrdw/namco.o \
	src/sndhrdw/namcos1.o \
	src/machine/segacrpt.o \
	src/machine/atarigen.o \
	src/machine/slapstic.o \
	src/machine/ticket.o \
	src/sndhrdw/upd7759.o \
	src/sndhrdw/k007232.o \
	src/sndhrdw/msm5205i.o

ifdef GP2X
OBJS += \
	src/gp2x/minimal.o \
	src/gp2x/uppermem.o \
	src/gp2x/cpuctrl.o \
	src/gp2x/usbjoy.o \
	src/gp2x/usbjoy_mame.o \
	src/gp2x/squidgehack.o \
	src/gp2x/flushcache.o \
	src/gp2x/gp2x.o \
	src/gp2x/gp2x_menu.o \
	src/gp2x/video.o \
	src/gp2x/input.o \
	src/gp2x/sound.o \
	src/gp2x/fileio.o \
	src/gp2x/memcmp.o \
	src/gp2x/memcpy.o \
	src/gp2x/memset.o \
	src/gp2x/strcmp.o \
	src/gp2x/strlen.o \
	src/gp2x/strncmp.o
else
OBJS += \
	src/sdl/fastmem.o \
	src/sdl/minimal.o \
	src/sdl/gp2x.o \
	src/sdl/gp2x_menu.o \
	src/sdl/video.o \
	src/sdl/video16.o \
	src/sdl/input.o \
	src/sdl/sound.o \
	src/sdl/audio.o \
	src/sdl/filecache.o \
	src/sdl/fileio.o
endif


ifdef ADPCM_CORE
OBJS += src/sndhrdw/adpcm.o 
else
ifdef OKIM6295_CORE
OBJS += src/sndhrdw/adpcm.o
else
ifdef MSM5205_CORE
OBJS += src/sndhrdw/adpcm.o
endif
endif
endif

ifdef YM2151_CORE
OBJS += \
	src/sndhrdw/fm.o \
	src/sndhrdw/2151intf.o \
	src/sndhrdw/ym2151.o \
	src/sndhrdw/ym3812.o 
endif

ifdef AY8910_CORE
OBJS += src/sndhrdw/ay8910.o
endif

ifdef YM2XXX_CORE
OBJS += \
	src/sndhrdw/psgintf.o \
	src/sndhrdw/2610intf.o \
	src/sndhrdw/2612intf.o \
	src/sndhrdw/ym2413.o
ifndef YM2151_CORE
OBJS += src/sndhrdw/fm.o 
endif
endif

#------
# CPU :
#------

ifndef CZ80
ifndef Z80_ASM_CORE
OBJS_CPU += src/z80/z80.o 
else
ifdef GP2X
OBJS_CPU += src/drz80/drz80.o src/drz80/z80.o
else

ifndef RAZE_IS_FAZE
ifdef MAME4ALL_GOLD
OBJS_CPU += src/raze/raze/raze.o
else
OBJS_CPU +=	src/raze/raze/raze_1.o \
		src/raze/raze/raze_2.o \
		src/raze/raze/raze_3.o \
		src/raze/raze/raze_4.o
endif
else
ifdef MAME4ALL_GOLD
OBJS_CPU += src/raze/faze/raze.o
else
OBJS_CPU +=	src/raze/faze/raze_1.o \
		src/raze/faze/raze_2.o \
		src/raze/faze/raze_3.o \
		src/raze/faze/raze_4.o
endif
endif


OBJS_CPU += src/raze/z80intrf.o
endif
endif
else
OBJS_CPU += src/cz80/cz80.o src/cz80/z80intrf.o
endif

ifdef M6502_CORE
OBJS_CPU += src/m6502/m6502.o
endif

ifdef H6280_CORE
OBJS_CPU += src/h6280/h6280.o
endif

ifdef TMS32010_CORE
OBJS_CPU += src/tms32010/tms32010.o
endif

ifdef M6809_CORE
OBJS_CPU += src/m6809/m6809.o
endif

ifdef M68000_CORE
ifdef M68000_ASM_CORE
ifndef GP2X
OBJS_CPU += \
	src/fame/fame.o \
	src/fame/musashi.o \
	src/fame/m68000.o
else
OBJS_CPU += \
	src/cyclone/cyclone.o \
	src/cyclone/m68000.o
endif
else
ifdef FAMEC
OBJS_CPU += \
	src/fame/famec.o \
	src/fame/musashi.o \
	src/fame/m68000.o
else
OBJS_CPU += \
	src/m68000/opcode0.o \
	src/m68000/opcode1.o \
	src/m68000/opcode2.o \
	src/m68000/opcode3.o \
	src/m68000/opcode4.o \
	src/m68000/opcode5.o \
	src/m68000/opcode6.o \
	src/m68000/opcode7.o \
	src/m68000/opcode8.o \
	src/m68000/opcode9.o \
	src/m68000/opcodeb.o \
	src/m68000/opcodec.o \
	src/m68000/opcoded.o \
	src/m68000/opcodee.o \
	src/m68000/mc68kmem.o \
	src/m68000/cpufunc.o 
endif
endif
endif

ifdef M6805_CORE
OBJS_CPU += src/m6805/m6805.o 
endif
ifdef I8039_CORE
OBJS_CPU += src/i8039/i8039.o
endif
ifdef M6808_CORE
OBJS_CPU += src/m6808/m6808.o
endif
ifdef S2650_CORE
OBJS_CPU += src/s2650/s2650.o
endif
ifdef I86_CORE
OBJS_CPU += src/i86/i86.o
endif
ifdef T11_CORE
OBJS_CPU += src/t11/t11.o
endif

ifdef TMS_CORE
OBJS_CPU += src/tms34010/tms34010.o \
	src/tms34010/34010fld.o \
	src/tms9900/tms9900.o
endif

ifdef I8085_CORE
OBJS_CPU += src/i8085/i8085.o 
endif


#---------
# DRIVERS :
#---------
OBJS_PACMAN = \
	src/machine/pacman.o \
	src/drivers/pacman.o \
	src/machine/pacplus.o \
	src/machine/theglob.o \
	src/drivers/maketrax.o \
	src/machine/jrpacman.o \
	src/drivers/jrpacman.o \
	src/vidhrdw/jrpacman.o \
	src/vidhrdw/pengo.o \
	src/drivers/pengo.o


OBJS_GALAXIAN= \
	src/vidhrdw/galaxian.o \
	src/drivers/galaxian.o \
	src/sndhrdw/mooncrst.o \
	src/drivers/mooncrst.o 


OBJS_SCRAMBLE= \
	src/machine/scramble.o \
	src/sndhrdw/scramble.o \
	src/drivers/scramble.o \
	src/vidhrdw/frogger.o \
	src/sndhrdw/frogger.o \
	src/drivers/frogger.o \
	src/drivers/ckongs.o \
	src/drivers/scobra.o \
	src/vidhrdw/amidar.o \
	src/drivers/amidar.o \
	src/vidhrdw/jumpbug.o \
	src/drivers/jumpbug.o \
	src/vidhrdw/fastfred.o \
	src/drivers/fastfred.o 


OBJS_CCLIMBER= \
	src/vidhrdw/cclimber.o \
	src/sndhrdw/cclimber.o \
	src/drivers/cclimber.o \
	src/vidhrdw/seicross.o \
	src/drivers/seicross.o 


OBJS_PHOENIX= \
	src/vidhrdw/phoenix.o \
	src/sndhrdw/phoenix.o \
	src/drivers/phoenix.o \
	src/sndhrdw/pleiads.o \
	src/vidhrdw/naughtyb.o \
	src/drivers/naughtyb.o


OBJS_NAMCO= \
	src/vidhrdw/rallyx.o \
	src/drivers/rallyx.o \
	src/drivers/locomotn.o \
	src/machine/bosco.o \
	src/sndhrdw/bosco.o \
	src/vidhrdw/bosco.o \
	src/drivers/bosco.o \
	src/machine/galaga.o \
	src/vidhrdw/galaga.o \
	src/drivers/galaga.o \
	src/machine/digdug.o \
	src/vidhrdw/digdug.o \
	src/drivers/digdug.o \
	src/vidhrdw/xevious.o \
	src/machine/xevious.o \
	src/drivers/xevious.o \
	src/machine/superpac.o \
	src/vidhrdw/superpac.o \
	src/drivers/superpac.o \
	src/machine/mappy.o \
	src/vidhrdw/mappy.o \
	src/drivers/mappy.o \
	src/vidhrdw/pacland.o \
	src/drivers/pacland.o \
	src/vidhrdw/rthunder.o \
	src/drivers/rthunder.o \
	src/machine/gaplus.o \
	src/vidhrdw/gaplus.o \
	src/drivers/gaplus.o


OBJS_UNIVERS= \
	src/vidhrdw/cosmica.o \
	src/drivers/cosmica.o \
	src/vidhrdw/cheekyms.o \
	src/drivers/cheekyms.o \
	src/machine/panic.o \
	src/vidhrdw/panic.o \
	src/drivers/panic.o \
	src/vidhrdw/ladybug.o \
	src/drivers/ladybug.o \
	src/vidhrdw/mrdo.o \
	src/drivers/mrdo.o \
	src/machine/docastle.o \
	src/vidhrdw/docastle.o \
	src/drivers/docastle.o \
	src/drivers/dowild.o


OBJS_NINTENDO= \
	src/vidhrdw/dkong.o \
	src/sndhrdw/dkong.o \
	src/drivers/dkong.o \
	src/vidhrdw/mario.o \
	src/sndhrdw/mario.o \
	src/drivers/mario.o \
	src/vidhrdw/popeye.o \
	src/drivers/popeye.o \
	src/vidhrdw/punchout.o \
	src/sndhrdw/punchout.o \
	src/drivers/punchout.o


OBJS_MIDW8080= \
	src/machine/8080bw.o \
	src/vidhrdw/8080bw.o \
	src/sndhrdw/8080bw.o \
	src/drivers/8080bw.o \
	src/vidhrdw/m79amb.o \
	src/drivers/m79amb.o 


OBJS_MIDWZ80= \
	src/machine/z80bw.o \
	src/vidhrdw/z80bw.o \
	src/sndhrdw/z80bw.o \
	src/drivers/z80bw.o


OBJS_MEADOWS= \
	src/drivers/lazercmd.o \
	src/vidhrdw/lazercmd.o \
	src/drivers/meadows.o \
	src/sndhrdw/meadows.o \
	src/vidhrdw/meadows.o \
	src/drivers/medlanes.o \
	src/vidhrdw/medlanes.o 


OBJS_ASTROCDE= \
	src/machine/wow.o \
	src/vidhrdw/wow.o \
	src/sndhrdw/wow.o \
	src/drivers/wow.o \
	src/sndhrdw/gorf.o 


OBJS_IREM= \
	src/vidhrdw/mpatrol.o \
	src/drivers/mpatrol.o \
	src/vidhrdw/yard.o \
	src/drivers/yard.o \
	src/vidhrdw/kungfum.o \
	src/drivers/kungfum.o \
	src/vidhrdw/travrusa.o \
	src/drivers/travrusa.o \
	src/vidhrdw/ldrun.o \
	src/drivers/ldrun.o \
	src/vidhrdw/kncljoe.o \
	src/drivers/kncljoe.o


OBJS_MCR= \
	src/machine/mcr.o \
	src/vidhrdw/mcr1.o \
	src/vidhrdw/mcr2.o \
	src/vidhrdw/mcr3.o \
	src/drivers/mcr1.o \
	src/drivers/mcr2.o \
	src/drivers/mcr3.o \
	src/machine/mcr68.o \
	src/vidhrdw/mcr68.o \
	src/drivers/mcr68.o


OBJS_GOTTLIEB= \
	src/vidhrdw/gottlieb.o \
	src/sndhrdw/gottlieb.o \
	src/drivers/gottlieb.o


OBJS_OLDTAITO= \
	src/vidhrdw/crbaloon.o \
	src/drivers/crbaloon.o


OBJS_QIXTAITO= \
	src/machine/qix.o \
	src/vidhrdw/qix.o \
	src/drivers/qix.o


OBJS_TAITO= \
	src/machine/taito.o \
	src/vidhrdw/taito.o \
	src/drivers/taito.o


OBJS_TAITO2= \
	src/vidhrdw/bking2.o \
	src/drivers/bking2.o \
	src/vidhrdw/gsword.o \
	src/drivers/gsword.o \
	src/vidhrdw/gladiatr.o \
	src/drivers/gladiatr.o \
	src/vidhrdw/tokio.o \
	src/drivers/tokio.o \
	src/machine/bublbobl.o \
	src/vidhrdw/bublbobl.o \
	src/drivers/bublbobl.o \
	src/vidhrdw/rastan.o \
	src/sndhrdw/rastan.o \
	src/drivers/rastan.o \
	src/machine/rainbow.o \
	src/drivers/rainbow.o \
	src/machine/arkanoid.o \
	src/vidhrdw/arkanoid.o \
	src/drivers/arkanoid.o \
	src/vidhrdw/superqix.o \
	src/drivers/superqix.o \
	src/machine/twincobr.o \
	src/vidhrdw/twincobr.o \
	src/drivers/twincobr.o \
	src/machine/tnzs.o \
	src/vidhrdw/tnzs.o \
	src/drivers/tnzs.o \
	src/drivers/arkanoi2.o \
	src/machine/slapfght.o \
	src/vidhrdw/slapfght.o \
	src/drivers/slapfght.o \
	src/vidhrdw/superman.o \
	src/drivers/superman.o \
	src/machine/cchip.o \
	src/vidhrdw/taitof2.o \
	src/drivers/taitof2.o \
	src/vidhrdw/ssi.o \
	src/drivers/ssi.o


OBJS_WILLIAMS= \
	src/machine/williams.o \
	src/vidhrdw/williams.o \
	src/drivers/williams.o


OBJS_CAPCOM= \
	src/vidhrdw/vulgus.o \
	src/drivers/vulgus.o \
	src/vidhrdw/sonson.o \
	src/drivers/sonson.o \
	src/vidhrdw/higemaru.o \
	src/drivers/higemaru.o \
	src/vidhrdw/1942.o \
	src/drivers/1942.o \
	src/vidhrdw/exedexes.o \
	src/drivers/exedexes.o \
	src/vidhrdw/commando.o \
	src/drivers/commando.o \
	src/vidhrdw/gng.o \
	src/drivers/gng.o \
	src/vidhrdw/gunsmoke.o \
	src/drivers/gunsmoke.o \
	src/vidhrdw/srumbler.o \
	src/drivers/srumbler.o \
	src/machine/lwings.o \
	src/vidhrdw/lwings.o \
	src/drivers/lwings.o \
	src/vidhrdw/sidearms.o \
	src/drivers/sidearms.o \
	src/vidhrdw/bionicc.o \
	src/drivers/bionicc.o \
	src/vidhrdw/1943.o \
	src/drivers/1943.o \
	src/vidhrdw/blktiger.o \
	src/drivers/blktiger.o \
	src/vidhrdw/tigeroad.o \
	src/drivers/tigeroad.o \
	src/vidhrdw/lastduel.o \
	src/drivers/lastduel.o \
	src/vidhrdw/sf1.o \
	src/drivers/sf1.o \

OBJS_CPS1= \
	src/machine/cps1.o \
	src/vidhrdw/cps1.o \
	src/sndhrdw/qsound.o \
	src/drivers/cps1.o


OBJS_CAPBOWL= \
	src/machine/capbowl.o \
	src/vidhrdw/capbowl.o \
	src/vidhrdw/tms34061.o \
	src/drivers/capbowl.o


OBJS_GREMLIN= \
	src/vidhrdw/blockade.o \
	src/drivers/blockade.o


OBJS_VICDUAL= \
	src/vidhrdw/vicdual.o \
	src/sndhrdw/vicdual.o \
	src/drivers/vicdual.o


OBJS_SEGAV= \
	src/vidhrdw/sega.o \
	src/sndhrdw/sega.o \
	src/machine/sega.o \
	src/drivers/sega.o


OBJS_SEGAR= \
	src/vidhrdw/segar.o \
	src/sndhrdw/segar.o \
	src/machine/segar.o \
	src/drivers/segar.o \
	src/sndhrdw/monsterb.o


OBJS_ZAXXON= \
	src/vidhrdw/zaxxon.o \
	src/sndhrdw/zaxxon.o \
	src/drivers/zaxxon.o \
	src/sndhrdw/congo.o \
	src/drivers/congo.o


OBJS_SYSTEM8= \
	src/vidhrdw/system8.o \
	src/drivers/system8.o


OBJS_SYSTEM16= \
	src/machine/system16.o \
	src/vidhrdw/system16.o \
	src/sndhrdw/system16.o \
	src/sndhrdw/segapcm.o \
	src/sndhrdw/rf5c68.o \
	src/drivers/system16.o


OBJS_BTIME= \
	src/vidhrdw/btime.o \
	src/drivers/btime.o \
	src/vidhrdw/tagteam.o \
	src/drivers/tagteam.o
	

OBJS_DATAEAST= \
	src/vidhrdw/astrof.o \
	src/sndhrdw/astrof.o \
	src/drivers/astrof.o \
	src/vidhrdw/kchamp.o \
	src/drivers/kchamp.o \
	src/vidhrdw/firetrap.o \
	src/drivers/firetrap.o \
	src/vidhrdw/brkthru.o \
	src/drivers/brkthru.o \
	src/vidhrdw/shootout.o \
	src/drivers/shootout.o \
	src/vidhrdw/sidepckt.o \
	src/drivers/sidepckt.o \
	src/vidhrdw/exprraid.o \
	src/drivers/exprraid.o


OBJS_DEC8= \
	src/vidhrdw/dec8.o \
	src/drivers/dec8.o


OBJS_DEC0= \
	src/vidhrdw/karnov.o \
	src/drivers/karnov.o \
	src/machine/dec0.o \
	src/vidhrdw/dec0.o \
	src/drivers/dec0.o \
	src/vidhrdw/darkseal.o \
	src/drivers/darkseal.o \
	src/vidhrdw/tumblep.o \
	src/drivers/tumblep.o \
	src/vidhrdw/cninja.o \
	src/drivers/cninja.o


OBJS_TEHKAN= \
	src/vidhrdw/bombjack.o \
	src/drivers/bombjack.o \
	src/sndhrdw/starforc.o \
	src/vidhrdw/starforc.o \
	src/drivers/starforc.o \
	src/vidhrdw/pbaction.o \
	src/drivers/pbaction.o \
	src/vidhrdw/tehkanwc.o \
	src/drivers/tehkanwc.o \
	src/vidhrdw/solomon.o \
	src/drivers/solomon.o \
	src/vidhrdw/tecmo.o \
	src/drivers/tecmo.o \
	src/vidhrdw/gaiden.o \
	src/drivers/gaiden.o \
	src/vidhrdw/wc90.o \
	src/drivers/wc90.o \
	src/vidhrdw/wc90b.o \
	src/drivers/wc90b.o


OBJS_KONAMI= \
	src/vidhrdw/pooyan.o \
	src/drivers/pooyan.o \
	src/vidhrdw/timeplt.o \
	src/drivers/timeplt.o \
	src/sndhrdw/timeplt.o \
	src/vidhrdw/rocnrope.o \
	src/drivers/rocnrope.o \
	src/sndhrdw/gyruss.o \
	src/vidhrdw/gyruss.o \
	src/drivers/gyruss.o \
	src/machine/konami.o \
	src/vidhrdw/trackfld.o \
	src/sndhrdw/trackfld.o \
	src/drivers/trackfld.o \
	src/vidhrdw/circusc.o \
	src/drivers/circusc.o \
	src/machine/tp84.o \
	src/vidhrdw/tp84.o \
	src/drivers/tp84.o \
	src/vidhrdw/hyperspt.o \
	src/drivers/hyperspt.o \
	src/vidhrdw/sbasketb.o \
	src/drivers/sbasketb.o \
	src/vidhrdw/mikie.o \
	src/drivers/mikie.o \
	src/vidhrdw/yiear.o \
	src/drivers/yiear.o \
	src/vidhrdw/shaolins.o \
	src/drivers/shaolins.o \
	src/vidhrdw/pingpong.o \
	src/drivers/pingpong.o \
	src/vidhrdw/gberet.o \
	src/drivers/gberet.o \
	src/vidhrdw/jailbrek.o \
	src/drivers/jailbrek.o \
	src/vidhrdw/ironhors.o \
	src/drivers/ironhors.o \
	src/machine/jackal.o \
	src/vidhrdw/jackal.o \
	src/drivers/jackal.o \
	src/vidhrdw/contra.o \
	src/drivers/contra.o \
	src/vidhrdw/mainevt.o \
	src/drivers/mainevt.o \
	src/machine/combatsc.o \
	src/vidhrdw/combatsc.o \
	src/drivers/combatsc.o \
	src/machine/ddrible.o \
	src/vidhrdw/ddrible.o \
	src/drivers/ddrible.o


OBJS_NEMESIS= \
	src/vidhrdw/nemesis.o \
	src/drivers/nemesis.o


OBJS_TMNT= \
	src/vidhrdw/tmnt.o \
	src/drivers/tmnt.o


OBJS_EXIDY= \
	src/machine/exidy.o \
	src/vidhrdw/exidy.o \
	src/sndhrdw/exidy.o \
	src/drivers/exidy.o \
	src/sndhrdw/targ.o \
	src/vidhrdw/circus.o \
	src/drivers/circus.o \
	src/machine/starfire.o \
	src/vidhrdw/starfire.o \
	src/drivers/starfire.o

OBJS_FOODF = \
	src/machine/foodf.o

OBJS_ATARIVG= \
	src/machine/atari_vg.o \
	src/machine/asteroid.o \
	src/sndhrdw/asteroid.o \
	src/vidhrdw/llander.o \
	src/drivers/asteroid.o \
	src/drivers/bwidow.o \
	src/sndhrdw/bzone.o \
	src/drivers/bzone.o \
	src/sndhrdw/redbaron.o \
	src/drivers/tempest.o \
	src/machine/starwars.o \
	src/machine/swmathbx.o \
	src/drivers/starwars.o \
	src/sndhrdw/starwars.o \
	src/machine/mhavoc.o \
	src/drivers/mhavoc.o \
	src/machine/quantum.o \
	src/drivers/quantum.o


OBJS_CENTIPEDE= \
	src/machine/centiped.o \
	src/vidhrdw/centiped.o \
	src/drivers/centiped.o \
	src/machine/milliped.o \
	src/vidhrdw/milliped.o \
	src/drivers/milliped.o \
	src/vidhrdw/qwakprot.o \
	src/drivers/qwakprot.o \
	src/vidhrdw/warlord.o \
	src/drivers/warlord.o


OBJS_KANGAROO= \
	src/machine/kangaroo.o \
	src/vidhrdw/kangaroo.o \
	src/drivers/kangaroo.o \
	src/machine/arabian.o \
	src/vidhrdw/arabian.o \
	src/drivers/arabian.o


OBJS_MISSILE= \
	src/machine/missile.o \
	src/vidhrdw/missile.o \
	src/drivers/missile.o


OBJS_ATARIBW= \
	src/machine/sprint2.o \
	src/vidhrdw/sprint2.o \
	src/drivers/sprint2.o \
	src/machine/sbrkout.o \
	src/vidhrdw/sbrkout.o \
	src/drivers/sbrkout.o \
	src/machine/dominos.o \
	src/vidhrdw/dominos.o \
	src/drivers/dominos.o \
	src/vidhrdw/nitedrvr.o \
	src/machine/nitedrvr.o \
	src/drivers/nitedrvr.o \
	src/vidhrdw/bsktball.o \
	src/machine/bsktball.o \
	src/drivers/bsktball.o \
	src/vidhrdw/copsnrob.o \
	src/machine/copsnrob.o \
	src/drivers/copsnrob.o \
	src/machine/avalnche.o \
	src/vidhrdw/avalnche.o \
	src/drivers/avalnche.o \
	src/machine/subs.o \
	src/vidhrdw/subs.o \
	src/drivers/subs.o


OBJS_ATARISY1= \
	src/machine/atarisy1.o \
	src/vidhrdw/atarisy1.o \
	src/drivers/atarisy1.o


OBJS_ATARISY2= \
	src/machine/atarisy2.o \
	src/vidhrdw/atarisy2.o \
	src/drivers/atarisy2.o


OBJS_ATARI= \
	src/machine/gauntlet.o \
	src/vidhrdw/gauntlet.o \
	src/drivers/gauntlet.o \
	src/vidhrdw/atetris.o \
	src/drivers/atetris.o \
	src/machine/toobin.o \
	src/vidhrdw/toobin.o \
	src/drivers/toobin.o \
	src/vidhrdw/vindictr.o \
	src/drivers/vindictr.o \
	src/vidhrdw/klax.o \
	src/drivers/klax.o \
	src/machine/blstroid.o \
	src/vidhrdw/blstroid.o \
	src/drivers/blstroid.o \
	src/vidhrdw/eprom.o \
	src/drivers/eprom.o \
	src/vidhrdw/xybots.o \
	src/drivers/xybots.o


OBJS_CRTC6845= \
	src/vidhrdw/crtc6845.o

OBJS_ROCKOLA= \
	src/vidhrdw/rockola.o \
	src/sndhrdw/rockola.o \
	src/drivers/rockola.o \
	src/vidhrdw/warpwarp.o \
	src/sndhrdw/warpwarp.o \
	src/drivers/warpwarp.o


OBJS_TECHNOS= \
	src/vidhrdw/mystston.o \
	src/drivers/mystston.o \
	src/vidhrdw/matmania.o \
	src/drivers/matmania.o \
	src/vidhrdw/renegade.o \
	src/drivers/renegade.o \
	src/vidhrdw/xain.o \
	src/drivers/xain.o \
	src/vidhrdw/ddragon.o \
	src/drivers/ddragon.o \
	src/vidhrdw/blockout.o \
	src/drivers/blockout.o


OBJS_BERZERK= \
	src/machine/berzerk.o \
	src/vidhrdw/berzerk.o \
	src/sndhrdw/berzerk.o \
	src/drivers/berzerk.o


OBJS_GAMEPLAN= \
	src/vidhrdw/gameplan.o \
	src/drivers/gameplan.o


OBJS_STRATVOX= \
	src/vidhrdw/route16.o \
	src/drivers/route16.o


OBJS_ZACCARIA= \
	src/vidhrdw/zaccaria.o \
	src/drivers/zaccaria.o

OBJS_UPL= \
	src/vidhrdw/nova2001.o \
	src/drivers/nova2001.o \
	src/vidhrdw/pkunwar.o \
	src/drivers/pkunwar.o \
	src/vidhrdw/ninjakd2.o \
	src/drivers/ninjakd2.o


OBJS_TMS= \
	src/machine/exterm.o \
	src/vidhrdw/exterm.o \
	src/drivers/exterm.o \
	src/machine/smashtv.o \
	src/vidhrdw/smashtv.o \
	src/sndhrdw/smashtv.o \
	src/drivers/smashtv.o 


OBJS_CINEMAR= \
	src/vidhrdw/jack.o \
	src/drivers/jack.o


OBJS_THEPIT= \
	src/vidhrdw/thepit.o \
	src/drivers/thepit.o


OBJS_VALADON= \
	src/machine/bagman.o \
	src/vidhrdw/bagman.o \
	src/drivers/bagman.o


OBJS_SEIBU= \
	src/vidhrdw/wiz.o \
	src/drivers/wiz.o


OBJS_NICHIBUT= \
	src/vidhrdw/cop01.o \
	src/drivers/cop01.o \
	src/vidhrdw/terracre.o \
	src/drivers/terracre.o \
	src/vidhrdw/galivan.o \
	src/drivers/galivan.o\
	src/vidhrdw/armedf.o \
	src/drivers/armedf.o


OBJS_NEOGEO= \
	src/machine/neogeo.o \
	src/machine/pd4990a.o \
	src/vidhrdw/neogeo.o \
	src/drivers/neogeo.o


OBJS_OTHER= \
	src/machine/spacefb.o \
	src/vidhrdw/spacefb.o \
	src/sndhrdw/spacefb.o \
	src/drivers/spacefb.o \
	src/vidhrdw/tutankhm.o \
	src/drivers/tutankhm.o \
	src/drivers/junofrst.o \
	src/vidhrdw/ccastles.o \
	src/drivers/ccastles.o \
	src/vidhrdw/blueprnt.o \
	src/drivers/blueprnt.o \
	src/drivers/omegrace.o \
	src/vidhrdw/bankp.o \
	src/drivers/bankp.o \
	src/machine/espial.o \
	src/vidhrdw/espial.o \
	src/drivers/espial.o \
	src/machine/cloak.o \
	src/vidhrdw/cloak.o \
	src/drivers/cloak.o \
	src/vidhrdw/champbas.o \
	src/drivers/champbas.o \
	src/drivers/sinbadm.o \
	src/vidhrdw/exerion.o \
	src/drivers/exerion.o \
	src/machine/foodf.o \
	src/vidhrdw/foodf.o \
	src/drivers/foodf.o \
	src/machine/vastar.o \
	src/vidhrdw/vastar.o \
	src/drivers/vastar.o \
	src/vidhrdw/aeroboto.o \
	src/drivers/aeroboto.o \
	src/vidhrdw/citycon.o \
	src/drivers/citycon.o \
	src/vidhrdw/psychic5.o \
	src/drivers/psychic5.o \
	src/machine/jedi.o \
	src/vidhrdw/jedi.o \
	src/sndhrdw/jedi.o \
	src/drivers/jedi.o \
	src/vidhrdw/tankbatt.o \
	src/drivers/tankbatt.o \
	src/vidhrdw/liberatr.o \
	src/machine/liberatr.o \
	src/drivers/liberatr.o \
	src/vidhrdw/dday.o \
	src/sndhrdw/dday.o \
	src/drivers/dday.o \
	src/vidhrdw/toki.o \
	src/drivers/toki.o \
	src/vidhrdw/snowbros.o \
	src/drivers/snowbros.o \
	src/vidhrdw/gundealr.o \
	src/drivers/gundealr.o \
	src/machine/leprechn.o \
	src/vidhrdw/leprechn.o \
	src/drivers/leprechn.o \
	src/vidhrdw/hexa.o \
	src/drivers/hexa.o \
	src/vidhrdw/redalert.o \
	src/sndhrdw/redalert.o \
	src/drivers/redalert.o \
	src/machine/irobot.o \
	src/vidhrdw/irobot.o \
	src/drivers/irobot.o \
	src/machine/spiders.o \
	src/vidhrdw/crtc6845.o \
	src/vidhrdw/spiders.o \
	src/drivers/spiders.o \
	src/machine/stactics.o \
	src/vidhrdw/stactics.o \
	src/drivers/stactics.o \
	src/vidhrdw/goldstar.o \
	src/drivers/goldstar.o \
	src/vidhrdw/vigilant.o \
	src/drivers/vigilant.o \
	src/vidhrdw/sharkatt.o \
	src/drivers/sharkatt.o \
	src/machine/turbo.o \
	src/vidhrdw/turbo.o \
	src/drivers/turbo.o \
	src/vidhrdw/kingobox.o \
	src/drivers/kingobox.o \
	src/vidhrdw/zerozone.o \
	src/drivers/zerozone.o \
	src/machine/exctsccr.o \
	src/vidhrdw/exctsccr.o \
	src/drivers/exctsccr.o \
	src/vidhrdw/speedbal.o \
	src/drivers/speedbal.o \
	src/vidhrdw/sauro.o \
	src/drivers/sauro.o \
	src/vidhrdw/pow.o \
	src/drivers/pow.o \
	src/machine/kabuki.o \
	src/vidhrdw/pang.o \
	src/drivers/pang.o \
	src/vidhrdw/pcktgal.o \
	src/drivers/pcktgal.o \
	src/vidhrdw/snk.o \
	src/drivers/snk.o \
	src/vidhrdw/galpanic.o \
	src/drivers/galpanic.o \
	src/vidhrdw/aerofgt.o \
	src/drivers/aerofgt.o \
	src/vidhrdw/toaplan1.o \
	src/drivers/zerowing.o \
	src/vidhrdw/prehisle.o \
	src/drivers/prehisle.o \
	src/vidhrdw/megasys1.o \
	src/drivers/megasys1.o \
	src/vidhrdw/sichuan2.o \
	src/sndhrdw/sichuan2.o \
	src/drivers/sichuan2.o \
	src/vidhrdw/cabal.o \
	src/drivers/cabal.o \
	src/vidhrdw/goindol.o \
	src/drivers/goindol.o \
	src/vidhrdw/megazone.o \
	src/drivers/megazone.o \
	src/vidhrdw/troangel.o \
	src/drivers/troangel.o \
	src/vidhrdw/marineb.o \
	src/drivers/marineb.o \
	src/vidhrdw/canyon.o \
	src/drivers/canyon.o \
        src/vidhrdw/ambush.o \
	src/drivers/ambush.o \
	src/drivers/funkybee.o \
	src/vidhrdw/funkybee.o \
	src/drivers/pandoras.o \
	src/vidhrdw/pandoras.o \
	src/drivers/wardner.o

ifdef DRIVER_GENERIC_8BIT
ifndef DRIVER_OTHER
DRIVERS += $(OBJS_CRTC6845)
endif

DRIVERS += \
	$(OBJS_PACMAN) \
	$(OBJS_GALAXIAN) \
	$(OBJS_SCRAMBLE) \
	$(OBJS_CCLIMBER) \
	$(OBJS_PHOENIX) \
	$(OBJS_NAMCO) \
	$(OBJS_UNIVERS) \
	$(OBJS_NINTENDO) \
	$(OBJS_MIDW8080) \
	$(OBJS_MIDWZ80) \
	$(OBJS_MEADOWS) \
	$(OBJS_ASTROCDE) \
	$(OBJS_IREM) \
	$(OBJS_OLDTAITO) \
	$(OBJS_QIXTAITO) \
	$(OBJS_TAITO) \
	$(OBJS_WILLIAMS) \
	$(OBJS_CAPBOWL) \
	$(OBJS_GREMLIN) \
	$(OBJS_VICDUAL) \
	$(OBJS_SEGAV) \
	$(OBJS_SEGAR) \
	$(OBJS_ZAXXON) \
	$(OBJS_SYSTEM8) \
	$(OBJS_BTIME) \
	$(OBJS_DATAEAST) \
	$(OBJS_DEC8) \
	$(OBJS_KONAMI) \
	$(OBJS_EXIDY) \
	$(OBJS_KANGAROO) \
	$(OBJS_MISSILE) \
	$(OBJS_ATARIBW) \
	$(OBJS_ATARISY2) \
	$(OBJS_ROCKOLA) \
	$(OBJS_BERZERK) \
	$(OBJS_GAMEPLAN) \
	$(OBJS_STRATVOX) \
	$(OBJS_ZACCARIA) \
	$(OBJS_UPL) \
	$(OBJS_CINEMAR) \
	$(OBJS_THEPIT) \
	$(OBJS_GOTTLIEB) \
	$(OBJS_VALADON)
endif


ifdef DRIVER_TMS
DRIVERS += $(OBJS_TMS) 
ifndef DRIVER_GENERIC_8BIT
DRIVERS += $(OBJS_GOTTLIEB)
endif
endif

ifdef DRIVER_TMNT
DRIVERS += $(OBJS_TMNT) 
endif

ifdef DRIVER_SYSTEM16
DRIVERS += $(OBJS_SYSTEM16) 
endif

ifdef DRIVER_TAITO2
DRIVERS += $(OBJS_TAITO2)
endif

ifdef DRIVER_ATARIVG
DRIVERS += $(OBJS_ATARIVG)
ifndef DRIVER_OTHER
DRIVERS += $(OBJS_FOODF)
endif
ifndef DRIVER_GENERIC_8BIT
DRIVERS += $(OBJS_DEC8)
endif
ifdef DRIVER_CENTIPEDE
DRIVERS += $(OBJS_CENTIPEDE)
endif
endif

ifdef DRIVER_NEMESIS
DRIVERS += $(OBJS_NEMESIS)
endif

ifdef DRIVER_MCR
DRIVERS += $(OBJS_MCR) 
endif

ifdef DRIVER_TEHKAN
DRIVERS += $(OBJS_TEHKAN)
endif

ifdef DRIVER_DEC0
DRIVERS += $(OBJS_DEC0) 
endif

ifdef DRIVER_ATARI
DRIVERS += $(OBJS_ATARI) 
endif

ifdef DRIVER_TECHNOS
DRIVERS += $(OBJS_TECHNOS)
endif

ifdef DRIVER_CAPCOM
DRIVERS += $(OBJS_CAPCOM)
endif

ifdef DRIVER_CPS1
DRIVERS += $(OBJS_CPS1)
endif

ifdef DRIVER_ATARISY1
DRIVERS += $(OBJS_ATARISY1)
endif

ifdef DRIVER_SEIBU
DRIVERS += $(OBJS_SEIBU) 
endif

ifdef DRIVER_NICHIBUT
DRIVERS += $(OBJS_NICHIBUT) 
endif

ifdef DRIVER_OTHER
ifndef DRIVER_ATARIVG
DRIVERS += src/machine/atari_vg.o 
endif
ifndef DRIVER_GENERIC_8BIT
ifndef DRIVER_ATARIVG
DRIVERS += $(OBJS_DEC8)
endif
endif
DRIVERS += $(OBJS_OTHER)
ifndef DRIVER_GENERIC_8BIT
DRIVERS += $(OBJS_KONAMI)
#DRIVERS += $(OBJS_CRTC6845)
endif
endif



ifdef DRIVER_NEOGEO
DRIVERS += $(OBJS_NEOGEO)
endif

cpu: $(OBJS_CPU)

drivers: $(DRIVERS)

parts: $(DRIVERS) $(OBJS_CPU)


clean-drivers:
	$(RM) $(DRIVERS)

clean-cpu:
	$(RM) $(OBJS_CPU)

clean-config:
	$(RM) src/driver.o src/cpuintrf.o src/*/minimal.o src/*/sound.o src/mame.o src/vidhrdw/wow.o src/sndhrdw/generic.o src/sndhrdw/adpcm.o src/sndhrdw/fm.o src/fame/m68000.o src/cz80/cz80.o

clean-all: clean clean-drivers clean-cpu 
	find src -name "*.o" -exec rm {} \;

