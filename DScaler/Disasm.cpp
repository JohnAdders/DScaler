/////////////////////////////////////////////////////////////////////////////
// $Id: Disasm.cpp,v 1.2 2001-07-27 16:11:32 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// This file was taken from VirtualDub
// VirtualDub - Video processing and capture application
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2001/07/24 12:19:00  adcockj
// Added code and tools for crash logging from VirtualDub
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "disasm.h"

#define MAX_INSTRUCTIONS (1024)

// WARNING: This is called from crash-time conditions!  No malloc() or new!!!

#define malloc not_allowed_here
#define new not_allowed_here

// Also, we keep as much of our initialized data as possible as const.  That way,
// it is in a write-locked code segment, which can't be overwritten.

void CCodeDisassemblyWindow::setFaultAddress(void *_pFault) 
{
	pFault = _pFault;
}


CCodeDisassemblyWindow::CCodeDisassemblyWindow(void *_code, long _length, void *_rbaseptr, void *_abaseptr)
                                            : code(_code)
                                            , rbase(_rbaseptr)
                                            , abase(_abaseptr)
                                            , length(_length)
                                            , pFault(0)
{
    lbents = (lbent *)VirtualAlloc(NULL, sizeof(lbent)*MAX_INSTRUCTIONS, MEM_COMMIT, PAGE_READWRITE);

    parse();

    hFontMono = CreateFont(
                            10,             // nHeight
                            0,              // nWidth
                            0,              // nEscapement
                            0,              // nOrientation
                            FW_DONTCARE,    // fnWeight
                            FALSE,          // fdwItalic
                            FALSE,          // fdwUnderline
                            FALSE,          // fdwStrikeOut
                            ANSI_CHARSET,   // fdwCharSet
                            OUT_DEFAULT_PRECIS,     // fdwOutputPrecision
                            CLIP_DEFAULT_PRECIS,    // fdwClipPrecision
                            DEFAULT_QUALITY,        // fdwQuality
                            DEFAULT_PITCH | FF_DONTCARE,    // fdwPitchAndFamily
                            "Lucida Console"
                           );

    if (!hFontMono)
    {
        hFontMono = (HFONT)GetStockObject(ANSI_FIXED_FONT);
    }
}

CCodeDisassemblyWindow::~CCodeDisassemblyWindow()
{
    if (hFontMono) 
    {
        DeleteObject(hFontMono);
    }

    if (lbents)
    {
        VirtualFree(lbents, 0, MEM_RELEASE);
    }
}

/////////////////////////////////////////////////////////////////////////

#define ADDR1(am) ((am)<<6)
#define ADDR2(am1,am2) ((am1) | ((am2)<<6))
#define ADDR3(am1,am2,am3) ((am1) | ((am2)<<6) | ((am3)<<6))
#define MOD(t) ((t)<<18)
#define TYPE(t) ((t)<<20)
#define GROUP(grp) ((grp)<<26)

#define GETADDR1(fl) ((fl) & 63)
#define GETADDR2(fl) (((fl)>>6) & 63)
#define GETADDR3(fl) (((fl)>>12) & 63)
#define GETMOD(fl) (((fl)>>18) & 3)
#define GETTYPE(fl) (((fl)>>20) & 63)
#define GETGROUP(fl) (((fl)>>26) & 63)

enum {
    ADDR0_MASK      =0x0000003fL,
    ADDR1_MASK      =0x00000fc0L,
    ADDR2_MASK      =0x0003f000L,
    MOD16_MASK      =0x000c0000L,
    TYPE_MASK       =0x03f00000L,
    GROUP_MASK      =0xfc000000L,

    ADDR_AL         =1,
    ADDR_CL,
    ADDR_DX,
    ADDR_EAX,

    ADDR_CS,
    ADDR_DS,
    ADDR_SS,
    ADDR_ES,
    ADDR_FS,
    ADDR_GS,

    ADDR_1,
    ADDR_Ap,    // direct address
    ADDR_Cd,    // reg of mod r/m selects control register
    ADDR_Dd,    // reg of mod r/m selects debug register
    ADDR_Eb,    // mod r/m specifies register or address
    ADDR_Ed,
    ADDR_Ep,
    ADDR_Eq,
    ADDR_Ev,
    ADDR_Ex,
    ADDR_Ew,
    ADDR_Gb,    // reg of mod r/m selects register or memory operand
    ADDR_Gv,
    ADDR_Gw,
    ADDR_Ib,    // immediate
    ADDR_Ib2,
    ADDR_Iv,
    ADDR_Iw,
    ADDR_Jb,    // relative offset
    ADDR_Jv,
    ADDR_M,     // reg of mod r/m selects memory
    ADDR_Ma,
    ADDR_Mp,
    ADDR_Ob,    // offset coded directly
    ADDR_Ow,
    ADDR_Od,
    ADDR_Oq,
    ADDR_Ov,
    ADDR_Pd,    // reg of mod r/m selects packed MMX register
    ADDR_Pq,
    ADDR_Pq2,   // INTEL DOCERR: p-shift instrs in group A have reg in bits 0-3 of mod r/m
    ADDR_Qd,    // mod r/m byte selects MMX register or memory operand
    ADDR_Qq,
    ADDR_Rd,    // mod field of mod r/m selects register only
    ADDR_Sw,    // reg of mod r/m selects segment register
    ADDR_Xb,    // ds:[e]si memory operand
    ADDR_Xv,
    ADDR_Yb,    // es:[e]di memory operand
    ADDR_Yv,

    ADDR_Idep       =60,
    ADDR_Ibs        =61,
    ADDR_reg8       =62,
    ADDR_reg32      =63,

    TYPE_cs_override            =1,
    TYPE_ds_override            =2,
    TYPE_ss_override            =3,
    TYPE_es_override            =4,
    TYPE_fs_override            =5,
    TYPE_gs_override            =6,
    TYPE_operand_size_override  =7,
    TYPE_address_size_override  =8,
    TYPE_0f                     =9,
    TYPE_prefix                 =10,

    MOD16_take_d_off            =1,
    MOD16_d_to_w                =2,
    MOD16_dq_to_wd              =3,
};

#define SPECIAL_3DNOW       (0xFFFFFF00)

static const char op_aaa        []="aaa";
static const char op_aad        []="aad";
static const char op_aam        []="aam";
static const char op_aas        []="aas";
static const char op_adc        []="adc";
static const char op_add        []="add";
static const char op_and        []="and";
static const char op_arpl       []="arpl";
static const char op_bound      []="bound";
static const char op_bsf        []="bsf";
static const char op_bsr        []="bsr";
static const char op_bswap      []="bswap";
static const char op_bt         []="bt";
static const char op_btc        []="btc";
static const char op_btr        []="btr";
static const char op_bts        []="bts";
static const char op_call       []="call";
static const char op_cbw        []="cbw";
static const char op_cdq        []="cdq";
static const char op_clc        []="clc";
static const char op_cld        []="cld";
static const char op_cli        []="cli";
static const char op_clts       []="clts";
static const char op_cmc        []="cmc";

static const char op_cmovc      []="cmovc";
static const char op_cmovnc     []="cmovnc";
static const char op_cmovp      []="cmovp";
static const char op_cmovnp     []="cmovnp";
static const char op_cmovs      []="cmovs";
static const char op_cmovns     []="cmovns";
static const char op_cmovl      []="cmovl";
static const char op_cmovle     []="cmovle";
static const char op_cmovg      []="cmovg";
static const char op_cmovge     []="cmovge";
static const char op_cmovo      []="cmovo";
static const char op_cmovno     []="cmovno";
static const char op_cmovz      []="cmovz";
static const char op_cmovnz     []="cmovnz";
static const char op_cmova      []="cmova";
static const char op_cmovbe     []="cmovbe";

static const char op_cmp        []="cmp";
static const char op_cmpsb      []="cmpsb";
static const char op_cmpsd      []="cmpsd";
static const char op_cmpxchg    []="cmpxchg";
static const char op_cmpxchg8b  []="cmpxchg8b";
static const char op_cpuid      []="cpuid";
static const char op_daa        []="daa";
static const char op_das        []="das";
static const char op_dec        []="dec";
static const char op_div        []="div";
static const char op_emms       []="emms";
static const char op_enter      []="enter";

static const char op_fadd       []="fadd";
static const char op_faddp      []="faddp";
static const char op_fbld       []="fbld";
static const char op_fbstp      []="fbstp";
static const char op_fcmovb     []="fcmovb";
static const char op_fcmovbe    []="fcmovbe";
static const char op_fcmove     []="fcmove";
static const char op_fcmovnb    []="fcmovnb";
static const char op_fcmovnbe   []="fcmovnbe";
static const char op_fcmovne    []="fcmovne";
static const char op_fcmovnu    []="fcmovnu";
static const char op_fcmovu     []="fcmovu";
static const char op_fcom       []="fcom";
static const char op_fcomi      []="fcomi";
static const char op_fcomip     []="fcomip";
static const char op_fcomp      []="fcomp";
static const char op_fdiv       []="fdiv";
static const char op_fdivp      []="fdivp";
static const char op_fdivr      []="fdivr";
static const char op_fdivrp     []="fdivrp";
static const char op_ffree      []="ffree";
static const char op_fiadd      []="fiadd";
static const char op_ficom      []="ficom";
static const char op_ficomp     []="ficomp";
static const char op_fidiv      []="fidiv";
static const char op_fidivr     []="fidivr";
static const char op_fild       []="fild";
static const char op_fimul      []="fimul";
static const char op_fist       []="fist";
static const char op_fistp      []="fistp";
static const char op_fisub      []="fisub";
static const char op_fisubr     []="fisubr";
static const char op_fld        []="fld";
static const char op_fldcw      []="fldcw";
static const char op_fldenv     []="fldenv";
static const char op_fmul       []="fmul";
static const char op_fmulp      []="fmulp";
static const char op_frstor     []="frstor";
static const char op_fsave      []="fsave";
static const char op_fst        []="fst";
static const char op_fstcw      []="fstcw";
static const char op_fstenv     []="fstenv";
static const char op_fstp       []="fstp";
static const char op_fstsw      []="fstsw";
static const char op_fsub       []="fsub";
static const char op_fsubp      []="fsubp";
static const char op_fsubr      []="fsubr";
static const char op_fsubrp     []="fsubrp";
static const char op_fucom      []="fucom";
static const char op_fucomi     []="fucomi";
static const char op_fucomip    []="fucomip";
static const char op_fucomp     []="fucomp";
static const char op_fxch       []="fxch";

static const char op_hlt        []="hlt";
static const char op_idiv       []="idiv";
static const char op_imul       []="imul";
static const char op_inc        []="inc";
static const char op_in         []="in";
static const char op_int        []="int";
static const char op_int3       []="int 3";
static const char op_into       []="into";
static const char op_insb       []="insb";
static const char op_insd       []="insd";
static const char op_invalid    []="invalid";
static const char op_invd       []="invd";
static const char op_invlpg     []="invlpg";
static const char op_iret       []="iret";

static const char op_jc         []="jc";
static const char op_jnc        []="jnc";
static const char op_jp         []="jp";
static const char op_jnp        []="jnp";
static const char op_js         []="js";
static const char op_jns        []="jns";
static const char op_jl         []="jl";
static const char op_jle        []="jle";
static const char op_jg         []="jg";
static const char op_jge        []="jge";
static const char op_jo         []="jo";
static const char op_jno        []="jno";
static const char op_jz         []="jz";
static const char op_jnz        []="jnz";
static const char op_ja         []="ja";
static const char op_jbe        []="jbe";

