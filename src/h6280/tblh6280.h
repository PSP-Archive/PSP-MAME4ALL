/*****************************************************************************

    tblh6280.cpp

	Copyright (c) 1999 Bryan McPhail, mish@tendril.force9.net

	This source code is based (with permission!) on the 6502 emulator by
	Juergen Buchmueller.  It is released as part of the Mame emulator project.
	Let me know if you intend to use this code in any other project.

******************************************************************************/

#undef	OP
#define OP(nnn) INLINE void h6280_##nnn(void)

/*****************************************************************************
 *****************************************************************************
 *
 *	 Hu6280 opcodes
 *
 *****************************************************************************
 * op	  temp	   cycles		      rdmem	  opc   wrmem   ******************/
OP(000) {          H6280_ICount -= 8; 	      BRK;		   } /* 8 BRK */
OP(020) {          H6280_ICount -= 7; EA_ABS; JSR;		   } /* 7 JSR  ABS */
OP(040) {          H6280_ICount -= 7;         RTI;		   } /* 7 RTI */
OP(060) {          H6280_ICount -= 7;         RTS;		   } /* 7 RTS */
OP(080) { int tmp;  				          BRA(1);	   } /* 4 BRA  REL */
OP(0a0) { int tmp; H6280_ICount -= 2; RD_IMM; LDY;		   } /* 2 LDY  IMM */
OP(0c0) { int tmp; H6280_ICount -= 2; RD_IMM; CPY;		   } /* 2 CPY  IMM */
OP(0e0) { int tmp; H6280_ICount -= 2; RD_IMM; CPX;		   } /* 2 CPX  IMM */

OP(010) { int tmp;							  BPL;		   } /* 2/4 BPL  REL */
OP(030) { int tmp;							  BMI;		   } /* 2/4 BMI  REL */
OP(050) { int tmp;							  BVC;		   } /* 2/4 BVC  REL */
OP(070) { int tmp;							  BVS;		   } /* 2/4 BVS  REL */
OP(090) { int tmp;							  BCC;		   } /* 2/4 BCC  REL */
OP(0b0) { int tmp;							  BCS;		   } /* 2/4 BCS  REL */
OP(0d0) { int tmp;							  BNE;		   } /* 2/4 BNE  REL */
OP(0f0) { int tmp;							  BEQ;		   } /* 2/4 BEQ  REL */

OP(001) { int tmp; H6280_ICount -= 7; RD_IDX; ORA;		   } /* 7 ORA  IDX */
OP(021) { int tmp; H6280_ICount -= 7; RD_IDX; AND;		   } /* 7 AND  IDX */
OP(041) { int tmp; H6280_ICount -= 7; RD_IDX; EOR;		   } /* 7 EOR  IDX */
OP(061) { int tmp; H6280_ICount -= 7; RD_IDX; ADC;		   } /* 7 ADC  IDX */
OP(081) { int tmp; H6280_ICount -= 7; RD_IDX; STA;		   } /* 7 STA  IDX */
OP(0a1) { int tmp; H6280_ICount -= 7; RD_IDX; LDA;		   } /* 7 LDA  IDX */
OP(0c1) { int tmp; H6280_ICount -= 7; RD_IDX; CMP;		   } /* 7 CMP  IDX */
OP(0e1) { int tmp; H6280_ICount -= 7; RD_IDX; SBC;		   } /* 7 SBC  IDX */

OP(011) { int tmp; H6280_ICount -= 7; RD_IDY; ORA;		   } /* 7 ORA  IDY */
OP(031) { int tmp; H6280_ICount -= 7; RD_IDY; AND;		   } /* 7 AND  IDY */
OP(051) { int tmp; H6280_ICount -= 7; RD_IDY; EOR;		   } /* 7 EOR  IDY */
OP(071) { int tmp; H6280_ICount -= 7; RD_IDY; ADC;		   } /* 7 ADC  AZP */
OP(091) { int tmp; H6280_ICount -= 7;         STA; WR_IDY; } /* 7 STA  IDY */
OP(0b1) { int tmp; H6280_ICount -= 7; RD_IDY; LDA;		   } /* 7 LDA  IDY */
OP(0d1) { int tmp; H6280_ICount -= 7; RD_IDY; CMP;		   } /* 7 CMP  IDY */
OP(0f1) { int tmp; H6280_ICount -= 7; RD_IDY; SBC;		   } /* 7 SBC  IDY */

OP(002) { int tmp; H6280_ICount -= 3;		  SXY;		   } /* 3 SXY */
OP(022) { int tmp; H6280_ICount -= 3;		  SAX;		   } /* 3 SAX */
OP(042) { int tmp; H6280_ICount -= 3;		  SAY;		   } /* 3 SAY */
OP(062) {          H6280_ICount -= 2;		  CLA;		   } /* 2 CLA */
OP(082) {          H6280_ICount -= 2;		  CLX;		   } /* 2 CLX */
OP(0a2) { int tmp; H6280_ICount -= 2; RD_IMM; LDX;		   } /* 2 LDX  IMM */
OP(0c2) {          H6280_ICount -= 2;		  CLY;		   } /* 2 CLY */
OP(0e2) {									  ILL;		   } /* 2 ??? */

OP(012) { int tmp; H6280_ICount -= 7; RD_ZPI; ORA;		   } /* 7 ORA  ZPI */
OP(032) { int tmp; H6280_ICount -= 7; RD_ZPI; AND;		   } /* 7 AND  ZPI */
OP(052) { int tmp; H6280_ICount -= 7; RD_ZPI; EOR;		   } /* 7 EOR  ZPI */
OP(072) { int tmp; H6280_ICount -= 7; RD_ZPI; ADC;		   } /* 7 ADC  ZPI */
OP(092) { int tmp; H6280_ICount -= 7;		  STA; WR_ZPI; } /* 7 STA  ZPI */
OP(0b2) { int tmp; H6280_ICount -= 7; RD_ZPI; LDA;		   } /* 7 LDA  ZPI */
OP(0d2) { int tmp; H6280_ICount -= 7; RD_ZPI; CMP;		   } /* 7 CMP  ZPI */
OP(0f2) { int tmp; H6280_ICount -= 7; RD_ZPI; SBC;		   } /* 7 SBC  ZPI */

