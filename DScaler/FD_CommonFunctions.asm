;////////////////////////////////////////////////////////////////////////////
; FD_CommonFunctions.asm
;////////////////////////////////////////////////////////////////////////////
; Copyright (c) 2000 John Adcock. All rights reserved.
;////////////////////////////////////////////////////////////////////////////
;
;	This file is subject to the terms of the GNU General Public License as
;	published by the Free Software Foundation.  A copy of this license is
;	included with this software distribution in the file COPYING.  If you
;	do not have a copy, you may obtain a copy by writing to the Free
;	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
;
;	This software is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details
;////////////////////////////////////////////////////////////////////////////
;
; Refinements made by Mark Rejhon and Steve Grimm
;
;////////////////////////////////////////////////////////////////////////////
; Change Log
;
; Date          Developer             Changes
;
; 01 Jun 2001   John Adcock           Split out into new file
;
;////////////////////////////////////////////////////////////////////////////

	.586
	.mmx
	.xmm
	.model	flat

extern _qwThreshold:qword
extern _qwYMask:qword
extern _qwOnes:qword
extern _qwBitShift:qword

    .code
;////////////////////////////////////////////////////////////////////
; WORD CalcCombFactorLineChroma(short* YVal1, short* YVal2, short* YVal3, 
;		long BytesToProcess);
;////////////////////////////////////////////////////////////////////
	
public _CalcCombFactorLine

	YVal1	equ	[esp+4+16]
	YVal2	equ	[esp+8+16]
	YVal3	equ	[esp+12+16]
	BytesToProcess	equ	[esp+16+16]

_CalcCombFactorLine:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov ecx, BytesToProcess
	mov eax,dword ptr [YVal1]
	mov ebx,dword ptr [YVal2]
	mov edx,dword ptr [YVal3]
	shr ecx, 3       ; there are BytesToProcess / 8 qwords
	movq mm0, _qwThreshold
	movq mm1, _qwYMask
	movq mm2, _qwOnes
	pxor mm7, mm7    ; mm0 = 0
align 8
Comb_Loop:
	movq mm3, qword ptr[eax]  ; mm3 = O1
	movq mm4, qword ptr[ebx]  ; mm4 = E
	movq mm5, qword ptr[edx]  ; mm5 = O2

	pand mm3, mm1
	pand mm4, mm1
	pand mm5, mm1

	; work out (O1 - E) * (O2 - E) - (O1 - O2) ^ 2 >> 7
	; result will be in mm6

	psrlw mm3, 01
	psrlw mm4, 01
	psrlw mm5, 01

    ; mm6 = (O1 - E)
    movq mm6, mm3
	psubw mm6, mm4

    ; mm3 = (O1 - O2)
	psubw mm3, mm5

    ; mm5 = (O2 - E)
	psubw mm5, mm4

    ; mm6 = (O1 - E) * (O2 - E)
	pmullw mm6, mm5

    ; mm3 = (O1 - O2)^2 >> 7
	pmullw mm3, mm3		; mm7 = (O1 - O2) ^ 2
	psrlw mm3, 7		; mm7 = (O1 - O2) ^ 2 >> 7

	psubw mm6, mm3      ; mm6 is what we want

    ; FF's if greater than qwThreshold
	pcmpgtw mm6, mm0

    ; add to count if we are greater than threshold
	pand mm6, mm2
	paddw mm7, mm6

	add eax, 8
	add ebx, 8
	add edx, 8

	dec ecx
	jne Comb_Loop

	movd eax, mm7
	psrlq mm7,32
	movd ecx, mm7
	add ecx, eax
    mov ax, cx
    shr ecx, 16
    add ax, cx

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

;////////////////////////////////////////////////////////////////////
; WORD CalcCombFactorLineChroma(short* YVal1, short* YVal2, short* YVal3, 
;		long BytesToProcess);
;////////////////////////////////////////////////////////////////////

public _CalcCombFactorLineChroma

	YVal1	equ	[esp+4+16]
	YVal2	equ	[esp+8+16]
	YVal3	equ	[esp+12+16]
	BytesToProcess	equ	[esp+16+16]