static const char op_jecxz      []="jecxz";
static const char op_jmp        []="jmp";
static const char op_lahf       []="lahf";
static const char op_lar        []="lar";
static const char op_lds        []="lds";
static const char op_lea        []="lea";
static const char op_leave      []="leave";
static const char op_les        []="les";
static const char op_lfs        []="lfs";
static const char op_lgdt       []="lgdt";
static const char op_lgs        []="lgs";
static const char op_lidt       []="lidt";
static const char op_lldt       []="lldt";
static const char op_lmsw       []="lmsw";
static const char op_lock       []="lock";
static const char op_lodsb      []="lodsb";
static const char op_lodsd      []="lodsd";
static const char op_loop       []="loop";
static const char op_loope      []="loope";
static const char op_loopn      []="loopn";
static const char op_lsl        []="lsl";
static const char op_lss        []="lss";
static const char op_ltr        []="ltr";
static const char op_mov        []="mov";
static const char op_movd       []="movd";
static const char op_movq       []="movq";
static const char op_movsb      []="movsb";
static const char op_movsd      []="movsd";
static const char op_movsx      []="movsx";
static const char op_movzx      []="movzx";
static const char op_mul        []="mul";
static const char op_neg        []="neg";
static const char op_nop        []="nop";
static const char op_not        []="not";
static const char op_or         []="or";
static const char op_out        []="out";
static const char op_outsb      []="outsb";
static const char op_outsd      []="outsd";
static const char op_packssdw   []="packssdw";
static const char op_packsswb   []="packsswb";
static const char op_packusdw   []="packusdw";
static const char op_paddb      []="paddb";
static const char op_paddd      []="paddd";
static const char op_paddw      []="paddw";
static const char op_paddsb     []="paddsb";
static const char op_paddsd     []="paddsd";
static const char op_paddsw     []="paddsw";
static const char op_paddusb    []="paddusb";
static const char op_paddusd    []="paddusd";
static const char op_paddusw    []="paddusw";
static const char op_pand       []="pand";
static const char op_pandn      []="pandn";
static const char op_pavgusb    []="pavgusb";
static const char op_pcmpeqb    []="pcmpeqb";
static const char op_pcmpeqd    []="pcmpeqd";
static const char op_pcmpeqw    []="pcmpeqw";
static const char op_pcmpgtb    []="pcmpgtb";
static const char op_pcmpgtd    []="pcmpgtd";
static const char op_pcmpgtw    []="pcmpgtw";

static const char op_pmaddwd    []="pmaddwd";
static const char op_pmulhw     []="pmulhw";
static const char op_pmullw     []="pmullw";
static const char op_pop        []="pop";
static const char op_popad      []="popad";
static const char op_popfd      []="popfd";
static const char op_por        []="por";
static const char op_pslld      []="pslld";
static const char op_psllq      []="psllq";
static const char op_psllw      []="psllw";
static const char op_psrad      []="psrad";
static const char op_psraw      []="psraw";
static const char op_psrld      []="psrld";
static const char op_psrlq      []="psrlq";
static const char op_psrlw      []="psrlw";
static const char op_psubb      []="psubb";
static const char op_psubd      []="psubd";
static const char op_psubw      []="psubw";
static const char op_psubsb     []="psubsb";
static const char op_psubsd     []="psubsd";
static const char op_psubsw     []="psubsw";
static const char op_psubusb    []="psubusb";
static const char op_psubusd    []="psubusd";
static const char op_psubusw    []="psubusw";
static const char op_punpckhbw  []="punpckhbw";
static const char op_punpckhdq  []="punpckhdq";
static const char op_punpckhwd  []="punpckhwd";
static const char op_punpcklbw  []="punpcklbw";
static const char op_punpckldq  []="punpckldq";
static const char op_punpcklwd  []="punpcklwd";
static const char op_push       []="push";
static const char op_pushad     []="pushad";
static const char op_pushfd     []="pushfd";
static const char op_pxor       []="pxor";
static const char op_rcl        []="rcl";
static const char op_rcr        []="rcr";
static const char op_rdmsr      []="rdmsr";
static const char op_rdpmc      []="rdpmc";
static const char op_rdtsc      []="rdtsc";
static const char op_rep        []="rep";
static const char op_repne      []="repne";
static const char op_ret        []="ret";
static const char op_retf       []="retf";
static const char op_rol        []="rol";
static const char op_ror        []="ror";
static const char op_rsm        []="rsm";
static const char op_sahf       []="sahf";
static const char op_sar        []="sar";
static const char op_sbb        []="sbb";
static const char op_scasb      []="scasb";
static const char op_scasd      []="scasd";

static const char op_setc   []="setc";
static const char op_setnc  []="setnc";
static const char op_setp   []="setp";
static const char op_setnp  []="setnp";
static const char op_sets   []="sets";
static const char op_setns  []="setns";
static const char op_setl   []="setl";
static const char op_setle  []="setle";
static const char op_setg   []="setg";
static const char op_setge  []="setge";
static const char op_seto   []="seto";
static const char op_setno  []="setno";
static const char op_setz   []="setz";
static const char op_setnz  []="setnz";
static const char op_seta   []="seta";
static const char op_setbe  []="setbe";

static const char op_shl    []="shl";
static const char op_shld   []="shld";
static const char op_shr    []="shr";
static const char op_shrd   []="shrd";
static const char op_sgdt   []="sgdt";
static const char op_sidt   []="sidt";
static const char op_sldt   []="sldt";
static const char op_smsw   []="smsw";
static const char op_stc    []="stc";
static const char op_std    []="std";
static const char op_sti    []="sti";
static const char op_str    []="str";
static const char op_stosb  []="stosb";
static const char op_stosd  []="stosd";
static const char op_sub    []="sub";
static const char op_test   []="test";
static const char op_ud2    []="ud2";
static const char op_verr   []="verr";
static const char op_verw   []="verw";
static const char op_wait   []="wait";
static const char op_wbinvd []="wbinvd";
static const char op_wrmsr  []="wrmsr";
static const char op_xadd   []="xadd";
static const char op_xchg   []="xchg";
static const char op_xlat   []="xlat";
static const char op_xor    []="xor";

// AMD 3DNow! instructions

static const char op_femms      []="femms";
static const char op_pfadd      []="pfadd";
static const char op_pfsub      []="pfsub";
static const char op_pfsubr     []="pfsubr";
static const char op_pfacc      []="pfacc";
static const char op_pfmul      []="pfmul";
static const char op_pfcmpge    []="pfcmpge";
static const char op_pfcmpgt    []="pfcmpgt";
static const char op_pfcmpeq    []="pfcmpeq";
static const char op_pfmin      []="pfmin";
static const char op_pfmax      []="pfmax";
static const char op_pi2fd      []="pi2fd";
static const char op_pf2id      []="pf2id";
static const char op_pfrcp      []="pfrcp";
static const char op_pfrsqrt    []="pfrsqrt";
static const char op_pfrcpit1   []="pfrcpit1";
static const char op_pfrsqit1   []="pfsqit1";
static const char op_pfrcpit2   []="pfrcpit2";
static const char op_pmulhrw    []="pmulhrw";
static const char op_prefetch   []="prefetch";
static const char op_prefetchw  []="prefetchw";

struct x86op {
    const char *name;
    long flags;
};

