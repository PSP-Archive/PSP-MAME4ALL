#include "cpudefs.h"
void op_b000(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = m68k_regs.d[srcreg].B.l;
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b010(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b018(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
	m68k_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_b020(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	m68k_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_b028(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b030(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b038(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b039(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b03a(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_b03b(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(m68k_getpc());
	BYTE src = get_byte(srca);
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b03c(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	BYTE dst = m68k_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b040(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	WORD src = m68k_regs.d[srcreg].W.l;
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b048(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	WORD src = m68k_regs.a[srcreg];
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b050(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b058(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
	m68k_regs.a[srcreg] += 2;
{	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_b060(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	m68k_regs.a[srcreg] -= 2;
{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_b068(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b070(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b078(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b079(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b07a(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_b07b(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(m68k_getpc());
	WORD src = get_word(srca);
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b07c(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	WORD src = nextiword();
	WORD dst = m68k_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b080(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = m68k_regs.d[srcreg].D;
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b088(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = m68k_regs.a[srcreg];
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b090(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b098(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
	m68k_regs.a[srcreg] += 4;
{	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_b0a0(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	m68k_regs.a[srcreg] -= 4;
{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_b0a8(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0b0(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0b8(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0b9(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0ba(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_b0bb(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(m68k_getpc());
	LONG src = get_long(srca);
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0bc(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	LONG dst = m68k_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0c0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0c8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = m68k_regs.a[srcreg];
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0d0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0d8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	m68k_regs.a[srcreg] += 2;
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0e0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 2;
{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0e8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0f0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0f8(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0f9(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0fa(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0fb(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	WORD src = get_word(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0fc(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b100(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	BYTE dst = m68k_regs.d[dstreg].B.l;
	src ^= dst;
	m68k_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b108(void) /* CMPM */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	m68k_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	m68k_regs.a[dstreg] += areg_byteinc[dstreg];
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}}
void op_b110(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b118(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	m68k_regs.a[dstreg] += areg_byteinc[dstreg];
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b120(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	m68k_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = m68k_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b128(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = m68k_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b130(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(m68k_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b138(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b139(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = m68k_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b140(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	WORD dst = m68k_regs.d[dstreg].W.l;
	src ^= dst;
	m68k_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b148(void) /* CMPM */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	WORD src = get_word(srca);
{	m68k_regs.a[srcreg] += 2;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	m68k_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}}
void op_b150(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b158(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	m68k_regs.a[dstreg] += 2;
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b160(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	m68k_regs.a[dstreg] -= 2;
{	CPTR dsta = m68k_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b168(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = m68k_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b170(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(m68k_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b178(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b179(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = m68k_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b180(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	LONG dst = m68k_regs.d[dstreg].D;
	src ^= dst;
	m68k_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b188(void) /* CMPM */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	m68k_regs.a[srcreg] += 4;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	m68k_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}}
void op_b190(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b198(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	m68k_regs.a[dstreg] += 4;
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b1a0(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	m68k_regs.a[dstreg] -= 4;
{	CPTR dsta = m68k_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b1a8(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = m68k_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b1b0(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(m68k_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b1b8(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b1b9(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = m68k_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b1c0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.d[srcreg].D;
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1c8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = m68k_regs.a[srcreg];
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1d0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1d8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	m68k_regs.a[srcreg] += 4;
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1e0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	m68k_regs.a[srcreg] -= 4;
{	CPTR srca = m68k_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1e8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1f0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1f8(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1f9(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1fa(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = m68k_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1fb(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(m68k_getpc());
{	LONG src = get_long(srca);
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1fc(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = m68k_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