OP(003) { int tmp; H6280_ICount -= 4; RD_IMM; ST0;		   } /* 4 ST0  IMM */
OP(023) { int tmp; H6280_ICount -= 4; RD_IMM; ST2;		   } /* 4 ST2  IMM */
OP(043) { int tmp; H6280_ICount -= 4; RD_IMM; TMA;		   } /* 4 TMA */
OP(063) {									  ILL;		   } /* 2 ??? */
OP(083) { int tmp; H6280_ICount -= 7; RD_IMM; EA_ZPG; TST; } /* 7 TST  IMM,ZPG */
OP(0a3) { int tmp; H6280_ICount -= 7; RD_IMM; EA_ZPX; TST; } /* 7 TST  IMM,ZPX */
OP(0c3) { int to,from,length;			      TDD;		   } /* 6*l+17 TDD  XFER */
OP(0e3) { int to,from,length,alternate;       TIA;		   } /* 6*l+17 TIA  XFER */

OP(013) { int tmp; H6280_ICount -= 4; RD_IMM; ST1;		   } /* 4 ST1 */
OP(033) {       							  ILL;		   } /* 2 ??? */
OP(053) { int tmp; H6280_ICount -= 5; RD_IMM; TAM;		   } /* 5 TAM  IMM */
OP(073) { int to,from,length;    			  TII;		   } /* 6*l+17 TII  XFER */
OP(093) { int tmp; H6280_ICount -= 8; RD_IMM; EA_ABS; TST; } /* 8 TST  IMM,ABS */
OP(0b3) { int tmp; H6280_ICount -= 8; RD_IMM; EA_ABX; TST; } /* 8 TST  IMM,ABX */
OP(0d3) { int to,from,length;			      TIN;		   } /* 6*l+17 TIN  XFER */
OP(0f3) { int to,from,length,alternate;       TAI;		   } /* 6*l+17 TAI  XFER */

OP(004) { int tmp; H6280_ICount -= 6; RD_ZPG; TSB; WB_EAZ; } /* 6 TSB  ZPG */
OP(024) { int tmp; H6280_ICount -= 4; RD_ZPG; BIT;		   } /* 4 BIT  ZPG */
OP(044) { int tmp;							  BSR;		   } /* 8 BSR  REL */
OP(064) { int tmp; H6280_ICount -= 4;		  STZ; WR_ZPG; } /* 4 STZ  ZPG */
OP(084) { int tmp; H6280_ICount -= 4;		  STY; WR_ZPG; } /* 4 STY  ZPG */
OP(0a4) { int tmp; H6280_ICount -= 4; RD_ZPG; LDY;		   } /* 4 LDY  ZPG */
OP(0c4) { int tmp; H6280_ICount -= 4; RD_ZPG; CPY;		   } /* 4 CPY  ZPG */
OP(0e4) { int tmp; H6280_ICount -= 4; RD_ZPG; CPX;		   } /* 4 CPX  ZPG */

OP(014) { int tmp; H6280_ICount -= 6; RD_ZPG; TRB; WB_EAZ; } /* 6 TRB  ZPG */
OP(034) { int tmp; H6280_ICount -= 4; RD_ZPX; BIT;		   } /* 4 BIT  ZPX */
OP(054) {												   } /* 2 CSL */
OP(074) { int tmp; H6280_ICount -= 4;		  STZ; WR_ZPX; } /* 4 STZ  ZPX */
OP(094) { int tmp; H6280_ICount -= 4;		  STY; WR_ZPX; } /* 4 STY  ZPX */
OP(0b4) { int tmp; H6280_ICount -= 4; RD_ZPX; LDY;		   } /* 4 LDY  ZPX */
OP(0d4) {												   } /* 2 CSH */
OP(0f4) {          H6280_ICount -= 2; 		  SET;		   } /* 2 SET */

OP(005) { int tmp; H6280_ICount -= 4; RD_ZPG; ORA;		   } /* 4 ORA  ZPG */
OP(025) { int tmp; H6280_ICount -= 4; RD_ZPG; AND;		   } /* 4 AND  ZPG */
OP(045) { int tmp; H6280_ICount -= 4; RD_ZPG; EOR;		   } /* 4 EOR  ZPG */
OP(065) { int tmp; H6280_ICount -= 4; RD_ZPG; ADC;		   } /* 4 ADC  ZPG */
OP(085) { int tmp; H6280_ICount -= 4;		  STA; WR_ZPG; } /* 4 STA  ZPG */
OP(0a5) { int tmp; H6280_ICount -= 4; RD_ZPG; LDA;		   } /* 4 LDA  ZPG */
OP(0c5) { int tmp; H6280_ICount -= 4; RD_ZPG; CMP;		   } /* 4 CMP  ZPG */
OP(0e5) { int tmp; H6280_ICount -= 4; RD_ZPG; SBC;		   } /* 4 SBC  ZPG */

OP(015) { int tmp; H6280_ICount -= 4; RD_ZPX; ORA;		   } /* 4 ORA  ZPX */
OP(035) { int tmp; H6280_ICount -= 4; RD_ZPX; AND;		   } /* 4 AND  ZPX */
OP(055) { int tmp; H6280_ICount -= 4; RD_ZPX; EOR;		   } /* 4 EOR  ZPX */
OP(075) { int tmp; H6280_ICount -= 4; RD_ZPX; ADC;		   } /* 4 ADC  ZPX */
OP(095) { int tmp; H6280_ICount -= 4;		  STA; WR_ZPX; } /* 4 STA  ZPX */
OP(0b5) { int tmp; H6280_ICount -= 4; RD_ZPX; LDA;		   } /* 4 LDA  ZPX */
OP(0d5) { int tmp; H6280_ICount -= 4; RD_ZPX; CMP;		   } /* 4 CMP  ZPX */
OP(0f5) { int tmp; H6280_ICount -= 4; RD_ZPX; SBC;		   } /* 4 SBC  ZPX */