static const struct x86op singops[]={
/* 00 */    op_add,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 01 */    op_add,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 02 */    op_add,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 03 */    op_add,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 04 */    op_add,         ADDR2(ADDR_AL,ADDR_Ib),
/* 05 */    op_add,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 06 */    op_push,        ADDR_ES,
/* 07 */    op_pop,         ADDR_ES,
/* 08 */    op_or,          ADDR2(ADDR_Eb,ADDR_Gb),
/* 09 */    op_or,          ADDR2(ADDR_Ev,ADDR_Gv),
/* 0a */    op_or,          ADDR2(ADDR_Gb,ADDR_Eb),
/* 0b */    op_or,          ADDR2(ADDR_Gv,ADDR_Ev),
/* 0c */    op_or,          ADDR2(ADDR_AL,ADDR_Ib),
/* 0d */    op_or,          ADDR2(ADDR_EAX,ADDR_Iv),
/* 0e */    op_push,        ADDR_CS,
/* 0f */    NULL,           TYPE(TYPE_0f),
/* 10 */    op_adc,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 11 */    op_adc,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 12 */    op_adc,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 13 */    op_adc,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 14 */    op_adc,         ADDR2(ADDR_AL,ADDR_Ib),
/* 15 */    op_adc,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 16 */    op_push,        ADDR_SS,
/* 17 */    op_pop,         ADDR_SS,
/* 18 */    op_sbb,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 19 */    op_sbb,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 1a */    op_sbb,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 1b */    op_sbb,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 1c */    op_sbb,         ADDR2(ADDR_AL,ADDR_Ib),
/* 1d */    op_sbb,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 1e */    op_push,        ADDR_DS,
/* 1f */    op_pop,         ADDR_DS,
/* 20 */    op_and,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 21 */    op_and,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 22 */    op_and,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 23 */    op_and,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 24 */    op_and,         ADDR2(ADDR_AL,ADDR_Ib),
/* 25 */    op_and,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 26 */    NULL,           TYPE(TYPE_es_override),
/* 27 */    op_daa,         0,
/* 28 */    op_sub,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 29 */    op_sub,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 2a */    op_sub,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 2b */    op_sub,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 2c */    op_sub,         ADDR2(ADDR_AL,ADDR_Ib),
/* 2d */    op_sub,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 2e */    NULL,           TYPE(TYPE_cs_override),
/* 2f */    op_das,         0,
/* 30 */    op_xor,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 31 */    op_xor,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 32 */    op_xor,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 33 */    op_xor,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 34 */    op_xor,         ADDR2(ADDR_AL,ADDR_Ib),
/* 35 */    op_xor,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 36 */    NULL,           TYPE(TYPE_ss_override),
/* 37 */    op_aaa,         0,
/* 38 */    op_cmp,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 39 */    op_cmp,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 3a */    op_cmp,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 3b */    op_cmp,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 3c */    op_cmp,         ADDR2(ADDR_AL,ADDR_Ib),
/* 3d */    op_cmp,         ADDR2(ADDR_EAX,ADDR_Iv),
/* 3e */    NULL,           TYPE(TYPE_ds_override),
/* 3f */    op_aas,         0,
/* 40 */    op_inc,         ADDR_reg32,
/* 41 */    op_inc,         ADDR_reg32,
/* 42 */    op_inc,         ADDR_reg32,
/* 43 */    op_inc,         ADDR_reg32,
/* 44 */    op_inc,         ADDR_reg32,
/* 45 */    op_inc,         ADDR_reg32,
/* 46 */    op_inc,         ADDR_reg32,
/* 47 */    op_inc,         ADDR_reg32,
/* 48 */    op_dec,         ADDR_reg32,
/* 49 */    op_dec,         ADDR_reg32,
/* 4a */    op_dec,         ADDR_reg32,
/* 4b */    op_dec,         ADDR_reg32,
/* 4c */    op_dec,         ADDR_reg32,
/* 4d */    op_dec,         ADDR_reg32,
/* 4e */    op_dec,         ADDR_reg32,
/* 4f */    op_dec,         ADDR_reg32,
/* 50 */    op_push,        ADDR_reg32,
/* 51 */    op_push,        ADDR_reg32,
/* 52 */    op_push,        ADDR_reg32,
/* 53 */    op_push,        ADDR_reg32,
/* 54 */    op_push,        ADDR_reg32,
/* 55 */    op_push,        ADDR_reg32,
/* 56 */    op_push,        ADDR_reg32,
/* 57 */    op_push,        ADDR_reg32,
/* 58 */    op_pop,         ADDR_reg32,
/* 59 */    op_pop,         ADDR_reg32,
/* 5a */    op_pop,         ADDR_reg32,
/* 5b */    op_pop,         ADDR_reg32,
/* 5c */    op_pop,         ADDR_reg32,
/* 5d */    op_pop,         ADDR_reg32,
/* 5e */    op_pop,         ADDR_reg32,
/* 5f */    op_pop,         ADDR_reg32,
/* 60 */    op_pushad,      MOD(MOD16_take_d_off),
/* 61 */    op_popad,       MOD(MOD16_take_d_off),
/* 62 */    op_bound,       ADDR2(ADDR_Gv,ADDR_Ma),
/* 63 */    op_arpl,        ADDR2(ADDR_Ew,ADDR_Gw),
/* 64 */    NULL,           TYPE(TYPE_fs_override),
/* 65 */    NULL,           TYPE(TYPE_gs_override),
/* 66 */    NULL,           TYPE(TYPE_operand_size_override),
/* 67 */    NULL,           TYPE(TYPE_address_size_override),
/* 68 */    op_push,        ADDR_Iv,
/* 69 */    op_imul,        ADDR3(ADDR_Gv,ADDR_Ev,ADDR_Iv),
/* 6a */    op_push,        ADDR_Ib,
/* 6b */    op_imul,        ADDR3(ADDR_Gv,ADDR_Ev,ADDR_Ib),
/* 6c */    op_insb,        ADDR2(ADDR_Yb,ADDR_DX),
/* 6d */    op_insd,        ADDR2(ADDR_Yv,ADDR_DX) | MOD(MOD16_d_to_w),
/* 6e */    op_outsb,       ADDR2(ADDR_DX,ADDR_Xb),
/* 6f */    op_outsd,       ADDR2(ADDR_DX,ADDR_Xv) | MOD(MOD16_d_to_w),
/* 70 */    op_jo,          ADDR_Jb,
/* 71 */    op_jno,         ADDR_Jb,
/* 72 */    op_jc,          ADDR_Jb,
/* 73 */    op_jnc,         ADDR_Jb,
/* 74 */    op_jz,          ADDR_Jb,
/* 75 */    op_jnz,         ADDR_Jb,
/* 76 */    op_jbe,         ADDR_Jb,
/* 77 */    op_ja,          ADDR_Jb,
/* 78 */    op_js,          ADDR_Jb,
/* 79 */    op_jns,         ADDR_Jb,
/* 7a */    op_jp,          ADDR_Jb,
/* 7b */    op_jnp,         ADDR_Jb,
/* 7c */    op_jl,          ADDR_Jb,
/* 7d */    op_jge,         ADDR_Jb,
/* 7e */    op_jle,         ADDR_Jb,
/* 7f */    op_jg,          ADDR_Jb,
/* 80 */    NULL,           ADDR2(ADDR_Eb,ADDR_Ib) | GROUP(1),
/* 81 */    NULL,           ADDR2(ADDR_Ev,ADDR_Iv) | GROUP(1),
/* 82 */    NULL,           ADDR2(ADDR_Eb,ADDR_Ib) | GROUP(1),
/* 83 */    NULL,           ADDR2(ADDR_Ev,ADDR_Ibs) | GROUP(1),
/* 84 */    op_test,        ADDR2(ADDR_Eb,ADDR_Gb),
/* 85 */    op_test,        ADDR2(ADDR_Ev,ADDR_Gv),
/* 86 */    op_xchg,        ADDR2(ADDR_Eb,ADDR_Gb),
/* 87 */    op_xchg,        ADDR2(ADDR_Ev,ADDR_Gv),
/* 88 */    op_mov,         ADDR2(ADDR_Eb,ADDR_Gb),
/* 89 */    op_mov,         ADDR2(ADDR_Ev,ADDR_Gv),
/* 8a */    op_mov,         ADDR2(ADDR_Gb,ADDR_Eb),
/* 8b */    op_mov,         ADDR2(ADDR_Gv,ADDR_Ev),
/* 8c */    op_mov,         ADDR2(ADDR_Ew,ADDR_Sw),
/* 8d */    op_lea,         ADDR2(ADDR_Gv,ADDR_M),
/* 8e */    op_mov,         ADDR2(ADDR_Sw,ADDR_Ew),
/* 8f */    op_pop,         ADDR_Ev,
/* 90 */    op_nop,         0,
/* 91 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 92 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 93 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 94 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 95 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 96 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 97 */    op_xchg,        ADDR2(ADDR_EAX,ADDR_reg32),
/* 98 */    op_cbw,         0,
/* 99 */    op_cdq,         MOD(MOD16_dq_to_wd),
/* 9a */    op_call,        ADDR_Ap,
/* 9b */    op_wait,        0,
/* 9c */    op_pushfd,      MOD(MOD16_take_d_off),
/* 9d */    op_popfd,       MOD(MOD16_take_d_off),
/* 9e */    op_sahf,        0,
/* 9f */    op_lahf,        0,
/* a0 */    op_mov,         ADDR2(ADDR_AL,ADDR_Ob),
/* a1 */    op_mov,         ADDR2(ADDR_EAX,ADDR_Ov),
/* a2 */    op_mov,         ADDR2(ADDR_Ob,ADDR_AL),
/* a3 */    op_mov,         ADDR2(ADDR_Ob,ADDR_EAX),
/* a4 */    op_movsb,       ADDR2(ADDR_Xb,ADDR_Yb),
/* a5 */    op_movsd,       ADDR2(ADDR_Xv,ADDR_Yv) | MOD(MOD16_d_to_w),
/* a6 */    op_cmpsb,       ADDR2(ADDR_Xb,ADDR_Yb),
/* a7 */    op_cmpsd,       ADDR2(ADDR_Xv,ADDR_Yv) | MOD(MOD16_d_to_w),
/* a8 */    op_test,        ADDR2(ADDR_AL,ADDR_Ib),
/* a9 */    op_test,        ADDR2(ADDR_EAX,ADDR_Iv),
/* aa */    op_stosb,       ADDR2(ADDR_Yb,ADDR_AL),
/* ab */    op_stosd,       ADDR2(ADDR_Yv,ADDR_EAX) | MOD(MOD16_d_to_w),
/* ac */    op_lodsb,       ADDR2(ADDR_AL,ADDR_Xb),
/* ad */    op_lodsd,       ADDR2(ADDR_EAX,ADDR_Xv) | MOD(MOD16_d_to_w),
/* ae */    op_scasb,       ADDR2(ADDR_AL,ADDR_Yb),
/* af */    op_scasd,       ADDR2(ADDR_EAX,ADDR_Yv) | MOD(MOD16_d_to_w),
/* b0 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b1 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b2 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b3 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b4 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b5 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b6 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b7 */    op_mov,         ADDR2(ADDR_reg8,ADDR_Ib),
/* b8 */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* b9 */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* ba */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* bb */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* bc */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* bd */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* be */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* bf */    op_mov,         ADDR2(ADDR_reg32,ADDR_Iv),
/* c0 */    NULL,           ADDR2(ADDR_Eb,ADDR_Ib) | GROUP(2),
/* c1 */    NULL,           ADDR2(ADDR_Ev,ADDR_Ib) | GROUP(2),
/* c2 */    op_ret,         ADDR_Iw,
/* c3 */    op_ret,         0,
/* c4 */    op_les,         ADDR2(ADDR_Gv,ADDR_Mp),
/* c5 */    op_lds,         ADDR2(ADDR_Gv,ADDR_Mp),
/* c6 */    op_mov,         ADDR2(ADDR_Eb,ADDR_Ib),
/* c7 */    op_mov,         ADDR2(ADDR_Ev,ADDR_Iv),
/* c8 */    op_enter,       ADDR2(ADDR_Iw,ADDR_Ib),
/* c9 */    op_leave,       0,
/* ca */    op_retf,        ADDR_Iw,
/* cb */    op_retf,        0,
/* cc */    op_int3,        0,
/* cd */    op_int,         ADDR_Ib,
/* ce */    op_into,        0,
/* cf */    op_iret,        0,
/* d0 */    NULL,           ADDR2(ADDR_Eb,ADDR_1) | GROUP(2),
/* d1 */    NULL,           ADDR2(ADDR_Ev,ADDR_1) | GROUP(2),
/* d2 */    NULL,           ADDR2(ADDR_Eb,ADDR_CL) | GROUP(2),
/* d3 */    NULL,           ADDR2(ADDR_Ev,ADDR_CL) | GROUP(2),
/* d4 */    op_aam,         ADDR_Ib,
/* d5 */    op_aad,         ADDR_Ib,
/* d6 */    NULL,           0,
/* d7 */    op_xlat,        0,
/* d8 */    NULL,           GROUP(13),  // D8-DF: floating point escapes
/* d9 */    NULL,           GROUP(14),
/* da */    NULL,           GROUP(15),
/* db */    NULL,           GROUP(16),
/* dc */    NULL,           GROUP(17),
/* dd */    NULL,           GROUP(18),
/* de */    NULL,           GROUP(19),
/* df */    NULL,           GROUP(20),
/* e0 */    op_loopn,       ADDR_Jb,
/* e1 */    op_loope,       ADDR_Jb,
/* e2 */    op_loop,        ADDR_Jb,
/* e3 */    op_jecxz,       ADDR_Jb,
/* e4 */    op_in,          ADDR2(ADDR_AL,ADDR_Ib),
/* e5 */    op_in,          ADDR2(ADDR_EAX,ADDR_Ib),
/* e6 */    op_out,         ADDR2(ADDR_Ib,ADDR_AL),
/* e7 */    op_out,         ADDR2(ADDR_Ib,ADDR_EAX),
/* e8 */    op_call,        ADDR_Jv,
/* e9 */    op_jmp,         ADDR_Jv,
/* ea */    op_jmp,         ADDR_Ap,
/* eb */    op_jmp,         ADDR_Jb,
/* ec */    op_in,          ADDR2(ADDR_AL,ADDR_DX),
/* ed */    op_in,          ADDR2(ADDR_EAX,ADDR_DX),
/* ee */    op_out,         ADDR2(ADDR_DX,ADDR_AL),
/* ef */    op_out,         ADDR2(ADDR_DX,ADDR_EAX),
/* f0 */    op_lock,        TYPE(TYPE_prefix),
/* f1 */    NULL,           0,
/* f2 */    op_repne,       TYPE(TYPE_prefix),
/* f3 */    op_rep,         TYPE(TYPE_prefix),
/* f4 */    op_hlt,         0,
/* f5 */    op_cmc,         0,
/* f6 */    NULL,           ADDR_Eb | GROUP(3),
/* f7 */    NULL,           ADDR_Ev | GROUP(3),
/* f8 */    op_clc,         0,
/* f9 */    op_stc,         0,
/* fa */    op_cli,         0,
/* fb */    op_sti,         0,
/* fc */    op_cld,         0,
/* fd */    op_std,         0,
/* fe */    NULL,           GROUP(4),
/* ff */    NULL,           GROUP(5),
};