_CalcCombFactorLineChroma:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov ecx, BytesToProcess
	mov eax, dword ptr [YVal1]
	mov ebx, dword ptr [YVal2]
	mov edx, dword ptr [YVal3]
	shr ecx, 3       ; there are BytesToProcess / 8 qwords
	pxor mm7, mm7    ; mm0 = 0
align 8
CombChroma_Loop:
	movq mm0, qword ptr[eax]   ; mm0 = O1
	movq mm1, qword ptr[ebx]   ; mm1 = E
	movq mm2, qword ptr[edx]   ; mm2 = O2
	movq mm3, mm0              ; mm3 = O1
	movq mm4, mm1              ; mm4 = E
	movq mm5, mm2              ; mm5 = O2

	pand mm3, _qwYMask
	pand mm4, _qwYMask
	pand mm5, _qwYMask

	; work out (O1 - E) * (O2 - E) - EdgeDetect * (O1 - O2) ^ 2 >> 12
	; result will be in mm6

	psrlw mm3, 01
    psrlw mm0, 9
	psrlw mm4, 01
    psrlw mm1, 9
	psrlw mm5, 01
    psrlw mm2, 9

    ; mm6 = (O1 - E)
    movq mm6, mm3
	psubw mm6, mm4

    ; mm3 = (O1 - O2)
	psubw mm3, mm5

    ; mm5 = (O2 - E)
	psubw mm5, mm4

    ; mm6 = (O1 - E) * (O2 - E)
	pmullw mm6, mm5

    ; mm3 = (O1 - O2)^2 >> 7
	pmullw mm3, mm3		; mm7 = (O1 - O2) ^ 2
	psrlw mm3, 7		; mm7 = (O1 - O2) ^ 2 >> 7

	psubw mm6, mm3      ; mm6 is what we want

    ; FF's if greater than qwThreshold
	pcmpgtw mm6, _qwThreshold
    
    ; add to count if we are greater than threshold
    pand mm6, _qwOnes
	paddw mm7, mm6

    ; mm6 = (O1 - E)
    movq mm6, mm0
	psubw mm6, mm1

    ; mm0 = (O1 - O2)
	psubw mm0, mm2

    ; mm2 = (O2 - E)
	psubw mm2, mm1

    ; mm6 = (O1 - E) * (O2 - E)
	pmullw mm6, mm2

    ; mm3 = (O1 - O2)^2 >> 7
	pmullw mm0, mm0		; mm7 = (O1 - O2) ^ 2
	psrlw mm0, 7		; mm7 = (O1 - O2) ^ 2 >> 12

	psubw mm6, mm0      ; mm6 is what we want

    ; FF's if greater than qwThreshold
	pcmpgtw mm6, _qwThreshold

    ; add to count if we are greater than threshold
    pand mm6, _qwOnes
	paddw mm7, mm6

	add eax, 8
	add ebx, 8
	add edx, 8

	dec ecx
	jne CombChroma_Loop

	movd eax, mm7
	psrlq mm7,32
	movd ecx, mm7
	add ecx, eax
    mov ax, cx
    shr ecx, 16
    add ax, cx
	shr ax, 1

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

;////////////////////////////////////////////////////////////////////
; DWORD CalcDiffFactorLine(short* YVal1, short* YVal2, long BytesToProcess);
;////////////////////////////////////////////////////////////////////

public _CalcDiffFactorLine

	YVal1	equ	[esp+4+16]
	YVal2	equ	[esp+8+16]
	BytesToProcess	equ	[esp+12+16]

_CalcDiffFactorLine:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov ecx, BytesToProcess
	mov eax,dword ptr [YVal1]
	mov ebx,dword ptr [YVal2]
	shr ecx, 3		 ; there are BytesToProcess / 8 qwords
	movq mm1, _qwYMask
	movq mm7, _qwBitShift
	pxor mm0, mm0    ; mm0 = 0  this is running total
align 8
Diff_Loop:
	movq mm4, qword ptr[eax] 
	movq mm5, qword ptr[ebx] 
	pand mm5, mm1    ; get only Y compoment
	pand mm4, mm1    ; get only Y compoment

	psubw mm4, mm5   ; mm4 = Y1 - Y2
	pmaddwd mm4, mm4 ; mm4 = (Y1 - Y2) ^ 2
	psrld mm4, mm7   ; divide mm4 by 2 ^ Bitshift
	paddd mm0, mm4   ; keep total in mm0

	add eax, 8
	add ebx, 8
	
	dec ecx
	jne Diff_Loop

	movd ecx, mm0
	psrlq mm0,32
	movd eax, mm0
	add eax, ecx

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret	