OP(006) { int tmp; H6280_ICount -= 6; RD_ZPG; ASL; WB_EAZ; } /* 6 ASL  ZPG */
OP(026) { int tmp; H6280_ICount -= 6; RD_ZPG; ROL; WB_EAZ; } /* 6 ROL  ZPG */
OP(046) { int tmp; H6280_ICount -= 6; RD_ZPG; LSR; WB_EAZ; } /* 6 LSR  ZPG */
OP(066) { int tmp; H6280_ICount -= 6; RD_ZPG; ROR; WB_EAZ; } /* 6 ROR  ZPG */
OP(086) { int tmp; H6280_ICount -= 4;		  STX; WR_ZPG; } /* 4 STX  ZPG */
OP(0a6) { int tmp; H6280_ICount -= 4; RD_ZPG; LDX;		   } /* 4 LDX  ZPG */
OP(0c6) { int tmp; H6280_ICount -= 6; RD_ZPG; DEC; WB_EAZ; } /* 6 DEC  ZPG */
OP(0e6) { int tmp; H6280_ICount -= 6; RD_ZPG; INC; WB_EAZ; } /* 6 INC  ZPG */

OP(016) { int tmp; H6280_ICount -= 6; RD_ZPX; ASL; WB_EAZ  } /* 6 ASL  ZPX */
OP(036) { int tmp; H6280_ICount -= 6; RD_ZPX; ROL; WB_EAZ  } /* 6 ROL  ZPX */
OP(056) { int tmp; H6280_ICount -= 6; RD_ZPX; LSR; WB_EAZ  } /* 6 LSR  ZPX */
OP(076) { int tmp; H6280_ICount -= 6; RD_ZPX; ROR; WB_EAZ  } /* 6 ROR  ZPX */
OP(096) { int tmp; H6280_ICount -= 4;		  STX; WR_ZPY; } /* 4 STX  ZPY */
OP(0b6) { int tmp; H6280_ICount -= 4; RD_ZPY; LDX;		   } /* 4 LDX  ZPY */
OP(0d6) { int tmp; H6280_ICount -= 6; RD_ZPX; DEC; WB_EAZ; } /* 6 DEC  ZPX */
OP(0f6) { int tmp; H6280_ICount -= 6; RD_ZPX; INC; WB_EAZ; } /* 6 INC  ZPX */

OP(007) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(0);WB_EAZ;} /* 7 RMB0 ZPG */
OP(027) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(2);WB_EAZ;} /* 7 RMB2 ZPG */
OP(047) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(4);WB_EAZ;} /* 7 RMB4 ZPG */
OP(067) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(6);WB_EAZ;} /* 7 RMB6 ZPG */
OP(087) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(0);WB_EAZ;} /* 7 SMB0 ZPG */
OP(0a7) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(2);WB_EAZ;} /* 7 SMB2 ZPG */
OP(0c7) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(4);WB_EAZ;} /* 7 SMB4 ZPG */
OP(0e7) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(6);WB_EAZ;} /* 7 SMB6 ZPG */

OP(017) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(1);WB_EAZ;} /* 7 RMB1 ZPG */
OP(037) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(3);WB_EAZ;} /* 7 RMB3 ZPG */
OP(057) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(5);WB_EAZ;} /* 7 RMB5 ZPG */
OP(077) { int tmp; H6280_ICount -= 7; RD_ZPG; RMB(7);WB_EAZ;} /* 7 RMB7 ZPG */
OP(097) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(1);WB_EAZ;} /* 7 SMB1 ZPG */
OP(0b7) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(3);WB_EAZ;} /* 7 SMB3 ZPG */
OP(0d7) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(5);WB_EAZ;} /* 7 SMB5 ZPG */
OP(0f7) { int tmp; H6280_ICount -= 7; RD_ZPG; SMB(7);WB_EAZ;} /* 7 SMB7 ZPG */

OP(008) {          H6280_ICount -= 3; 		  PHP;		   } /* 3 PHP */
OP(028) {          H6280_ICount -= 4;		  PLP;		   } /* 4 PLP */
OP(048) {          H6280_ICount -= 3;		  PHA;		   } /* 3 PHA */
OP(068) {          H6280_ICount -= 4;		  PLA;		   } /* 4 PLA */
OP(088) {          H6280_ICount -= 2;		  DEY;		   } /* 2 DEY */
OP(0a8) {          H6280_ICount -= 2;		  TAY;		   } /* 2 TAY */
OP(0c8) {          H6280_ICount -= 2;		  INY;		   } /* 2 INY */
OP(0e8) {          H6280_ICount -= 2;		  INX;		   } /* 2 INX */

OP(018) {          H6280_ICount -= 2;		  CLC;		   } /* 2 CLC */
OP(038) {          H6280_ICount -= 2;		  SEC;		   } /* 2 SEC */
OP(058) {          H6280_ICount -= 2;		  CLI;		   } /* 2 CLI */
OP(078) {          H6280_ICount -= 2;		  SEI;		   } /* 2 SEI */
OP(098) {          H6280_ICount -= 2;		  TYA;		   } /* 2 TYA */
OP(0b8) {          H6280_ICount -= 2;		  CLV;		   } /* 2 CLV */
OP(0d8) {          H6280_ICount -= 2;		  CLD;		   } /* 2 CLD */
OP(0f8) {          H6280_ICount -= 2;		  SED;		   } /* 2 SED */