static const struct x86op prefix0f_ops[]={
/* 00 */    NULL,           GROUP(6),
/* 01 */    NULL,           0,
/* 02 */    op_lar,         ADDR2(ADDR_Gv, ADDR_Ew),
/* 03 */    op_lsl,         ADDR2(ADDR_Gv, ADDR_Ew),
/* 04 */    NULL,           0,
/* 05 */    NULL,           0,
/* 06 */    op_clts,        0,
/* 07 */    NULL,           0,
/* 08 */    op_invd,        0,
/* 09 */    op_wbinvd,      0,
/* 0a */    NULL,           0,
/* 0b */    op_ud2,         0,
/* 0c */    NULL,           0,
/* 0d */    NULL,           GROUP(21),      // 3DNow! prefetch instructions
/* 0e */    op_femms,       0,
/* 0f */    NULL,           SPECIAL_3DNOW,
/* 10 */    NULL,           0,
/* 11 */    NULL,           0,
/* 12 */    NULL,           0,
/* 13 */    NULL,           0,
/* 14 */    NULL,           0,
/* 15 */    NULL,           0,
/* 16 */    NULL,           0,
/* 17 */    NULL,           0,
/* 18 */    NULL,           0,
/* 19 */    NULL,           0,
/* 1a */    NULL,           0,
/* 1b */    NULL,           0,
/* 1c */    NULL,           0,
/* 1d */    NULL,           0,
/* 1e */    NULL,           0,
/* 1f */    NULL,           0,
/* 20 */    op_mov,         ADDR2(ADDR_Rd, ADDR_Cd),
/* 21 */    op_mov,         ADDR2(ADDR_Rd, ADDR_Dd),
/* 22 */    op_mov,         ADDR2(ADDR_Cd, ADDR_Rd),
/* 23 */    op_mov,         ADDR2(ADDR_Dd, ADDR_Rd),
/* 24 */    NULL,           0,
/* 25 */    NULL,           0,
/* 26 */    NULL,           0,
/* 27 */    NULL,           0,
/* 28 */    NULL,           0,
/* 29 */    NULL,           0,
/* 2a */    NULL,           0,
/* 2b */    NULL,           0,
/* 2c */    NULL,           0,
/* 2d */    NULL,           0,
/* 2e */    NULL,           0,
/* 2f */    NULL,           0,
/* 30 */    op_wrmsr,       0,
/* 31 */    op_rdtsc,       0,
/* 32 */    op_rdmsr,       0,
/* 33 */    op_rdpmc,       0,
/* 34 */    NULL,           0,
/* 35 */    NULL,           0,
/* 36 */    NULL,           0,
/* 37 */    NULL,           0,
/* 38 */    NULL,           0,
/* 39 */    NULL,           0,
/* 3a */    NULL,           0,
/* 3b */    NULL,           0,
/* 3c */    NULL,           0,
/* 3d */    NULL,           0,
/* 3e */    NULL,           0,
/* 3f */    NULL,           0,
/* 40 */    op_cmovo,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 41 */    op_cmovno,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 42 */    op_cmovc,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 43 */    op_cmovnc,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 44 */    op_cmovz,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 45 */    op_cmovnz,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 46 */    op_cmovbe,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 47 */    op_cmova,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 48 */    op_cmovs,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 49 */    op_cmovns,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 4a */    op_cmovp,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 4b */    op_cmovnp,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 4c */    op_cmovl,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 4d */    op_cmovge,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 4e */    op_cmovle,      ADDR2(ADDR_Gv,ADDR_Ev),
/* 4f */    op_cmovg,       ADDR2(ADDR_Gv,ADDR_Ev),
/* 50 */    NULL,           0,
/* 51 */    NULL,           0,
/* 52 */    NULL,           0,
/* 53 */    NULL,           0,
/* 54 */    NULL,           0,
/* 55 */    NULL,           0,
/* 56 */    NULL,           0,
/* 57 */    NULL,           0,
/* 58 */    NULL,           0,
/* 59 */    NULL,           0,
/* 5a */    NULL,           0,
/* 5b */    NULL,           0,
/* 5c */    NULL,           0,
/* 5d */    NULL,           0,
/* 5e */    NULL,           0,
/* 5f */    NULL,           0,
/* 60 */    op_punpcklbw,   ADDR2(ADDR_Pq,ADDR_Qd),
/* 61 */    op_punpcklwd,   ADDR2(ADDR_Pq,ADDR_Qd),
/* 62 */    op_punpckldq,   ADDR2(ADDR_Pq,ADDR_Qd),
/* 63 */    op_packusdw,    ADDR2(ADDR_Pq,ADDR_Qd),
/* 64 */    op_pcmpgtb,     ADDR2(ADDR_Pq,ADDR_Qd),
/* 65 */    op_pcmpgtw,     ADDR2(ADDR_Pq,ADDR_Qd),
/* 66 */    op_pcmpgtd,     ADDR2(ADDR_Pq,ADDR_Qd),
/* 67 */    op_packsswb,    ADDR2(ADDR_Pq,ADDR_Qd),
/* 68 */    op_punpckhbw,   ADDR2(ADDR_Pq,ADDR_Qd),
/* 69 */    op_punpckhwd,   ADDR2(ADDR_Pq,ADDR_Qd),
/* 6a */    op_punpckhdq,   ADDR2(ADDR_Pq,ADDR_Qd),
/* 6b */    op_packssdw,    ADDR2(ADDR_Pq,ADDR_Qd),
/* 6c */    NULL,           0,
/* 6d */    NULL,           0,
/* 6e */    op_movd,        ADDR2(ADDR_Pd,ADDR_Ed),
/* 6f */    op_movq,        ADDR2(ADDR_Pq,ADDR_Qq),
/* 70 */    NULL,           0,
/* 71 */    NULL,           GROUP(10),
/* 72 */    NULL,           GROUP(11),
/* 73 */    NULL,           GROUP(12),
/* 74 */    op_pcmpeqb,     ADDR2(ADDR_Pq,ADDR_Qd),
/* 75 */    op_pcmpeqw,     ADDR2(ADDR_Pq,ADDR_Qd),
/* 76 */    op_pcmpeqd,     ADDR2(ADDR_Pq,ADDR_Qd),
/* 77 */    op_emms,        0,
/* 78 */    NULL,           0,
/* 79 */    NULL,           0,
/* 7a */    NULL,           0,
/* 7b */    NULL,           0,
/* 7c */    NULL,           0,
/* 7d */    NULL,           0,
/* 7e */    op_movd,        ADDR2(ADDR_Ed,ADDR_Pd),
/* 7f */    op_movq,        ADDR2(ADDR_Qq,ADDR_Pq),
/* 80 */    op_jo,          ADDR_Jv,
/* 81 */    op_jno,         ADDR_Jv,
/* 82 */    op_jc,          ADDR_Jv,
/* 83 */    op_jnc,         ADDR_Jv,
/* 84 */    op_jz,          ADDR_Jv,
/* 85 */    op_jnz,         ADDR_Jv,
/* 86 */    op_jbe,         ADDR_Jv,
/* 87 */    op_ja,          ADDR_Jv,
/* 88 */    op_js,          ADDR_Jv,
/* 89 */    op_jns,         ADDR_Jv,
/* 8a */    op_jp,          ADDR_Jv,
/* 8b */    op_jnp,         ADDR_Jv,
/* 8c */    op_jl,          ADDR_Jv,
/* 8d */    op_jge,         ADDR_Jv,
/* 8e */    op_jle,         ADDR_Jv,
/* 8f */    op_jg,          ADDR_Jv,
/* 90 */    op_seto,        ADDR_Eb,
/* 91 */    op_setno,       ADDR_Eb,
/* 92 */    op_setc,        ADDR_Eb,
/* 93 */    op_setnc,       ADDR_Eb,
/* 94 */    op_setz,        ADDR_Eb,
/* 95 */    op_setnz,       ADDR_Eb,
/* 96 */    op_setbe,       ADDR_Eb,
/* 97 */    op_seta,        ADDR_Eb,
/* 98 */    op_sets,        ADDR_Eb,
/* 99 */    op_setns,       ADDR_Eb,
/* 9a */    op_setp,        ADDR_Eb,
/* 9b */    op_setnp,       ADDR_Eb,
/* 9c */    op_setl,        ADDR_Eb,
/* 9d */    op_setge,       ADDR_Eb,
/* 9e */    op_setle,       ADDR_Eb,
/* 9f */    op_setg,        ADDR_Eb,
/* a0 */    op_push,        ADDR_FS,
/* a1 */    op_pop,         ADDR_FS,
/* a2 */    op_cpuid,       0,
/* a3 */    op_bt,          ADDR2(ADDR_Ev,ADDR_Gv),
/* a4 */    op_shld,        ADDR3(ADDR_Ev,ADDR_Gv,ADDR_Ib),
/* a5 */    op_shld,        ADDR3(ADDR_Ev,ADDR_Gv,ADDR_CL),
/* a6 */    NULL,           0,
/* a7 */    NULL,           0,
/* a8 */    op_push,        ADDR_GS,
/* a9 */    op_pop,         ADDR_GS,
/* aa */    op_rsm,         0,
/* ab */    op_bts,         ADDR2(ADDR_Ev,ADDR_Gv),
/* ac */    op_shrd,        ADDR3(ADDR_Ev,ADDR_Gv,ADDR_Ib),
/* ad */    op_shrd,        ADDR3(ADDR_Ev,ADDR_Gv,ADDR_CL),
/* ae */    NULL,           0,
/* af */    op_imul,        ADDR2(ADDR_Gv,ADDR_Ev),
/* b0 */    op_cmpxchg,     ADDR2(ADDR_Eb,ADDR_Gb),
/* b1 */    op_cmpxchg,     ADDR2(ADDR_Ev,ADDR_Gv),
/* b2 */    op_lss,         ADDR_Mp,
/* b3 */    op_btr,         ADDR2(ADDR_Ev,ADDR_Gv),
/* b4 */    op_lfs,         ADDR_Mp,
/* b5 */    op_lgs,         ADDR_Mp,
/* b6 */    op_movzx,       ADDR2(ADDR_Gv,ADDR_Eb),
/* b7 */    op_movzx,       ADDR2(ADDR_Gv,ADDR_Ew),
/* b8 */    NULL,           0,
/* b9 */    op_invalid,     0,
/* ba */    NULL,           GROUP(8),
/* bb */    op_btc,         ADDR2(ADDR_Ev,ADDR_Gv),
/* bc */    op_bsf,         ADDR2(ADDR_Gv,ADDR_Ev),
/* bd */    op_bsr,         ADDR2(ADDR_Gv,ADDR_Ev),
/* be */    op_movsx,       ADDR2(ADDR_Gv,ADDR_Eb),
/* bf */    op_movsx,       ADDR2(ADDR_Gv,ADDR_Ew),
/* c0 */    op_xadd,        ADDR2(ADDR_Eb,ADDR_Gb),
/* c1 */    op_xadd,        ADDR2(ADDR_Ev,ADDR_Gv),
/* c2 */    NULL,           0,
/* c3 */    NULL,           0,
/* c4 */    NULL,           0,
/* c5 */    NULL,           0,
/* c6 */    NULL,           0,
/* c7 */    NULL,           GROUP(9),
/* c8 */    op_bswap,       ADDR_reg32,
/* c9 */    op_bswap,       ADDR_reg32,
/* ca */    op_bswap,       ADDR_reg32,
/* cb */    op_bswap,       ADDR_reg32,
/* cc */    op_bswap,       ADDR_reg32,
/* cd */    op_bswap,       ADDR_reg32,
/* ce */    op_bswap,       ADDR_reg32,
/* cf */    op_bswap,       ADDR_reg32,
/* d0 */    NULL,           0,
/* d1 */    op_psrlw,       ADDR2(ADDR_Pq,ADDR_Qd),
/* d2 */    op_psrld,       ADDR2(ADDR_Pq,ADDR_Qd),
/* d3 */    op_psrlq,       ADDR2(ADDR_Pq,ADDR_Qd),
/* d4 */    NULL,           0,
/* d5 */    op_pmullw,      ADDR2(ADDR_Pq,ADDR_Qd),
/* d6 */    NULL,           0,
/* d7 */    NULL,           0,
/* d8 */    op_psubusb,     ADDR2(ADDR_Pq,ADDR_Qq),
/* d9 */    op_psubusw,     ADDR2(ADDR_Pq,ADDR_Qq),
/* da */    NULL,           0,
/* db */    op_pand,        ADDR2(ADDR_Pq,ADDR_Qq),
/* dc */    op_paddusb,     ADDR2(ADDR_Pq,ADDR_Qq),
/* dd */    op_paddusw,     ADDR2(ADDR_Pq,ADDR_Qq),
/* de */    NULL,           0,
/* df */    op_pandn,       ADDR2(ADDR_Pq,ADDR_Qq),
/* e0 */    NULL,           0,
/* e1 */    op_psraw,       ADDR2(ADDR_Pq,ADDR_Qd),
/* e2 */    op_psrad,       ADDR2(ADDR_Pq,ADDR_Qd),
/* e3 */    NULL,           0,
/* e4 */    NULL,           0,
/* e5 */    op_pmulhw,      ADDR2(ADDR_Pq,ADDR_Qd),
/* e6 */    NULL,           0,
/* e7 */    NULL,           0,
/* e8 */    op_psubsb,      ADDR2(ADDR_Pq,ADDR_Qd),
/* e9 */    op_psubsw,      ADDR2(ADDR_Pq,ADDR_Qd),
/* ea */    NULL,           0,
/* eb */    op_por,         ADDR2(ADDR_Pq,ADDR_Qd),
/* ec */    op_paddsb,      ADDR2(ADDR_Pq,ADDR_Qd),
/* ed */    op_paddsw,      ADDR2(ADDR_Pq,ADDR_Qd),
/* ee */    NULL,           0,
/* ef */    op_pxor,        ADDR2(ADDR_Pq,ADDR_Qd),
/* f0 */    NULL,           0,
/* f1 */    op_psllw,       ADDR2(ADDR_Pq,ADDR_Qd),
/* f2 */    op_pslld,       ADDR2(ADDR_Pq,ADDR_Qd),
/* f3 */    op_psllq,       ADDR2(ADDR_Pq,ADDR_Qd),
/* f4 */    NULL,           0,
/* f5 */    op_pmaddwd,     ADDR2(ADDR_Pq,ADDR_Qd),
/* f6 */    NULL,           0,
/* f7 */    NULL,           0,
/* f8 */    op_psubb,       ADDR2(ADDR_Pq,ADDR_Qd),
/* f9 */    op_psubw,       ADDR2(ADDR_Pq,ADDR_Qd),
/* fa */    op_psubd,       ADDR2(ADDR_Pq,ADDR_Qd),
/* fb */    NULL,           0,
/* fc */    op_paddb,       ADDR2(ADDR_Pq,ADDR_Qd),
/* fd */    op_paddw,       ADDR2(ADDR_Pq,ADDR_Qd),
/* fe */    op_paddd,       ADDR2(ADDR_Pq,ADDR_Qd),
/* ff */    NULL,           0,
};