;////////////////////////////////////////////////////////////////////
; DWORD CalcDiffFactorLineChroma(short* YVal1, short* YVal2, long BytesToProcess);
;////////////////////////////////////////////////////////////////////

public _CalcDiffFactorLineChroma

	YVal1	equ	[esp+4+16]
	YVal2	equ	[esp+8+16]
	BytesToProcess	equ	[esp+12+16]

_CalcDiffFactorLineChroma:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov ecx, BytesToProcess
	mov eax,dword ptr [YVal1]
	mov ebx,dword ptr [YVal2]
	shr ecx, 3		 ; there are BytesToProcess / 8 qwords
	movq mm1, _qwYMask
	movq mm0, _qwBitShift
	pxor mm6, mm6    ; mm0 = 0  this is running total
	pxor mm7, mm7    ; mm0 = 0  this is running total
align 8
DiffChroma_Loop:
	movq mm2, qword ptr[eax] 
	movq mm3, qword ptr[ebx] 
	movq mm4, mm2 
	movq mm5, mm3
	pand mm4, mm1    ; get only Y compoment
	pand mm5, mm1    ; get only Y compoment
    psrlw mm2, 8    ; get only U compoment
    psrlw mm3, 8    ; get only U compoment

	psubw mm4, mm5   ; mm4 = Y1 - Y2
	psubw mm2, mm3   ; mm4 = U1 - U2
	pmaddwd mm4, mm4 ; mm4 = (Y1 - Y2) ^ 2
	pmaddwd mm2, mm2 ; mm2 = (U1 - U) ^ 2
	psrld mm4, mm0   ; divide mm4 by 2 ^ Bitshift
	psrld mm2, mm0   ; divide mm2 by 2 ^ Bitshift
	paddd mm6, mm4   ; keep total in mm6
	paddd mm7, mm2   ; keep total in mm7

	add eax, 8
	add ebx, 8
	
	loop DiffChroma_Loop

    ; add the two running totals
    paddd mm6, mm7
	movd ecx, mm6
	psrlq mm6, 32
	movd eax, mm6
	add eax, ecx

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret	

;////////////////////////////////////////////////////////////////////
; void memcpyBOBMMX(void *Dest1, void *Dest2, void *Src, size_t nBytes);
;////////////////////////////////////////////////////////////////////

public _memcpyBOBMMX

	Dest1	equ	[esp+4+16]
	Dest2	equ	[esp+8+16]
	Src	equ	[esp+12+16]
	nBytes	equ	[esp+16+16]

_memcpyBOBMMX:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov		esi, dword ptr[Src]
	mov		edi, dword ptr[Dest1]
	mov     ebx, dword ptr[Dest2]
	mov		ecx, nBytes
	shr     ecx, 6                      ; nBytes / 64
align 8
memcpyBOB_Loop:
	movq	mm0, qword ptr[esi]
	movq	mm1, qword ptr[esi+8*1]
	movq	mm2, qword ptr[esi+8*2]
	movq	mm3, qword ptr[esi+8*3]
	movq	mm4, qword ptr[esi+8*4]
	movq	mm5, qword ptr[esi+8*5]
	movq	mm6, qword ptr[esi+8*6]
	movq	mm7, qword ptr[esi+8*7]
	movq	qword ptr[edi], mm0
	movq	qword ptr[edi+8*1], mm1
	movq	qword ptr[edi+8*2], mm2
	movq	qword ptr[edi+8*3], mm3
	movq	qword ptr[edi+8*4], mm4
	movq	qword ptr[edi+8*5], mm5
	movq	qword ptr[edi+8*6], mm6
	movq	qword ptr[edi+8*7], mm7
	movq	qword ptr[ebx], mm0
	movq	qword ptr[ebx+8*1], mm1
	movq	qword ptr[ebx+8*2], mm2
	movq	qword ptr[ebx+8*3], mm3
	movq	qword ptr[ebx+8*4], mm4
	movq	qword ptr[ebx+8*5], mm5
	movq	qword ptr[ebx+8*6], mm6
	movq	qword ptr[ebx+8*7], mm7
	add		esi, 64
	add		edi, 64
	add		ebx, 64
	loop memcpyBOB_Loop

	mov		ecx, nBytes
	and     ecx, 63
	shr     ecx, 2
	cmp     ecx, 0
	je memcpyBOB_End