OP(009) { int tmp; H6280_ICount -= 2; RD_IMM; ORA;		   } /* 2 ORA  IMM */
OP(029) { int tmp; H6280_ICount -= 2; RD_IMM; AND;		   } /* 2 AND  IMM */
OP(049) { int tmp; H6280_ICount -= 2; RD_IMM; EOR;		   } /* 2 EOR  IMM */
OP(069) { int tmp; H6280_ICount -= 2; RD_IMM; ADC;		   } /* 2 ADC  IMM */
OP(089) { int tmp; H6280_ICount -= 2; RD_IMM; BIT;		   } /* 2 BIT  IMM */
OP(0a9) { int tmp; H6280_ICount -= 2; RD_IMM; LDA;		   } /* 2 LDA  IMM */
OP(0c9) { int tmp; H6280_ICount -= 2; RD_IMM; CMP;		   } /* 2 CMP  IMM */
OP(0e9) { int tmp; H6280_ICount -= 2; RD_IMM; SBC;		   } /* 2 SBC  IMM */

OP(019) { int tmp; H6280_ICount -= 5; RD_ABY; ORA;		   } /* 5 ORA  ABY */
OP(039) { int tmp; H6280_ICount -= 5; RD_ABY; AND;		   } /* 5 AND  ABY */
OP(059) { int tmp; H6280_ICount -= 5; RD_ABY; EOR;		   } /* 5 EOR  ABY */
OP(079) { int tmp; H6280_ICount -= 5; RD_ABY; ADC;		   } /* 5 ADC  ABY */
OP(099) { int tmp; H6280_ICount -= 5;		  STA; WR_ABY; } /* 5 STA  ABY */
OP(0b9) { int tmp; H6280_ICount -= 5; RD_ABY; LDA;		   } /* 5 LDA  ABY */
OP(0d9) { int tmp; H6280_ICount -= 5; RD_ABY; CMP;		   } /* 5 CMP  ABY */
OP(0f9) { int tmp; H6280_ICount -= 5; RD_ABY; SBC;		   } /* 5 SBC  ABY */

OP(00a) { int tmp; H6280_ICount -= 2; RD_ACC; ASL; WB_ACC; } /* 2 ASL  A */
OP(02a) { int tmp; H6280_ICount -= 2; RD_ACC; ROL; WB_ACC; } /* 2 ROL  A */
OP(04a) { int tmp; H6280_ICount -= 2; RD_ACC; LSR; WB_ACC; } /* 2 LSR  A */
OP(06a) { int tmp; H6280_ICount -= 2; RD_ACC; ROR; WB_ACC; } /* 2 ROR  A */
OP(08a) {          H6280_ICount -= 2;		  TXA;		   } /* 2 TXA */
OP(0aa) {          H6280_ICount -= 2;		  TAX;		   } /* 2 TAX */
OP(0ca) {          H6280_ICount -= 2;		  DEX;		   } /* 2 DEX */
OP(0ea) {          H6280_ICount -= 2;		  NOP;		   } /* 2 NOP */

OP(01a) {          H6280_ICount -= 2;		  INA;		   } /* 2 INC  A */
OP(03a) {          H6280_ICount -= 2;		  DEA;		   } /* 2 DEC  A */
OP(05a) {          H6280_ICount -= 3;		  PHY;		   } /* 3 PHY */
OP(07a) {          H6280_ICount -= 4;		  PLY;		   } /* 4 PLY */
OP(09a) {          H6280_ICount -= 2;		  TXS;		   } /* 2 TXS */
OP(0ba) {          H6280_ICount -= 2;		  TSX;		   } /* 2 TSX */
OP(0da) {          H6280_ICount -= 3;		  PHX;		   } /* 3 PHX */
OP(0fa) {          H6280_ICount -= 4;		  PLX;		   } /* 4 PLX */

OP(00b) {									  ILL;		   } /* 2 ??? */
OP(02b) {									  ILL;		   } /* 2 ??? */
OP(04b) {									  ILL;		   } /* 2 ??? */
OP(06b) {									  ILL;		   } /* 2 ??? */
OP(08b) {									  ILL;		   } /* 2 ??? */
OP(0ab) {									  ILL;		   } /* 2 ??? */
OP(0cb) {									  ILL;		   } /* 2 ??? */
OP(0eb) {									  ILL;		   } /* 2 ??? */

OP(01b) {									  ILL;		   } /* 2 ??? */
OP(03b) {									  ILL;		   } /* 2 ??? */
OP(05b) {									  ILL;		   } /* 2 ??? */
OP(07b) {									  ILL;		   } /* 2 ??? */
OP(09b) {									  ILL;		   } /* 2 ??? */
OP(0bb) {									  ILL;		   } /* 2 ??? */
OP(0db) {									  ILL;		   } /* 2 ??? */
OP(0fb) {									  ILL;		   } /* 2 ??? */

OP(00c) { int tmp; H6280_ICount -= 7; RD_ABS; TSB; WB_EA;  } /* 7 TSB  ABS */
OP(02c) { int tmp; H6280_ICount -= 5; RD_ABS; BIT;		   } /* 5 BIT  ABS */
OP(04c) {          H6280_ICount -= 4; EA_ABS; JMP;		   } /* 4 JMP  ABS */
OP(06c) { int tmp; H6280_ICount -= 7; EA_IND; JMP;		   } /* 7 JMP  IND */
OP(08c) { int tmp; H6280_ICount -= 5;		  STY; WR_ABS; } /* 5 STY  ABS */
OP(0ac) { int tmp; H6280_ICount -= 5; RD_ABS; LDY;		   } /* 5 LDY  ABS */
OP(0cc) { int tmp; H6280_ICount -= 5; RD_ABS; CPY;		   } /* 5 CPY  ABS */
OP(0ec) { int tmp; H6280_ICount -= 5; RD_ABS; CPX;		   } /* 5 CPX  ABS */