static const struct x86op group1_ops[]={
/* 000 */   op_add,         0,
/* 001 */   op_or,          0,
/* 010 */   op_adc,         0,
/* 011 */   op_sbb,         0,
/* 100 */   op_and,         0,
/* 101 */   op_sub,         0,
/* 110 */   op_xor,         0,
/* 111 */   op_cmp,         0,
};

static const struct x86op group2_ops[]={
/* 000 */   op_rol,         0,
/* 001 */   op_ror,         0,
/* 010 */   op_rcl,         0,
/* 011 */   op_rcr,         0,
/* 100 */   op_shl,         0,
/* 101 */   op_shr,         0,
/* 110 */   NULL,           0,
/* 111 */   op_sar,         0,
};

static const struct x86op group3_ops[]={
/* 000 */   op_test,        ADDR1(ADDR_Idep),
/* 001 */   NULL,           0,
/* 010 */   op_not,         0,
/* 011 */   op_neg,         0,
/* 100 */   op_mul,         0,
/* 101 */   op_imul,        0,
/* 110 */   op_div,         0,
/* 111 */   op_idiv,        0,
};

static const struct x86op group4_ops[]={
/* 000 */   op_inc,         ADDR_Eb,
/* 001 */   op_dec,         ADDR_Eb,
/* 010 */   NULL,           0,
/* 011 */   NULL,           0,
/* 100 */   NULL,           0,
/* 101 */   NULL,           0,
/* 110 */   NULL,           0,
/* 111 */   NULL,           0,
};

static const struct x86op group5_ops[]={
/* 000 */   op_inc,         ADDR_Ev,
/* 001 */   op_dec,         ADDR_Ev,
/* 010 */   op_call,        ADDR_Ev,
/* 011 */   op_call,        ADDR_Ep,
/* 100 */   op_jmp,         ADDR_Ev,
/* 101 */   op_jmp,         ADDR_Ep,
/* 110 */   op_push,        ADDR_Ev,
/* 111 */   NULL,           0,
};

static const struct x86op group6_ops[]={
/* 000 */   op_sldt,        ADDR_Ew,
/* 001 */   op_str,         ADDR_Ew,
/* 010 */   op_lldt,        ADDR_Ew,
/* 011 */   op_ltr,         ADDR_Ew,
/* 100 */   op_verr,        ADDR_Ew,
/* 101 */   op_verw,        ADDR_Ew,
/* 110 */   NULL,           0,
/* 111 */   NULL,           0,
};

static const struct x86op group7_ops[]={
/* 000 */   op_sgdt,        ADDR_M,
/* 001 */   op_sidt,        ADDR_M,
/* 010 */   op_lgdt,        ADDR_M,
/* 011 */   op_lidt,        ADDR_M,
/* 100 */   op_smsw,        ADDR_Ew,
/* 101 */   NULL,           0,
/* 110 */   op_lmsw,        ADDR_Ew,
/* 111 */   op_invlpg,      0,
};

static const struct x86op group8_ops[]={
/* 000 */   NULL,           0,
/* 001 */   NULL,           0,
/* 010 */   NULL,           0,
/* 011 */   NULL,           0,
/* 100 */   op_bt,          ADDR2(ADDR_Gv, ADDR_Ib),
/* 101 */   op_bts,         ADDR2(ADDR_Gv, ADDR_Ib),
/* 110 */   op_btr,         ADDR2(ADDR_Gv, ADDR_Ib),
/* 111 */   op_btc,         ADDR2(ADDR_Gv, ADDR_Ib),
};

static const struct x86op group9_ops[]={
/* 000 */   NULL,           0,
/* 001 */   op_cmpxchg8b,   ADDR_M,
/* 010 */   NULL,           0,
/* 011 */   NULL,           0,
/* 100 */   NULL,           0,
/* 101 */   NULL,           0,
/* 110 */   NULL,           0,
/* 111 */   NULL,           0,
};

static const struct x86op group10w_ops[]={
/* 000 */   NULL,           0,
/* 001 */   NULL,           0,
/* 010 */   op_psrlw,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 011 */   NULL,           0,
/* 100 */   op_psraw,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 101 */   NULL,           0,
/* 110 */   op_psllw,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 111 */   NULL,           0,
};

static const struct x86op group10d_ops[]={
/* 000 */   NULL,           0,
/* 001 */   NULL,           0,
/* 010 */   op_psrld,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 011 */   NULL,           0,
/* 100 */   op_psrad,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 101 */   NULL,           0,
/* 110 */   op_pslld,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 111 */   NULL,           0,
};

static const struct x86op group10q_ops[]={
/* 000 */   NULL,           0,
/* 001 */   NULL,           0,
/* 010 */   op_psrlq,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 011 */   NULL,           0,
/* 100 */   NULL,           0,
/* 101 */   NULL,           0,
/* 110 */   op_psllq,       ADDR2(ADDR_Pq2, ADDR_Ib2),
/* 111 */   NULL,           0,
};

static const struct x86op group_fpD8[]={
/* 000 */   op_fadd,            ADDR_Ed,
/* 001 */   op_fmul,            ADDR_Ed,
/* 010 */   op_fcom,            ADDR_Ed,
/* 011 */   op_fcomp,           ADDR_Ed,
/* 100 */   op_fsub,            ADDR_Ed,
/* 101 */   op_fsubr,           ADDR_Ed,
/* 110 */   op_fdiv,            ADDR_Ed,
/* 111 */   op_fdivr,           ADDR_Ed,
};

static const struct x86op group_fpD9[]={
/* 000 */   op_fld,             ADDR_Ed,
/* 001 */   NULL,               0,
/* 010 */   op_fst,             ADDR_Ed,
/* 011 */   op_fstp,            ADDR_Ed,
/* 100 */   op_fldenv,          ADDR_Eb,
/* 101 */   op_fldcw,           ADDR_Ew,
/* 110 */   op_fstenv,          ADDR_Eb,
/* 111 */   op_fstcw,           ADDR_Ew,
};

static const struct x86op group_fpDA[]={
/* 000 */   op_fiadd,           ADDR_Ed,
/* 001 */   op_fimul,           ADDR_Ed,
/* 010 */   op_ficom,           ADDR_Ed,
/* 011 */   op_ficomp,          ADDR_Ed,
/* 100 */   op_fisub,           ADDR_Ed,
/* 101 */   op_fisubr,          ADDR_Ed,
/* 110 */   op_fidiv,           ADDR_Ed,
/* 111 */   op_fidivr,          ADDR_Ed,
};

static const struct x86op group_fpDB[]={
/* 000 */   op_fild,            ADDR_Ed,
/* 001 */   NULL,               0,
/* 010 */   op_fist,            ADDR_Ed,
/* 011 */   op_fistp,           ADDR_Ed,
/* 100 */   NULL,               0,
/* 101 */   op_fld,             ADDR_Ex,
/* 110 */   NULL,               0,
/* 111 */   op_fstp,            ADDR_Ex,
};

static const struct x86op group_fpDC[]={
/* 000 */   op_fadd,            ADDR_Eq,
/* 001 */   op_fmul,            ADDR_Eq,
/* 010 */   op_fcom,            ADDR_Eq,
/* 011 */   op_fcomp,           ADDR_Eq,
/* 100 */   op_fsub,            ADDR_Eq,
/* 101 */   op_fsubr,           ADDR_Eq,
/* 110 */   op_fdiv,            ADDR_Eq,
/* 111 */   op_fdivr,           ADDR_Eq,
};

