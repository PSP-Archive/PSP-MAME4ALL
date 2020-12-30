#include "cpudefs.h"
void op_d000(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
	m68k_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d010(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d018(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	m68k_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
	m68k_regs.d[dstreg].B.l = newv;
}}}}}}}}
void op_d020(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
	m68k_regs.d[dstreg].B.l = newv;
}}}}}}}}
void op_d028(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d030(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d038(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d039(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d03a(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d03b(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	BYTE src = get_byte(srca);
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d03c(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = nextiword();
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	m68k_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d040(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d048(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.a[srcreg];
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d050(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d058(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	m68k_regs.a[srcreg] += 2;
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d060(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 2;
{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d068(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d070(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d078(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d079(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d07a(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d07b(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	WORD src = get_word(srca);
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d07c(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = m68k_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	m68k_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d080(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d088(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.a[srcreg];
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d090(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d098(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	m68k_regs.a[srcreg] += 4;
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0a0(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 4;
{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0a8(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0b0(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0b8(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0b9(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0ba(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0bb(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0bc(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = m68k_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	m68k_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0c0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d0c8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.a[srcreg];
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d0d0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d0d8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	m68k_regs.a[srcreg] += 2;
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d0e0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 2;
{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d0e8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d0f0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d0f8(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d0f9(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d0fa(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d0fb(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d0fc(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d100(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	BYTE dst = m68k_regs.d[dstreg].B.l;
{	ULONG newv = dst + src + (m68k_regs.x ? 1 : 0);
	m68k_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	m68k_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
}}}}}}

void op_d108(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	m68k_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
	m68k_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = dst + src + (m68k_regs.x ? 1 : 0);
	put_byte(dsta,newv);
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
{	int flgn = ((BYTE)(newv)) < 0;
	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	m68k_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
}}}}

void op_d110(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d118(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	BYTE src = m68k_regs.d[srcreg].B.l;
	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	m68k_regs.a[dstreg] += areg_byteinc[dstreg];
{	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgn = ((BYTE)(newv)) < 0;
	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}

void op_d120(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	m68k_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d128(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = m68k_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d130(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(m68k_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d138(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d139(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d140(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	WORD dst = m68k_regs.d[dstreg].W.l;
{	ULONG newv = dst + src + (m68k_regs.x ? 1 : 0);
	m68k_regs.d[dstreg].W.l = newv;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	m68k_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((WORD)(newv)) != 0) ZFLG = 0;
	NFLG = ((WORD)(newv)) < 0;
}}}}}}
void op_d148(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 2;
{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	m68k_regs.a[dstreg] -= 2;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	ULONG newv = dst + src + (m68k_regs.x ? 1 : 0);
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	m68k_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((WORD)(newv)) != 0) ZFLG = 0;
	NFLG = ((WORD)(newv)) < 0;
}}}}}}}}
void op_d150(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d158(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	m68k_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d160(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	m68k_regs.a[dstreg] -= 2;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d168(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = m68k_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d170(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(m68k_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d178(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d179(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d180(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	LONG dst = m68k_regs.d[dstreg].D;
{	ULONG newv = dst + src + (m68k_regs.x ? 1 : 0);
	m68k_regs.d[dstreg].D = (newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	m68k_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((LONG)(newv)) != 0) ZFLG = 0;
	NFLG = ((LONG)(newv)) < 0;
}}}}}}
void op_d188(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 4;
{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	m68k_regs.a[dstreg] -= 4;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	ULONG newv = dst + src + (m68k_regs.x ? 1 : 0);
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	m68k_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((LONG)(newv)) != 0) ZFLG = 0;
	NFLG = ((LONG)(newv)) < 0;
}}}}}}}}
void op_d190(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d198(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	m68k_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d1a0(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	m68k_regs.a[dstreg] -= 4;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d1a8(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = m68k_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d1b0(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(m68k_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d1b8(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d1b9(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = m68k_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d1c0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d1c8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.a[srcreg];
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d1d0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d1d8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	m68k_regs.a[srcreg] += 4;
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d1e0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 4;
{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d1e8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d1f0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d1f8(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d1f9(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
void op_d1fa(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d1fb(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}}
void op_d1fc(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = m68k_regs.a[dstreg];
{	ULONG newv = dst + src;
	m68k_regs.a[dstreg] = (newv);
}}}}}