OP(01c) { int tmp; H6280_ICount -= 7; RD_ABS; TRB; WB_EA;  } /* 7 TRB  ABS */
OP(03c) { int tmp; H6280_ICount -= 5; RD_ABX; BIT;		   } /* 5 BIT  ABX */
OP(05c) {									  ILL;		   } /* 2 ??? */
OP(07c) { int tmp; H6280_ICount -= 7; EA_IAX; JMP;		   } /* 7 JMP  IAX */
OP(09c) { int tmp; H6280_ICount -= 5;		  STZ; WR_ABS; } /* 5 STZ  ABS */
OP(0bc) { int tmp; H6280_ICount -= 5; RD_ABX; LDY;		   } /* 5 LDY  ABX */
OP(0dc) {									  ILL;		   } /* 2 ??? */
OP(0fc) {									  ILL;		   } /* 2 ??? */

OP(00d) { int tmp; H6280_ICount -= 5; RD_ABS; ORA;		   } /* 5 ORA  ABS */
OP(02d) { int tmp; H6280_ICount -= 5; RD_ABS; AND;		   } /* 4 AND  ABS */
OP(04d) { int tmp; H6280_ICount -= 5; RD_ABS; EOR;		   } /* 4 EOR  ABS */
OP(06d) { int tmp; H6280_ICount -= 5; RD_ABS; ADC;		   } /* 4 ADC  ABS */
OP(08d) { int tmp; H6280_ICount -= 5;		  STA; WR_ABS; } /* 4 STA  ABS */
OP(0ad) { int tmp; H6280_ICount -= 5; RD_ABS; LDA;		   } /* 4 LDA  ABS */
OP(0cd) { int tmp; H6280_ICount -= 5; RD_ABS; CMP;		   } /* 4 CMP  ABS */
OP(0ed) { int tmp; H6280_ICount -= 5; RD_ABS; SBC;		   } /* 4 SBC  ABS */

OP(01d) { int tmp; H6280_ICount -= 5; RD_ABX; ORA;		   } /* 5 ORA  ABX */
OP(03d) { int tmp; H6280_ICount -= 5; RD_ABX; AND;		   } /* 4 AND  ABX */
OP(05d) { int tmp; H6280_ICount -= 5; RD_ABX; EOR;		   } /* 4 EOR  ABX */
OP(07d) { int tmp; H6280_ICount -= 5; RD_ABX; ADC;		   } /* 4 ADC  ABX */
OP(09d) { int tmp; H6280_ICount -= 5;		  STA; WR_ABX; } /* 5 STA  ABX */
OP(0bd) { int tmp; H6280_ICount -= 5; RD_ABX; LDA;		   } /* 5 LDA  ABX */
OP(0dd) { int tmp; H6280_ICount -= 5; RD_ABX; CMP;		   } /* 4 CMP  ABX */
OP(0fd) { int tmp; H6280_ICount -= 5; RD_ABX; SBC;		   } /* 4 SBC  ABX */

OP(00e) { int tmp; H6280_ICount -= 7; RD_ABS; ASL; WB_EA;  } /* 6 ASL  ABS */
OP(02e) { int tmp; H6280_ICount -= 7; RD_ABS; ROL; WB_EA;  } /* 6 ROL  ABS */
OP(04e) { int tmp; H6280_ICount -= 7; RD_ABS; LSR; WB_EA;  } /* 6 LSR  ABS */
OP(06e) { int tmp; H6280_ICount -= 7; RD_ABS; ROR; WB_EA;  } /* 6 ROR  ABS */
OP(08e) { int tmp; H6280_ICount -= 5;		  STX; WR_ABS; } /* 4 STX  ABS */
OP(0ae) { int tmp; H6280_ICount -= 5; RD_ABS; LDX;		   } /* 5 LDX  ABS */
OP(0ce) { int tmp; H6280_ICount -= 7; RD_ABS; DEC; WB_EA;  } /* 6 DEC  ABS */
OP(0ee) { int tmp; H6280_ICount -= 7; RD_ABS; INC; WB_EA;  } /* 6 INC  ABS */

OP(01e) { int tmp; H6280_ICount -= 7; RD_ABX; ASL; WB_EA;  } /* 7 ASL  ABX */
OP(03e) { int tmp; H6280_ICount -= 7; RD_ABX; ROL; WB_EA;  } /* 7 ROL  ABX */
OP(05e) { int tmp; H6280_ICount -= 7; RD_ABX; LSR; WB_EA;  } /* 7 LSR  ABX */
OP(07e) { int tmp; H6280_ICount -= 7; RD_ABX; ROR; WB_EA;  } /* 7 ROR  ABX */
OP(09e) { int tmp; H6280_ICount -= 5;		  STZ; WR_ABX; } /* 5 STZ  ABX */
OP(0be) { int tmp; H6280_ICount -= 5; RD_ABY; LDX;		   } /* 4 LDX  ABY */
OP(0de) { int tmp; H6280_ICount -= 7; RD_ABX; DEC; WB_EA;  } /* 7 DEC  ABX */
OP(0fe) { int tmp; H6280_ICount -= 7; RD_ABX; INC; WB_EA;  } /* 7 INC  ABX */

OP(00f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(0);	   } /* 6/8 BBR0 ZPG,REL */
OP(02f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(2);	   } /* 6/8 BBR2 ZPG,REL */
OP(04f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(4);	   } /* 6/8 BBR4 ZPG,REL */
OP(06f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(6);	   } /* 6/8 BBR6 ZPG,REL */
OP(08f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(0);	   } /* 6/8 BBS0 ZPG,REL */
OP(0af) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(2);	   } /* 6/8 BBS2 ZPG,REL */
OP(0cf) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(4);	   } /* 6/8 BBS4 ZPG,REL */
OP(0ef) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(6);	   } /* 6/8 BBS6 ZPG,REL */