static const struct x86op group_fpDD[]={
/* 000 */   op_fld,             ADDR_Eq,
/* 001 */   NULL,               0,
/* 010 */   op_fst,             ADDR_Eq,
/* 011 */   op_fstp,            ADDR_Eq,
/* 100 */   op_frstor,          ADDR_Eb,
/* 101 */   NULL,               0,
/* 110 */   op_fsave,           ADDR_Eb,
/* 111 */   op_fstsw,           ADDR_Ew,
};

static const struct x86op group_fpDE[]={
/* 000 */   op_fiadd,           ADDR_Ew,
/* 001 */   op_fimul,           ADDR_Ew,
/* 010 */   op_ficom,           ADDR_Ew,
/* 011 */   op_ficomp,          ADDR_Ew,
/* 100 */   op_fisub,           ADDR_Ew,
/* 101 */   op_fisubr,          ADDR_Ew,
/* 110 */   op_fidiv,           ADDR_Ew,
/* 111 */   op_fidivr,          ADDR_Ew,
};

static const struct x86op group_fpDF[]={
/* 000 */   op_fild,            ADDR_Ew,
/* 001 */   NULL,               0,
/* 010 */   op_fist,            ADDR_Ew,
/* 011 */   op_fistp,           ADDR_Ew,
/* 100 */   op_fbld,            ADDR_Eb,
/* 101 */   op_fild,            ADDR_Eq,
/* 110 */   op_fbstp,           ADDR_Eb,
/* 111 */   op_fistp,           ADDR_Eq,
};

static const struct x86op group_3DNow_PREFETCH[]={
/* 000 */   op_prefetch,        ADDR_Qq,
/* 001 */   op_prefetchw,       ADDR_Qq,
/* 010 */   NULL,               0,
/* 011 */   NULL,               0,
/* 100 */   NULL,               0,
/* 101 */   NULL,               0,
/* 110 */   NULL,               0,
/* 111 */   NULL,               0,
};

static const struct x86op * const groups[]={
    group1_ops,
    group2_ops,
    group3_ops,
    group4_ops,
    group5_ops,
    group6_ops,
    group7_ops,
    group8_ops,
    group9_ops,
    group10w_ops,
    group10d_ops,
    group10q_ops,
    group_fpD8,
    group_fpD9,
    group_fpDA,
    group_fpDB,
    group_fpDC,
    group_fpDD,
    group_fpDE,
    group_fpDF,
    group_3DNow_PREFETCH,
};

static const char fp_format_st0_stn[]="st(0),st(%d)";
static const char fp_format_stn_st0[]="st(%d),st(0)";
static const char fp_format_stn[]="st(%d)";

static const char * const fpext_reg_tab[8][8][2]={
    /* D8 */ {  op_fadd,    fp_format_st0_stn,
                op_fmul,    fp_format_st0_stn,
                op_fcom,    fp_format_st0_stn,
                op_fcomp,   fp_format_st0_stn,
                op_fsub,    fp_format_st0_stn,
                op_fsubr,   fp_format_st0_stn,
                op_fdiv,    fp_format_st0_stn,
                op_fdivr,   fp_format_st0_stn,
             },
    /* D9 */ {  op_fld,     fp_format_st0_stn,
                op_fxch,    fp_format_st0_stn,
                NULL,       "fnop\0\0\0\0\0\0\0",
                NULL,       NULL,
                NULL,       "fchs\0fabs\0\0\0ftst\0fxam\0\0",
                NULL,       "fld1\0fldl2t\0fldl2e\0fldpi\0fldlg2\0fldln2\0fldz\0",
                NULL,       "f2xm1\0fyl2x\0fptan\0fpatan\0fxtract\0fprem1\0fdecstp\0fincstp",
                NULL,       "fprem\0fyl2xp1\0fsqrt\0fsincos\0frndint\0fscale\0fsin\0fcos",
             },
    /* DA */ {  op_fcmovb,  fp_format_st0_stn,
                op_fcmovbe, fp_format_st0_stn,
                op_fcmove,  fp_format_st0_stn,
                op_fcmovu,  fp_format_st0_stn,
                NULL,       NULL,
                NULL,       "\0fucompp\0\0\0\0\0\0",
                NULL,       NULL,
                NULL,       NULL,
             },
    /* DB */ {  op_fcmovnb,     fp_format_st0_stn,
                op_fcmovne,     fp_format_st0_stn,
                op_fcmovnbe,    fp_format_st0_stn,
                op_fcmovnu,     fp_format_st0_stn,
                NULL,           "\0\0fclex\0finit\0\0\0\0",
                op_fucomi,      fp_format_st0_stn,
                op_fcomi,       fp_format_st0_stn,
                NULL,           NULL,
             },
    /* DC */ {  op_fadd,    fp_format_stn_st0,
                op_fmul,    fp_format_stn_st0,
                NULL,       NULL,
                NULL,       NULL,
                op_fsubr,   fp_format_stn_st0,
                op_fsub,    fp_format_stn_st0,
                op_fdivr,   fp_format_stn_st0,
                op_fdiv,    fp_format_stn_st0,
             },
    /* DD */ {  op_ffree,   fp_format_stn,
                NULL,       NULL,
                op_fst,     fp_format_stn,
                op_fstp,    fp_format_stn,
                op_fucom,   fp_format_stn,
                op_fucomp,  fp_format_stn_st0,
                NULL,       NULL,
                NULL,       NULL,
             },
    /* DE */ {  op_faddp,   fp_format_stn_st0,
                op_fmulp,   fp_format_stn_st0,
                NULL,       NULL,
                NULL,       "\0fcompp\0\0\0\0\0\0",
                op_fsubrp,  fp_format_stn_st0,
                op_fsubp,   fp_format_stn_st0,
                op_fdivrp,  fp_format_stn_st0,
                op_fdivp,   fp_format_stn_st0,
             },
    /* DF */ {  NULL,       NULL,
                NULL,       NULL,
                NULL,       NULL,
                NULL,       NULL,
                NULL,       "fstsw ax\0\0\0\0\0\0\0",
                op_fucomip, fp_format_st0_stn,
                op_fcomip,  fp_format_st0_stn,
                NULL,       NULL,
             },
};

static const struct x86_3DNow_op {
    unsigned char   opcode;
    struct x86op opdata;
} group_3DNow[]={
    {   0x0D,   op_pi2fd,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x1D,   op_pf2id,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x90,   op_pfcmpge,     ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x94,   op_pfmin,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x96,   op_pfrcp,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x97,   op_pfrsqrt,     ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x9A,   op_pfsub,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0x9E,   op_pfadd,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xA0,   op_pfcmpgt,     ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xA4,   op_pfmax,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xA6,   op_pfrcpit1,    ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xA7,   op_pfrsqit1,    ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xAA,   op_pfsubr,      ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xAE,   op_pfacc,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xB0,   op_pfcmpeq,     ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xB4,   op_pfmul,       ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xB6,   op_pfrcpit2,    ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xB7,   op_pmulhrw,     ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0xBF,   op_pavgusb,     ADDR2(ADDR_Pq,ADDR_Qq)  },
    {   0, NULL }
};

static const char * const reg8[]={ "al","cl","dl","bl","ah","ch","dh","bh" };
static const char * const reg16[]={ "ax","cx","dx","bx","sp","bp","si","di" };
static const char * const reg32[]={ "eax","ecx","edx","ebx","esp","ebp","esi","edi" };
static const char * const regmmx[]={ "mm0","mm1","mm2","mm3","mm4","mm5","mm6","mm7" };
static const char * const sregs[]={ "es", "cs", "ss", "ds", "fs", "gs" };

static char *strtack(char *d, const char *s) 
{
    char c;

    while(c=*s++) *d++=c;
    *d = 0;
    return d;
}

static long regmask_modify, regmask_access, regmask_address;
static int pipe_force;
static char *ptr_insert;
static const char *ptr_insert_type;

#define PIPE_UV (0)
#define PIPE_U (1)
#define PIPE_V (2)

#define PIPE_RESET_UV() (pipe_force = PIPE_UV)
#define PIPE_FORCE_U() (pipe_force = PIPE_U)
#define PIPE_FORCE_V() (pipe_force = PIPE_V)

#define DATA_ACCESS_32(r) (regmask_access |= 0x1L<<(r))
#define DATA_ACCESS_FP(r) (regmask_access |= 0x100L<<(r))
#define DATA_ACCESS_MMX DATA_ACCESS_FP
#define DATA_ACCESS_SEGMENT(r) (regmask_access |= 0x10000L<<(r))
#define ADDRESS_ACCESS_32(r) (regmask_address |= 0x1L<<(r))

static void disasm_modrm(unsigned char *&sptr, BOOL big, char *&buf, BOOL is_32_op, BOOL is_32_addr, const char *ptrtype, const char *segoverride) 
{
    const char *const *regs = big==2 ? regmmx : big ? is_32_op ? reg32 : reg16 : reg8;
    unsigned char modrm = *sptr++;

    ptr_insert_type = ptrtype;

    if (!is_32_addr) 
    {
        strcpy(buf,"<<16-bit addr>>");
    } 
    else 
    {
        if ((modrm&7) == 4 && modrm<0xc0) 
        {
            unsigned char sib = *sptr++;
            bool needplus = false;

            ptr_insert = buf;

            *buf++='[';

            // We have to watch out for these special cases:
            //
            //  MOD=00, SIB=x5 or xD    disp32[index]
            //  MOD=01, SIB=x5 or xD    disp8[EBP+index]
            //  MOD=10, SIB=x5 or xD    disp32[EBP+index]

            // Print out base register; omit EBP for MOD=00

            if ((sib&7) != 5 || modrm>=0x40) 
            {
                strcpy(buf, reg32[sib&7]);
                ADDRESS_ACCESS_32(sib&7);
                buf+=3;
                needplus = true;
            }

            // Print out index*scale; if index=100 there is no index register

            if ((sib&0x38)!=0x20) 
            {

                if (needplus)
                {
                    *buf++ = '+';
                }

                strcpy(buf, reg32[(sib>>3)&7]);
                ADDRESS_ACCESS_32((sib>>3)&7);
                buf+=3;

                if (sib>=0x40) 
                {
                    *buf++ = '*';
                    *buf++ = '0' + (1<<((sib>>6)&3));
                }

                needplus = true;
            }

            // Print out displacement; force a disp32 for base=101, mod=00.
            // We can get only a disp32 if mod=00, base=101, and index=100,
            // so watch the plus
            
            if (modrm>=0x40 && modrm<0x80) 
            {
                buf+=sprintf(buf,"%+d",(int)(signed char)*sptr++);
            } 
            else if (((sib&7) == 5 && modrm<0x40) || (modrm>=0x80 && modrm<0xc0)) 
            {
                buf+=sprintf(buf,"%s%08lx",needplus?"+":"",*(long *)sptr);
                sptr+=4;
            }

            *buf++=']';
            *buf=0;
        } 
        else 
        {
            switch(modrm & 0xc0) 
            {
            case 0x00:
                ptr_insert = buf;
                if ((modrm & 7) == 5) 
                {
                    sprintf(buf,"[0%08lxh]",*(long *)sptr);
                    sptr+=4;
                } 
                else 
                {
                    sprintf(buf,"[%s]",reg32[modrm&7]);
                    ADDRESS_ACCESS_32(modrm&7);
                }
                break;
            case 0x40:
                ptr_insert = buf;
                sprintf(buf,"[%s%+d]",reg32[modrm&7], (signed char)*sptr++);
                ADDRESS_ACCESS_32(modrm&7);
                break;
            case 0x80:
                ptr_insert = buf;
                sprintf(buf,"[%s+0%lxh]",reg32[modrm&7],*(long *)sptr);
                ADDRESS_ACCESS_32(modrm&7);
                sptr += 4;
                break;
            case 0xc0:
                strcpy(buf,regs[modrm&7]);
                DATA_ACCESS_32(modrm&7);
                break;
            }
        }
    }

    while(*buf) ++buf;
}