align 8
memcpyBOB_Loop2:
	mov edx, [esi] 
	mov [edi], edx
	mov [ebx], edx
	add esi, 4
	add edi, 4
	add ebx, 4
	loop memcpyBOB_Loop2

memcpyBOB_End:
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret	

;////////////////////////////////////////////////////////////////////
; void memcpyBOBSSE(void *Dest1, void *Dest2, void *Src, size_t nBytes);
;////////////////////////////////////////////////////////////////////

public _memcpyBOBSSE

	Dest1	equ	[esp+4+16]
	Dest2	equ	[esp+8+16]
	Src	equ	[esp+12+16]
	nBytes	equ	[esp+16+16]

_memcpyBOBSSE:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov		esi, dword ptr[Src]
	mov		edi, dword ptr[Dest1]
	mov     ebx, dword ptr[Dest2]
	mov		ecx, nBytes
	shr     ecx, 7                      ; nBytes / 128
align 8
memcpyBOBSSE_Loop:
	movaps	xmm0, [esi]
	movaps	xmm1, [esi+16*1]
	movaps	xmm2, [esi+16*2]
	movaps	xmm3, [esi+16*3]
	movaps	xmm4, [esi+16*4]
	movaps	xmm5, [esi+16*5]
	movaps	xmm6, [esi+16*6]
	movaps	xmm7, [esi+16*7]
	movntps	[edi], xmm0
	movntps	[edi+16*1], xmm1
	movntps	[edi+16*2], xmm2
	movntps	[edi+16*3], xmm3
	movntps	[edi+16*4], xmm4
	movntps	[edi+16*5], xmm5
	movntps	[edi+16*6], xmm6
	movntps	[edi+16*7], xmm7
	movntps	[ebx], xmm0
	movntps	[ebx+16*1], xmm1
	movntps	[ebx+16*2], xmm2
	movntps	[ebx+16*3], xmm3
	movntps	[ebx+16*4], xmm4
	movntps	[ebx+16*5], xmm5
	movntps	[ebx+16*6], xmm6
	movntps	[ebx+16*7], xmm7
	add		esi, 128
	add		edi, 128
	add		ebx, 128
	loop memcpyBOBSSE_Loop

	mov		ecx, nBytes
	and     ecx, 127
	shr     ecx, 2
	cmp     ecx, 0
	je memcpyBOBSSE_End
align 8
memcpyBOBSSE_Loop2:
	mov edx, [esi] 
	mov [edi], edx
	mov [ebx], edx
	add esi, 4
	add edi, 4
	add ebx, 4
	loop memcpyBOBSSE_Loop2

memcpyBOBSSE_End:
	
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret	

;////////////////////////////////////////////////////////////////////
; void memcpyMMX(void *Dest, void *Src, size_t nBytes);
;
; Uses MMX instructions to move memory around
; does as much as we can in 64 byte chunks (128-byte on SSE machines)
; using MMX instructions
; then copies any extra bytes
; assumes there will be at least 64 bytes to copy
; This code was originally from Borg's bTV plugin SDK 
;////////////////////////////////////////////////////////////////////

public _memcpyMMX

	Dest	equ	[esp+12]
	Src	    equ	[esp+16]
	nBytes	equ	[esp+20]

_memcpyMMX:
	push	edi
	push	esi

	mov		esi, dword ptr[Src]
	mov		edi, dword ptr[Dest]
	mov		ecx, nBytes
	shr     ecx, 6                      ; nBytes / 64