OP(01f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(1);	   } /* 6/8 BBR1 ZPG,REL */
OP(03f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(3);	   } /* 6/8 BBR3 ZPG,REL */
OP(05f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(5);	   } /* 6/8 BBR5 ZPG,REL */
OP(07f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBR(7);	   } /* 6/8 BBR7 ZPG,REL */
OP(09f) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(1);	   } /* 6/8 BBS1 ZPG,REL */
OP(0bf) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(3);	   } /* 6/8 BBS3 ZPG,REL */
OP(0df) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(5);	   } /* 6/8 BBS5 ZPG,REL */
OP(0ff) { int tmp; H6280_ICount -= 4; RD_ZPG; BBS(7);	   } /* 6/8 BBS7 ZPG,REL */


#ifdef H6280_BIG_SWITCH

#define H6280_SWITCH \
	switch(in) { \
		case 0x00: h6280_000(); break; \
		case 0x01: h6280_001(); break; \
		case 0x02: h6280_002(); break; \
		case 0x03: h6280_003(); break; \
		case 0x04: h6280_004(); break; \
		case 0x05: h6280_005(); break; \
		case 0x06: h6280_006(); break; \
		case 0x07: h6280_007(); break; \
		case 0x08: h6280_008(); break; \
		case 0x09: h6280_009(); break; \
		case 0x0a: h6280_00a(); break; \
		case 0x0b: h6280_00b(); break; \
		case 0x0c: h6280_00c(); break; \
		case 0x0d: h6280_00d(); break; \
		case 0x0e: h6280_00e(); break; \
		case 0x0f: h6280_00f(); break; \
		case 0x10: h6280_010(); break; \
		case 0x11: h6280_011(); break; \
		case 0x12: h6280_012(); break; \
		case 0x13: h6280_013(); break; \
		case 0x14: h6280_014(); break; \
		case 0x15: h6280_015(); break; \
		case 0x16: h6280_016(); break; \
		case 0x17: h6280_017(); break; \
		case 0x18: h6280_018(); break; \
		case 0x19: h6280_019(); break; \
		case 0x1a: h6280_01a(); break; \
		case 0x1b: h6280_01b(); break; \
		case 0x1c: h6280_01c(); break; \
		case 0x1d: h6280_01d(); break; \
		case 0x1e: h6280_01e(); break; \
		case 0x1f: h6280_01f(); break; \
		case 0x20: h6280_020(); break; \
		case 0x21: h6280_021(); break; \
		case 0x22: h6280_022(); break; \
		case 0x23: h6280_023(); break; \
		case 0x24: h6280_024(); break; \
		case 0x25: h6280_025(); break; \
		case 0x26: h6280_026(); break; \
		case 0x27: h6280_027(); break; \
		case 0x28: h6280_028(); break; \
		case 0x29: h6280_029(); break; \
		case 0x2a: h6280_02a(); break; \
		case 0x2b: h6280_02b(); break; \
		case 0x2c: h6280_02c(); break; \
		case 0x2d: h6280_02d(); break; \
		case 0x2e: h6280_02e(); break; \
		case 0x2f: h6280_02f(); break; \
		case 0x30: h6280_030(); break; \
		case 0x31: h6280_031(); break; \
		case 0x32: h6280_032(); break; \
		case 0x33: h6280_033(); break; \
		case 0x34: h6280_034(); break; \
		case 0x35: h6280_035(); break; \
		case 0x36: h6280_036(); break; \
		case 0x37: h6280_037(); break; \
		case 0x38: h6280_038(); break; \
		case 0x39: h6280_039(); break; \
		case 0x3a: h6280_03a(); break; \
		case 0x3b: h6280_03b(); break; \
		case 0x3c: h6280_03c(); break; \
		case 0x3d: h6280_03d(); break; \
		case 0x3e: h6280_03e(); break; \
		case 0x3f: h6280_03f(); break; \
		case 0x40: h6280_040(); break; \
		case 0x41: h6280_041(); break; \
		case 0x42: h6280_042(); break; \
		case 0x43: h6280_043(); break; \
		case 0x44: h6280_044(); break; \
		case 0x45: h6280_045(); break; \
		case 0x46: h6280_046(); break; \
		case 0x47: h6280_047(); break; \
		case 0x48: h6280_048(); break; \
		case 0x49: h6280_049(); break; \
		case 0x4a: h6280_04a(); break; \
		case 0x4b: h6280_04b(); break; \
		case 0x4c: h6280_04c(); break; \
		case 0x4d: h6280_04d(); break; \
		case 0x4e: h6280_04e(); break; \
		case 0x4f: h6280_04f(); break; \
		case 0x50: h6280_050(); break; \
		case 0x51: h6280_051(); break; \
		case 0x52: h6280_052(); break; \
		case 0x53: h6280_053(); break; \
		case 0x54: h6280_054(); break; \
		case 0x55: h6280_055(); break; \
		case 0x56: h6280_056(); break; \
		case 0x57: h6280_057(); break; \
		case 0x58: h6280_058(); break; \
		case 0x59: h6280_059(); break; \
		case 0x5a: h6280_05a(); break; \
		case 0x5b: h6280_05b(); break; \
		case 0x5c: h6280_05c(); break; \
		case 0x5d: h6280_05d(); break; \
		case 0x5e: h6280_05e(); break; \
		case 0x5f: h6280_05f(); break; \
		case 0x60: h6280_060(); break; \
		case 0x61: h6280_061(); break; \
		case 0x62: h6280_062(); break; \
		case 0x63: h6280_063(); break; \
		case 0x64: h6280_064(); break; \
		case 0x65: h6280_065(); break; \
		case 0x66: h6280_066(); break; \
		case 0x67: h6280_067(); break; \
		case 0x68: h6280_068(); break; \
		case 0x69: h6280_069(); break; \
		case 0x6a: h6280_06a(); break; \
		case 0x6b: h6280_06b(); break; \
		case 0x6c: h6280_06c(); break; \
		case 0x6d: h6280_06d(); break; \
		case 0x6e: h6280_06e(); break; \
		case 0x6f: h6280_06f(); break; \
		case 0x70: h6280_070(); break; \
		case 0x71: h6280_071(); break; \
		case 0x72: h6280_072(); break; \
		case 0x73: h6280_073(); break; \
		case 0x74: h6280_074(); break; \
		case 0x75: h6280_075(); break; \
		case 0x76: h6280_076(); break; \
		case 0x77: h6280_077(); break; \
		case 0x78: h6280_078(); break; \
		case 0x79: h6280_079(); break; \
		case 0x7a: h6280_07a(); break; \
		case 0x7b: h6280_07b(); break; \
		case 0x7c: h6280_07c(); break; \
		case 0x7d: h6280_07d(); break; \
		case 0x7e: h6280_07e(); break; \
		case 0x7f: h6280_07f(); break; \
		case 0x80: h6280_080(); break; \
		case 0x81: h6280_081(); break; \
		case 0x82: h6280_082(); break; \
		case 0x83: h6280_083(); break; \
		case 0x84: h6280_084(); break; \
		case 0x85: h6280_085(); break; \
		case 0x86: h6280_086(); break; \
		case 0x87: h6280_087(); break; \
		case 0x88: h6280_088(); break; \
		case 0x89: h6280_089(); break; \
		case 0x8a: h6280_08a(); break; \
		case 0x8b: h6280_08b(); break; \
		case 0x8c: h6280_08c(); break; \
		case 0x8d: h6280_08d(); break; \
		case 0x8e: h6280_08e(); break; \
		case 0x8f: h6280_08f(); break; \
		case 0x90: h6280_090(); break; \
		case 0x91: h6280_091(); break; \
		case 0x92: h6280_092(); break; \
		case 0x93: h6280_093(); break; \
		case 0x94: h6280_094(); break; \
		case 0x95: h6280_095(); break; \
		case 0x96: h6280_096(); break; \
		case 0x97: h6280_097(); break; \
		case 0x98: h6280_098(); break; \
		case 0x99: h6280_099(); break; \
		case 0x9a: h6280_09a(); break; \
		case 0x9b: h6280_09b(); break; \
		case 0x9c: h6280_09c(); break; \
		case 0x9d: h6280_09d(); break; \
		case 0x9e: h6280_09e(); break; \
		case 0x9f: h6280_09f(); break; \
		case 0xa0: h6280_0a0(); break; \
		case 0xa1: h6280_0a1(); break; \
		case 0xa2: h6280_0a2(); break; \
		case 0xa3: h6280_0a3(); break; \
		case 0xa4: h6280_0a4(); break; \
		case 0xa5: h6280_0a5(); break; \
		case 0xa6: h6280_0a6(); break; \
		case 0xa7: h6280_0a7(); break; \
		case 0xa8: h6280_0a8(); break; \
		case 0xa9: h6280_0a9(); break; \
		case 0xaa: h6280_0aa(); break; \
		case 0xab: h6280_0ab(); break; \
		case 0xac: h6280_0ac(); break; \
		case 0xad: h6280_0ad(); break; \
		case 0xae: h6280_0ae(); break; \
		case 0xaf: h6280_0af(); break; \
		case 0xb0: h6280_0b0(); break; \
		case 0xb1: h6280_0b1(); break; \
		case 0xb2: h6280_0b2(); break; \
		case 0xb3: h6280_0b3(); break; \
		case 0xb4: h6280_0b4(); break; \
		case 0xb5: h6280_0b5(); break; \
		case 0xb6: h6280_0b6(); break; \
		case 0xb7: h6280_0b7(); break; \
		case 0xb8: h6280_0b8(); break; \
		case 0xb9: h6280_0b9(); break; \
		case 0xba: h6280_0ba(); break; \
		case 0xbb: h6280_0bb(); break; \
		case 0xbc: h6280_0bc(); break; \
		case 0xbd: h6280_0bd(); break; \
		case 0xbe: h6280_0be(); break; \
		case 0xbf: h6280_0bf(); break; \
		case 0xc0: h6280_0c0(); break; \
		case 0xc1: h6280_0c1(); break; \
		case 0xc2: h6280_0c2(); break; \
		case 0xc3: h6280_0c3(); break; \
		case 0xc4: h6280_0c4(); break; \
		case 0xc5: h6280_0c5(); break; \
		case 0xc6: h6280_0c6(); break; \
		case 0xc7: h6280_0c7(); break; \
		case 0xc8: h6280_0c8(); break; \
		case 0xc9: h6280_0c9(); break; \
		case 0xca: h6280_0ca(); break; \
		case 0xcb: h6280_0cb(); break; \
		case 0xcc: h6280_0cc(); break; \
		case 0xcd: h6280_0cd(); break; \
		case 0xce: h6280_0ce(); break; \
		case 0xcf: h6280_0cf(); break; \
		case 0xd0: h6280_0d0(); break; \
		case 0xd1: h6280_0d1(); break; \
		case 0xd2: h6280_0d2(); break; \
		case 0xd3: h6280_0d3(); break; \
		case 0xd4: h6280_0d4(); break; \
		case 0xd5: h6280_0d5(); break; \
		case 0xd6: h6280_0d6(); break; \
		case 0xd7: h6280_0d7(); break; \
		case 0xd8: h6280_0d8(); break; \
		case 0xd9: h6280_0d9(); break; \
		case 0xda: h6280_0da(); break; \
		case 0xdb: h6280_0db(); break; \
		case 0xdc: h6280_0dc(); break; \
		case 0xdd: h6280_0dd(); break; \
		case 0xde: h6280_0de(); break; \
		case 0xdf: h6280_0df(); break; \
		case 0xe0: h6280_0e0(); break; \
		case 0xe1: h6280_0e1(); break; \
		case 0xe2: h6280_0e2(); break; \
		case 0xe3: h6280_0e3(); break; \
		case 0xe4: h6280_0e4(); break; \
		case 0xe5: h6280_0e5(); break; \
		case 0xe6: h6280_0e6(); break; \
		case 0xe7: h6280_0e7(); break; \
		case 0xe8: h6280_0e8(); break; \
		case 0xe9: h6280_0e9(); break; \
		case 0xea: h6280_0ea(); break; \
		case 0xeb: h6280_0eb(); break; \
		case 0xec: h6280_0ec(); break; \
		case 0xed: h6280_0ed(); break; \
		case 0xee: h6280_0ee(); break; \
		case 0xef: h6280_0ef(); break; \
		case 0xf0: h6280_0f0(); break; \
		case 0xf1: h6280_0f1(); break; \
		case 0xf2: h6280_0f2(); break; \
		case 0xf3: h6280_0f3(); break; \
		case 0xf4: h6280_0f4(); break; \
		case 0xf5: h6280_0f5(); break; \
		case 0xf6: h6280_0f6(); break; \
		case 0xf7: h6280_0f7(); break; \
		case 0xf8: h6280_0f8(); break; \
		case 0xf9: h6280_0f9(); break; \
		case 0xfa: h6280_0fa(); break; \
		case 0xfb: h6280_0fb(); break; \
		case 0xfc: h6280_0fc(); break; \
		case 0xfd: h6280_0fd(); break; \
		case 0xfe: h6280_0fe(); break; \
		case 0xff: h6280_0ff(); break; \
		default: break; \
	}