static unsigned char *disasm_inst(unsigned char *sptr, unsigned char *sbase, char *buf, BOOL is_32) 
{
    unsigned char op;
    BOOL is_32_addr = is_32;
    BOOL is_32_op = is_32;
    const struct x86op *opdata;
    char *segment_override="";
    char *large_ptr_type, *ptr_ptr_type;
    long flags;
    const char *const *regx;

    unsigned char esave;

    regmask_modify = regmask_access = regmask_address=0;
    ptr_insert = NULL;

    for(;;) 
    {
        op = *sptr++;

        // Special floating point handling for op=D8-DF, modrm>=0xc0

        if ((op&0xf8) == 0xd8 && *sptr >= 0xc0) 
        {
            const char *opcode, *operand_format;
            unsigned char modrm = *sptr++;

            opcode = fpext_reg_tab[op-0xd8][(modrm - 0xc0)>>3][0];
            operand_format = fpext_reg_tab[op-0xd8][(modrm - 0xc0)>>3][1];

            if (opcode) 
            {
                buf += sprintf(buf, "%s ", opcode);
                buf += sprintf(buf, operand_format, modrm&7);
            } 
            else
            {
                if (operand_format)
                {
                    for(int i=0; i<(modrm&7); i++)
                    {
                        while(*operand_format++)
                        {
                            ;
                        }
                    }
                }

                if (!operand_format || !*operand_format)
                {
                    buf += sprintf(buf, "db\t%02x,%02x", opcode, operand_format);
                }
                else
                {
                    buf += sprintf(buf, operand_format);
                }
            }
            break;
        }


        opdata = &singops[op];

        flags = opdata->flags;

        if (GETTYPE(flags) == TYPE_0f) 
        {
            op = *sptr++;
            opdata = &prefix0f_ops[op];
            flags = opdata->flags;
        }

        if (flags == SPECIAL_3DNOW) 
        {
            int i = 0;

            op = *sptr++;

            while(group_3DNow[i].opcode) 
            {
                if (group_3DNow[i].opcode == op) 
                {
                    opdata = &group_3DNow[i].opdata;
                    flags = opdata->flags;
                    break;
                }
                ++i;
            }
        }
        else if (GETGROUP(flags)) 
        {
            opdata = &groups[GETGROUP(flags)-1][(*sptr>>3)&7];
            flags |= opdata->flags;
        }

        PIPE_FORCE_U();     // Pentium: all prefixes except 0fh force u-pipe

        switch(GETTYPE(flags)) 
        {
        case TYPE_cs_override:  
            segment_override = "cs:"; 
            continue;
        case TYPE_ds_override:  
            segment_override = "ds:"; 
            continue;
        case TYPE_ss_override:  
            segment_override = "ss:"; 
            continue;
        case TYPE_es_override:  
            segment_override = "es:"; 
            continue;
        case TYPE_fs_override:  
            segment_override = "fs:"; 
            continue;
        case TYPE_gs_override:  
            segment_override = "gs:"; 
            continue;
        case TYPE_operand_size_override: 
            is_32_op = !is_32_op; 
            continue;
        case TYPE_address_size_override: 
            is_32_addr = !is_32_addr; 
            continue;
        case TYPE_prefix:
            strcpy(buf, opdata->name);
            while(*buf) ++buf;
            *buf++=' ';
            continue;
        default:
            PIPE_RESET_UV();
            break;
        }

        if (opdata->name) 
        {
            strcpy(buf, opdata->name);
            while(*buf) ++buf;
        }

        if (!is_32_op) switch(GETMOD(opdata->flags)) 
        {
            case MOD16_take_d_off:
                --buf;
                break;
            case MOD16_d_to_w:
                buf[-1] = 'w';
                break;
            case MOD16_dq_to_wd:
                buf[-2] = 'w';
                buf[-1] = 'd';
                break;
        }

        esave = sptr[0];

        regx = is_32_op ? reg32 : reg16;
        large_ptr_type = is_32_addr ? "dword ptr" : "word ptr";
        ptr_ptr_type = is_32_addr ? "fword ptr" : "dword ptr";

        for(int i=0; i<3; i++) 
        {
            int amode = (flags>>(6*i)) & 63;

            if (!amode) 
            {
                continue;
            }

            if (amode==ADDR_Idep)
            {
                amode=op&1 ? ADDR_Iv : ADDR_Ib;
            }

            if (i)
            {
                *buf++=','; 
            }
            else
            {
                *buf++=' ';
            }

            switch(amode)
            {
            case ADDR_AL:   
                buf=strtack(buf,reg8[0]);           
                DATA_ACCESS_32(0);          
                break;
            case ADDR_CL:   
                buf=strtack(buf,reg8[1]);           
                DATA_ACCESS_32(1);          
                break;
            case ADDR_DX:   
                buf=strtack(buf,reg16[2]);      
                DATA_ACCESS_32(2);          
                break;
            case ADDR_EAX:  
                buf=strtack(buf,regx[0]);           
                DATA_ACCESS_32(0);          
                break;
            case ADDR_CS:   
                buf=strtack(buf,"cs");              
                DATA_ACCESS_SEGMENT(1);     
                break;
            case ADDR_DS:   
                buf=strtack(buf,"ds");              
                DATA_ACCESS_SEGMENT(3);     
                break;
            case ADDR_SS:   
                buf=strtack(buf,"ss");              
                DATA_ACCESS_SEGMENT(2);     
                break;
            case ADDR_ES:   
                buf=strtack(buf,"es");              
                DATA_ACCESS_SEGMENT(0);     
                break;
            case ADDR_FS:   
                buf=strtack(buf,"fs");              
                DATA_ACCESS_SEGMENT(4);     
                break;
            case ADDR_GS:   
                buf=strtack(buf,"gs");              
                DATA_ACCESS_SEGMENT(5);     
                break;
            case ADDR_1:    
                *buf++='1'; 
                break;
            case ADDR_Ap:
                if (is_32_addr) 
                {
                    buf+=sprintf(buf,"%04x:%08lx",*(unsigned short *)(sptr+4), *(unsigned long *)sptr);
                    sptr += 6;
                } 
                else 
                {
                    buf+=sprintf(buf,"%04x:%04x",*(unsigned short *)(sptr+2), *(unsigned short *)sptr);
                    sptr += 4;
                }
                break;
            case ADDR_Cd:   
                buf[0]='c'; 
                buf[1]='r'; 
                buf[2]='0'+((*sptr++)&7);
                break;
            case ADDR_Dd:   
                buf[0]='d'; 
                buf[1]='r'; 
                buf[2]='0'+((*sptr++)&7); 
                break;
            case ADDR_Eb:   
                disasm_modrm(sptr, FALSE, buf, is_32_op, is_32_addr, "byte ptr ", segment_override);
                break;
            case ADDR_Ep:   
                disasm_modrm(sptr, TRUE, buf, is_32_op, is_32_addr, ptr_ptr_type, segment_override);
                break;
            case ADDR_Ev:   
                disasm_modrm(sptr, TRUE, buf, is_32_op, is_32_addr, large_ptr_type, segment_override);
                break;
            case ADDR_Ew:   
                disasm_modrm(sptr, TRUE, buf, FALSE, is_32_addr, "word ptr ", segment_override);
                break;
            case ADDR_Ed:   
                disasm_modrm(sptr, TRUE, buf, TRUE, is_32_addr, "dword ptr ", segment_override);
                break;
            case ADDR_Eq:   
                disasm_modrm(sptr, TRUE, buf, TRUE, is_32_addr, "qword ptr ", segment_override);
                break;
            case ADDR_Ex:   
                disasm_modrm(sptr, TRUE, buf, TRUE, is_32_addr, "tbyte ptr ", segment_override);
                break;
            case ADDR_Gb:   
                buf=strtack(buf,reg8[(esave>>3)&7]);    
                DATA_ACCESS_32((esave>>3)&7);
                break;
            case ADDR_Gv:   
                buf=strtack(buf,regx[(esave>>3)&7]);    
                DATA_ACCESS_32((esave>>3)&7);   
                break;
            case ADDR_Ib:   
                buf+=sprintf(buf,"%02x",*sptr++);
                break;
            case ADDR_Ib2:  
                buf+=sprintf(buf,"%d",sptr[1]);
                sptr += 2;
                break;
            case ADDR_Ibs:  
                if (is_32_op) 
                {
                    buf+=sprintf(buf,"%08lx",(long)(signed char)*sptr++);
                }
                else 
                {
                    buf+=sprintf(buf,"%04x",(unsigned short)(signed char)*sptr++);
                }
                break;
            case ADDR_Iv:   
                if (is_32_op) 
                {
                    buf+=sprintf(buf,"%08lx",*(long *)sptr);
                    sptr+=4;
                    break;
                }
            case ADDR_Iw:   
                buf+=sprintf(buf,"%04x",*(unsigned short *)sptr);
                sptr+=2;
                break;
            case ADDR_Jb:   
                ++sptr;
                if (is_32_addr)
                {
                    buf+=sprintf(buf,"%lx",(sptr-sbase) + (long)(signed char)sptr[-1]);
                }
                else
                {
                    buf+=sprintf(buf,"%x",(unsigned short)((sptr-sbase) + (long)(signed char)sptr[-1]));
                }
                break;
            case ADDR_Jv:
                if (is_32_addr) 
                {
                    sptr += 4;
                    buf+=sprintf(buf,"%lx",(sptr-sbase) + *(long *)(sptr-4));
                } 
                else
                {
                    sptr += 2;
                    buf+=sprintf(buf,"%x",(unsigned short)((sptr-sbase) + (long)*(signed short *)(sptr-2)));
                }
                break;
            case ADDR_M:    
                disasm_modrm(sptr, TRUE, buf, is_32_op, is_32_addr, "", segment_override);
                break;
            case ADDR_Ma:   
                disasm_modrm(sptr, TRUE, buf, is_32_op, is_32_addr, large_ptr_type, segment_override);
                break;
            case ADDR_Mp:   
                disasm_modrm(sptr, TRUE, buf, is_32_op, is_32_addr, ptr_ptr_type, segment_override);
                break;
            case ADDR_Ob:   
                if (is_32_addr) 
                {
                    buf += sprintf(buf,"byte ptr [%08lx]",*(long *)sptr);
                    sptr += 4;
                }
                else 
                {
                    buf += sprintf(buf,"byte ptr [%04x]",*(unsigned short *)sptr);
                    sptr += 2;
                }
                break;
            case ADDR_Ov:   
                if (is_32_addr) 
                {
                    buf += sprintf(buf,"%s [%08lx]",large_ptr_type,*(long *)sptr);
                    sptr += 4;
                }
                else 
                {
                    buf += sprintf(buf,"%s [%04x]",large_ptr_type,*(unsigned short *)sptr);
                    sptr += 2;
                }
                break;
            case ADDR_Pd:   // fall through
            case ADDR_Pq:   
                buf[0]='m'; 
                buf[1]='m'; 
                buf[2]='0'+((esave>>3)&7); 
                buf += 3; 
                DATA_ACCESS_MMX((esave>>3)&7);
                break;
            case ADDR_Pq2:  
                buf[0]='m'; 
                buf[1]='m'; 
                buf[2]='0'+(*sptr&7); 
                buf += 3; 
                DATA_ACCESS_MMX(*sptr&7);
                break;
            case ADDR_Qd:   // fall through
            case ADDR_Qq:   
                disasm_modrm(sptr, 2, buf, is_32_op, is_32_addr, "", segment_override);
                break;
            case ADDR_Rd:   
                buf = strtack(buf, reg32[*sptr++ & 7]);             
                DATA_ACCESS_32(sptr[-1]&7);     
                break;
            case ADDR_Sw:   
                buf = strtack(buf, sregs[(esave>>3)&7]); 
                break;
            case ADDR_Xb:   
                buf+=sprintf(buf,"%sbyte ptr [%s]",segment_override,regx[6]);           
                DATA_ACCESS_32(6);  
                break;
            case ADDR_Xv:   
                buf+=sprintf(buf,"%s%s [%s]",segment_override,large_ptr_type,regx[6]);  
                DATA_ACCESS_32(6);  
                break;
            case ADDR_Yb:   
                buf+=sprintf(buf,"%sbyte ptr [%s]",segment_override,regx[7]);           
                DATA_ACCESS_32(7);
                break;
            case ADDR_Yv:   
                buf+=sprintf(buf,"%s%s [%s]",segment_override,large_ptr_type,regx[7]);  
                DATA_ACCESS_32(7);  
                break;
            case ADDR_reg8: 
                buf = strtack(buf, reg8[op&7]);                     
                DATA_ACCESS_32(op&7);           
                break;
            case ADDR_reg32: 
                buf = strtack(buf, regx[op&7]);                 
                DATA_ACCESS_32(op&7);           
                break;
            }

            if (!i) 
            {
                regmask_modify = regmask_access;
                regmask_access = 0;
            }
        }
        break;
    }
    buf[0]=0;

    if (!((regmask_modify|regmask_access) & 0xff) && ptr_insert) 
    {
        memmove(ptr_insert + strlen(ptr_insert_type), ptr_insert, strlen(ptr_insert)+1);
        memcpy(ptr_insert, ptr_insert_type, strlen(ptr_insert_type));
    }

    return sptr;
}