align 8
Memcpy_Loop:
	movq	mm0, qword ptr[esi]
	movq	mm1, qword ptr[esi+8*1]
	movq	mm2, qword ptr[esi+8*2]
	movq	mm3, qword ptr[esi+8*3]
	movq	mm4, qword ptr[esi+8*4]
	movq	mm5, qword ptr[esi+8*5]
	movq	mm6, qword ptr[esi+8*6]
	movq	mm7, qword ptr[esi+8*7]
	movq	qword ptr[edi], mm0
	movq	qword ptr[edi+8*1], mm1
	movq	qword ptr[edi+8*2], mm2
	movq	qword ptr[edi+8*3], mm3
	movq	qword ptr[edi+8*4], mm4
	movq	qword ptr[edi+8*5], mm5
	movq	qword ptr[edi+8*6], mm6
	movq	qword ptr[edi+8*7], mm7
	add		esi, 64
	add		edi, 64
	loop Memcpy_Loop

	mov		ecx, nBytes
	and     ecx, 63
	cmp     ecx, 0
	je Memcpy_End
	rep movsb

Memcpy_End:
	pop	esi
	pop	edi
	ret	


;////////////////////////////////////////////////////////////////////
; void memcpySSE(void *Dest, void *Src, size_t nBytes);
;
; On SSE machines we use the 
; bypass write caching to copy a bit faster.  The destination has to be
; 16-byte aligned.  
;////////////////////////////////////////////////////////////////////
public _memcpySSE

	Dest	equ	[esp+12]
	Src	    equ	[esp+16]
	nBytes	equ	[esp+20]

_memcpySSE:
	push	edi
	push	esi

	mov		esi, dword ptr[Src]
	mov		edi, dword ptr[Dest]
	mov		ecx, nBytes
	shr     ecx, 7
align 4
MemcpySSE_Loop:
	; movaps should be slightly more efficient
	; as the data is 16 bit aligned
	movaps	xmm0, [esi]
	movaps	xmm1, [esi+16*1]
	movaps	xmm2, [esi+16*2]
	movaps	xmm3, [esi+16*3]
	movaps	xmm4, [esi+16*4]
	movaps	xmm5, [esi+16*5]
	movaps	xmm6, [esi+16*6]
	movaps	xmm7, [esi+16*7]
	movntps	[edi], xmm0
	movntps	[edi+16*1], xmm1
	movntps	[edi+16*2], xmm2
	movntps	[edi+16*3], xmm3
	movntps	[edi+16*4], xmm4
	movntps	[edi+16*5], xmm5
	movntps	[edi+16*6], xmm6
	movntps	[edi+16*7], xmm7
	add		esi, 128
	add		edi, 128
	dec ecx
	jnz MemcpySSE_Loop

	mov		ecx, nBytes
	and     ecx, 127
	cmp     ecx, 0
	je MemcpySSE_End
	rep movsb

MemcpySSE_End:
	pop	esi
	pop	edi
	ret	

;////////////////////////////////////////////////////////////////////
; void memcpyAMD(void *Dest, void *Src, size_t nBytes);
;
; Comes from AMD Site
;////////////////////////////////////////////////////////////////////

public _memcpyAMD

	Dest	equ	[esp+12]
	Src	    equ	[esp+16]
	nBytes	equ	[esp+20]

_memcpyAMD:
	push	edi
	push	esi

	mov		esi, dword ptr[Src]
	mov		edi, dword ptr[Dest]
	mov		ecx, nBytes
	shr     ecx, 6                      ; nBytes / 64
align 8
MemcpyAMD_Loop:
	movq mm0,[esi+0]
	movq mm1,[esi+8]
	movq [edi+0],mm0
	movq [edi+8],mm1
	movq mm2,[esi+16]
	movq mm3,[esi+24]
	movq [edi+16],mm2
	movq [edi+24],mm3
	movq mm0,[esi+32]
	movq mm1,[esi+40]
	movq [edi+32],mm0
	movq [edi+40],mm1
	movq mm2,[esi+48]
	movq mm3,[esi+56]
	movq [edi+48],mm2
	movq [edi+56],mm3
	add		esi, 64
	add		edi, 64
	loop MemcpyAMD_Loop

	mov ecx, nBytes
	shr ecx,2               ; dword count
	and ecx,1111b
	neg ecx
	add ecx, MemcpyAMD_End
	jmp ecx

align 4
	movsd
	movsd                   ; perform last 1-15 dword copies
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd                   ; perform last 1-7 dword copies
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd

MemcpyAMD_End:
	pop	esi
	pop	edi
	ret	
	
	end