#else

static void (*insnh6280[0x100])(void) = {
	h6280_000,h6280_001,h6280_002,h6280_003,h6280_004,h6280_005,h6280_006,h6280_007,
	h6280_008,h6280_009,h6280_00a,h6280_00b,h6280_00c,h6280_00d,h6280_00e,h6280_00f,
	h6280_010,h6280_011,h6280_012,h6280_013,h6280_014,h6280_015,h6280_016,h6280_017,
	h6280_018,h6280_019,h6280_01a,h6280_01b,h6280_01c,h6280_01d,h6280_01e,h6280_01f,
	h6280_020,h6280_021,h6280_022,h6280_023,h6280_024,h6280_025,h6280_026,h6280_027,
	h6280_028,h6280_029,h6280_02a,h6280_02b,h6280_02c,h6280_02d,h6280_02e,h6280_02f,
	h6280_030,h6280_031,h6280_032,h6280_033,h6280_034,h6280_035,h6280_036,h6280_037,
	h6280_038,h6280_039,h6280_03a,h6280_03b,h6280_03c,h6280_03d,h6280_03e,h6280_03f,
	h6280_040,h6280_041,h6280_042,h6280_043,h6280_044,h6280_045,h6280_046,h6280_047,
	h6280_048,h6280_049,h6280_04a,h6280_04b,h6280_04c,h6280_04d,h6280_04e,h6280_04f,
	h6280_050,h6280_051,h6280_052,h6280_053,h6280_054,h6280_055,h6280_056,h6280_057,
	h6280_058,h6280_059,h6280_05a,h6280_05b,h6280_05c,h6280_05d,h6280_05e,h6280_05f,
	h6280_060,h6280_061,h6280_062,h6280_063,h6280_064,h6280_065,h6280_066,h6280_067,
	h6280_068,h6280_069,h6280_06a,h6280_06b,h6280_06c,h6280_06d,h6280_06e,h6280_06f,
	h6280_070,h6280_071,h6280_072,h6280_073,h6280_074,h6280_075,h6280_076,h6280_077,
	h6280_078,h6280_079,h6280_07a,h6280_07b,h6280_07c,h6280_07d,h6280_07e,h6280_07f,
	h6280_080,h6280_081,h6280_082,h6280_083,h6280_084,h6280_085,h6280_086,h6280_087,
	h6280_088,h6280_089,h6280_08a,h6280_08b,h6280_08c,h6280_08d,h6280_08e,h6280_08f,
	h6280_090,h6280_091,h6280_092,h6280_093,h6280_094,h6280_095,h6280_096,h6280_097,
	h6280_098,h6280_099,h6280_09a,h6280_09b,h6280_09c,h6280_09d,h6280_09e,h6280_09f,
	h6280_0a0,h6280_0a1,h6280_0a2,h6280_0a3,h6280_0a4,h6280_0a5,h6280_0a6,h6280_0a7,
	h6280_0a8,h6280_0a9,h6280_0aa,h6280_0ab,h6280_0ac,h6280_0ad,h6280_0ae,h6280_0af,
	h6280_0b0,h6280_0b1,h6280_0b2,h6280_0b3,h6280_0b4,h6280_0b5,h6280_0b6,h6280_0b7,
	h6280_0b8,h6280_0b9,h6280_0ba,h6280_0bb,h6280_0bc,h6280_0bd,h6280_0be,h6280_0bf,
	h6280_0c0,h6280_0c1,h6280_0c2,h6280_0c3,h6280_0c4,h6280_0c5,h6280_0c6,h6280_0c7,
	h6280_0c8,h6280_0c9,h6280_0ca,h6280_0cb,h6280_0cc,h6280_0cd,h6280_0ce,h6280_0cf,
	h6280_0d0,h6280_0d1,h6280_0d2,h6280_0d3,h6280_0d4,h6280_0d5,h6280_0d6,h6280_0d7,
	h6280_0d8,h6280_0d9,h6280_0da,h6280_0db,h6280_0dc,h6280_0dd,h6280_0de,h6280_0df,
	h6280_0e0,h6280_0e1,h6280_0e2,h6280_0e3,h6280_0e4,h6280_0e5,h6280_0e6,h6280_0e7,
	h6280_0e8,h6280_0e9,h6280_0ea,h6280_0eb,h6280_0ec,h6280_0ed,h6280_0ee,h6280_0ef,
	h6280_0f0,h6280_0f1,h6280_0f2,h6280_0f3,h6280_0f4,h6280_0f5,h6280_0f6,h6280_0f7,
	h6280_0f8,h6280_0f9,h6280_0fa,h6280_0fb,h6280_0fc,h6280_0fd,h6280_0fe,h6280_0ff
};

#endif