/////////////////////////////////////////////////////////////////////////

#define PENALTY_RAW         (0x00000001L)
#define PENALTY_WAW         (0x00000002L)
#define PENALTY_AGI         (0x00000004L)

void CCodeDisassemblyWindow::parse() 
{
    unsigned char *ip = (unsigned char *)code, *ip_start;
    unsigned char *ipend = ip + length;
    lbent *ipd = lbents, *ipd_last = NULL;
    long regmask_last_clock_modify = 0;
    long regmask_last_inst_modify = 0;
    long penalty_flags;
    int cnt=0;

    if (!ipd) 
    {
        num_ents = 0;
        return;
    }

    while(ip < ipend && cnt++ < MAX_INSTRUCTIONS) 
    {
        ip_start = ip;

        ip = disasm_inst(ip, (unsigned char *)code, buf, TRUE);

        // penalties?

        penalty_flags = 0;

        if (regmask_last_clock_modify & regmask_address)
        {
            penalty_flags |= PENALTY_AGI;
        }

        if (ipd_last && pipe_force!=PIPE_U) 
        {
            if (regmask_last_inst_modify & regmask_address)
            {
                penalty_flags |= PENALTY_AGI | PENALTY_RAW;
            }

            if (regmask_last_inst_modify & regmask_access)
            {
                penalty_flags |= PENALTY_RAW;
            }

            if (regmask_last_inst_modify & regmask_modify)
            {
                penalty_flags |= PENALTY_WAW;
            }
        }

        // pairable in V-pipe?

        if (ipd_last && pipe_force!=PIPE_U && !(penalty_flags & (PENALTY_RAW | PENALTY_WAW))) 
        {
            ipd_last->ip_v = ip_start;
            ipd_last->flags |= penalty_flags;
            ipd_last = NULL;

            regmask_last_clock_modify |= regmask_modify;
        } 
        else 
        {
            // pairable in U-pipe?
            if (pipe_force != PIPE_V) 
            {
                ipd->ip_u = ip_start;
                ipd->ip_v = NULL;
                ipd_last = ipd;
                regmask_last_inst_modify = regmask_modify;
            }
            else 
            {
                ipd->ip_u = NULL;
                ipd->ip_v = ip_start;
                ipd_last = NULL;
                regmask_last_inst_modify = 0;
            }
            ipd->flags = penalty_flags;

            ++ipd;

            regmask_last_clock_modify = regmask_modify;
        }
    }

    num_ents = ipd-lbents;
}

char *CCodeDisassemblyWindow::penalty_string(long f) 
{
    char *bp = buf;

    if (f & PENALTY_RAW)
    {
        bp = strtack(bp, "RAW");
    }

    if (f & PENALTY_WAW) 
    {
        if (bp>buf)
        {
            bp=strtack(bp,", ");
        }
        bp = strtack(bp, "WAW");
    }

    if (f & PENALTY_AGI) 
    {
        if (bp>buf)
        {
            bp=strtack(bp,", ");
        }
        bp = strtack(bp, "AGI");
    }

    *bp=0;

    return buf;
}

long CCodeDisassemblyWindow::getInstruction(char *buf, long val) 
{
    lbent *ipd;
    unsigned char *ip;

    if ((val>>1) >= num_ents)
    {
        return 0;
    }

    ipd = &lbents[val>>1];
    ip = val&1 ? ipd->ip_v : ipd->ip_u;

    if (!ip) 
    {
        val |= 1;
        ip = ipd->ip_v;
    }

    wsprintf(buf, "%08lx: ", ip - (unsigned char *)code + (unsigned char *)abase);
    disasm_inst(ip, (unsigned char *)rbase, buf+10, TRUE);

    if (ip - (unsigned char *)code + (unsigned char *)abase == pFault)
    {
        strcat(buf, "      <-- FAULT");
    }

    ++val;

    if (val&1 && !ipd->ip_v)
    {
        ++val;
    }

    return val;
}

void CCodeDisassemblyWindow::DoInitListbox(HWND hwndList) 
{
    SendMessage(hwndList, LB_SETCOUNT, num_ents, 0);

    if (hFontMono)
    {
        SendMessage(hwndList, WM_SETFONT, (WPARAM)hFontMono, MAKELPARAM(TRUE, 0));
    }
}

BOOL CCodeDisassemblyWindow::DoMeasureItem(LPARAM lParam)
{
    if (((LPMEASUREITEMSTRUCT)lParam)->CtlType != ODT_LISTBOX)
    {
        return FALSE;
    }
    ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = 23;
    return TRUE;
}

BOOL CCodeDisassemblyWindow::DoDrawItem(LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
    lbent *ipd;
    HBRUSH hbrBack;
    HPEN hPenOld;

    if (lpdis->CtlType != ODT_LISTBOX)
    {
        return FALSE;
    }
    if (!(lpdis->itemAction & ODA_DRAWENTIRE))
    {
        return FALSE;
    }
    if (lpdis->itemID < 0)
    {
        return FALSE;
    }

    ipd = &lbents[lpdis->itemID];

    if (hbrBack = CreateSolidBrush(RGB(0xe8,0xff,0xe8))) 
    {
        RECT r;

        r.left = lpdis->rcItem.left;
        r.top = lpdis->rcItem.top+11;
        r.right = lpdis->rcItem.right;
        r.bottom = lpdis->rcItem.bottom;

        FillRect(lpdis->hDC, &r, hbrBack);
        DeleteObject(hbrBack);
    }
    if (hbrBack = CreateSolidBrush(RGB(0xe8,0xe8,0xff))) 
    {
        RECT r;

        r.left = lpdis->rcItem.left;
        r.top = lpdis->rcItem.top;
        r.right = lpdis->rcItem.right;
        r.bottom = lpdis->rcItem.top+11;

        FillRect(lpdis->hDC, &r, hbrBack);

        DeleteObject(hbrBack);
    }

    SetBkMode(lpdis->hDC, TRANSPARENT);
    SetTextColor(lpdis->hDC, RGB(0x00,0x00,0x00));

    if (ipd->ip_u) 
    {
        wsprintf(buf, "%08lx: ", ipd->ip_u - (unsigned char *)code + (unsigned char *)abase);
        disasm_inst(ipd->ip_u, (unsigned char *)rbase, buf+10, TRUE);

        TabbedTextOut(  
                        lpdis->hDC,
                        lpdis->rcItem.left,
                        lpdis->rcItem.top,
                        buf,
                        strlen(buf),
                        0,
                        NULL,
                        0
                     );

    }
    else if (ipd->ip_v) 
    {
        wsprintf(buf, "%08lx: ", ipd->ip_v - (unsigned char *)code + (unsigned char *)abase);
        disasm_inst(ipd->ip_v, (unsigned char *)rbase, buf+10, TRUE);

        TabbedTextOut(  lpdis->hDC,
                        lpdis->rcItem.left,
                        lpdis->rcItem.top+11,
                        buf,
                        strlen(buf),
                        0,
                        NULL,
                        0
                        );
    }

    if (ipd->ip_u - (unsigned char *)code + (unsigned char *)abase == pFault)
    {
        strtack(buf, "**FAULT**");
        TextOut(lpdis->hDC,
                (lpdis->rcItem.left+3*lpdis->rcItem.right)/4,
                lpdis->rcItem.top+2,
                buf,
                strlen(buf)
                );
    }
    else if (ipd->flags) 
    {
        penalty_string(ipd->flags);             
        TextOut(lpdis->hDC,
                (lpdis->rcItem.left+3*lpdis->rcItem.right)/4,
                lpdis->rcItem.top+2,
                buf,
                strlen(buf)
                );
    }

    hPenOld = (HPEN)SelectObject(lpdis->hDC, GetStockObject(BLACK_PEN));
    MoveToEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.bottom-1, NULL);
    LineTo(lpdis->hDC, lpdis->rcItem.right, lpdis->rcItem.bottom-1);
    DeleteObject(SelectObject(lpdis->hDC, hPenOld));

    return TRUE;
}

BOOL CALLBACK CCodeDisassemblyWindow::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    CCodeDisassemblyWindow *thisPtr = (CCodeDisassemblyWindow *)GetWindowLong(hDlg, DWL_USER);

    switch(msg) 
    {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        thisPtr = (CCodeDisassemblyWindow *)lParam;

        thisPtr->DoInitListbox(GetDlgItem(hDlg, IDC_ASMBOX));
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
        case IDCANCEL: 
        case IDOK:
            EndDialog(hDlg, FALSE);
            return TRUE;
        }
        break;

    case WM_MEASUREITEM:
        return thisPtr->DoMeasureItem(lParam);

    case WM_DRAWITEM:
        return thisPtr->DoDrawItem(lParam);
    }

    return FALSE;
}
