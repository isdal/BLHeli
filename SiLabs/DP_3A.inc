;**** **** **** **** ****
;
; BLHeli program for controlling brushless motors in helicopters
;
; Copyright 2011, 2012 Steffen Skaug
; This program is distributed under the terms of the GNU General Public License
;
; This file is part of BLHeli.
;
; BLHeli is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; BLHeli is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with BLHeli.  If not, see <http://www.gnu.org/licenses/>.
;
;**** **** **** **** ****
;
; DP 3A hardware definition file
;
;**** **** **** **** ****



;*********************
; Device SiLabs F330
;*********************
$include (c8051f330.inc)

;**** **** **** **** ****
; Uses internal calibrated oscillator set to 24Mhz
;**** **** **** **** ****

;**** **** **** **** ****
; Constant definitions
;**** **** **** **** ****
CSEG AT 1A40h
Eep_ESC_Layout:		DB	"#DP3A#          "	; ESC layout tag
DAMPED_TAIL_ENABLE		EQU	1	; Damped tail mode enabled
NFETON_DELAY			EQU	2	; Wait delay from pfets off to nfets on
PFETON_DELAY			EQU	1	; Wait delay from nfets off to pfets on
ADC_LIMIT_L			EQU 	128	; Power supply measurement ADC value for which main motor power is limited (low byte)
ADC_LIMIT_H			EQU	2	; Power supply measurement ADC value for which main motor power is limited (2 MSBs)


;*********************
; PORT 0 definitions *
;*********************  
;			EQU	7	;i
Rcp_In		EQU	6	;i
Mux_A		EQU	5	;i
;			EQU	4	;i
Mux_B		EQU	3	;i
Comp_Com		EQU	2	;i
Mux_C		EQU	1	;i
;			EQU	0	;i

P0_DIGITAL	EQU	NOT((1 SHL Mux_A)+(1 SHL Mux_B)+(1 SHL Mux_C)+(1 SHL Comp_Com))
P0_INIT		EQU	0FFh
P0_PUSHPULL	EQU	0
P0_SKIP		EQU	NOT(1 SHL Rcp_In) AND 0FFh

MACRO Read_Rcp_Int
	mov	A, P0
	jnb	Flags2.PGM_RCP_PWM_POL, ($+4)	; Is pwm polarity negative?
	cpl	A						; Yes - invert
ENDM
MACRO Rcp_Int_Enable
	orl	PCA0CPM0, #01h				; Interrupt enabled
ENDM
MACRO Rcp_Int_Disable
	anl	PCA0CPM0, #0FEh			; Interrupt disabled
ENDM
MACRO Rcp_Int_First
	anl	PCA0CPM0, #0CFh			
	jb	Flags2.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity positive?
	orl	PCA0CPM0, #20h				; Capture rising edge
	jnb	Flags2.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity negative?
	orl	PCA0CPM0, #10h				; Capture falling edge
ENDM
MACRO Rcp_Int_Second
	anl	PCA0CPM0, #0CFh			
	jb	Flags2.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity positive?
	orl	PCA0CPM0, #10h				; Capture falling edge
	jnb	Flags2.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity negative?
	orl	PCA0CPM0, #20h				; Capture rising edge
ENDM
MACRO Rcp_Clear_Int_Flag
	clr 	CCF0						; Clear interrupt flag
ENDM		


;*********************
; PORT 1 definitions *
;*********************
Ref_Gnd		EQU	7	;o
Adc_Ip		EQU	6	;i
CpFET		EQU	5	;o
BpFET		EQU	4	;o
ApFET		EQU	3	;o
CnFET		EQU	2	;o
BnFET		EQU	1	;i
AnFET		EQU	0	;i

P1_DIGITAL	EQU	(1 SHL AnFET)+(1 SHL BnFET)+(1 SHL CnFET)+(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)+(1 SHL Ref_Gnd)
P1_INIT		EQU 	(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)+(1 SHL Adc_Ip)		; Setting pFET outputs turn them off
P1_PUSHPULL	EQU	(1 SHL AnFET)+(1 SHL BnFET)+(1 SHL CnFET)+(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)
P1_SKIP		EQU	0

MACRO AnFET_on
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.AnFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.CnFET
ENDM
MACRO AnFET_off
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.AnFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.CnFET
ENDM
MACRO BnFET_on
	setb	P1.BnFET
ENDM
MACRO BnFET_off
	clr	P1.BnFET
ENDM
MACRO CnFET_on
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.CnFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.AnFET
ENDM
MACRO CnFET_off
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.CnFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.AnFET
ENDM
MACRO All_nFETs_Off
	clr	P1.AnFET
	clr	P1.BnFET
	clr	P1.CnFET
ENDM

MACRO ApFET_on
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.ApFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.CpFET
ENDM
MACRO ApFET_off
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.ApFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.CpFET
ENDM
MACRO BpFET_on
	clr	P1.BpFET
ENDM
MACRO BpFET_off
	setb	P1.BpFET
ENDM
MACRO CpFET_on
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.CpFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	clr	P1.ApFET
ENDM
MACRO CpFET_off
	jb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.CpFET
	jnb	Flags2.PGM_DIRECTION_REV, ($+5)
	setb	P1.ApFET
ENDM
MACRO All_pFETs_Off
	setb	P1.ApFET
	setb	P1.BpFET
	setb	P1.CpFET
ENDM
MACRO All_pFETs_On
	clr	P1.ApFET
	clr	P1.BpFET
	clr	P1.CpFET
ENDM

MACRO Set_Comp_Phase_A
	jb	Flags2.PGM_DIRECTION_REV, ($+6)
	mov	CPT0MX, #21h	; Set comparator multiplexer to phase A
	jnb	Flags2.PGM_DIRECTION_REV, ($+6)
	mov	CPT0MX, #01h
ENDM
MACRO Set_Comp_Phase_B
	mov	CPT0MX, #11h	; Set comparator multiplexer to phase B
ENDM
MACRO Set_Comp_Phase_C
	jb	Flags2.PGM_DIRECTION_REV, ($+6)
	mov	CPT0MX, #01h	; Set comparator multiplexer to phase C
	jnb	Flags2.PGM_DIRECTION_REV, ($+6)
	mov	CPT0MX, #21h
ENDM
MACRO Read_Comp_Out
	mov	A, CPT0CN		; Read comparator output
	cpl	A			; Invert output
ENDM


;*********************
; PORT B definitions *
;*********************
DebugPin		EQU	0	;o

P2_PUSHPULL	EQU	(1 SHL DebugPin)


;**********************
; MCU specific macros *
;**********************
MACRO Interrupt_Table_Definition
CSEG AT 0				; Code segment start
	jmp reset
CSEG AT 0Bh			; Timer0 interrupt	
	jmp	t0_int	
CSEG AT 2Bh			; Timer2 interrupt	
	jmp	t2_int	
CSEG AT 5Bh			; PCA interrupt	
	jmp	pca_int	
CSEG AT 73h			; Timer3 interrupt	
	jmp	t3_int	
ENDM

MACRO Initialize_Adc
	mov	REF0CN, #01h		; Enable reference buffer
	mov	ADC0CF, #0B8h		; ADC clock 1MHz
	mov	AMX0P, #(8+Adc_Ip)	; Select positive input
	mov	AMX0N, #11h		; Select negative input as ground
	mov	ADC0CN, #80h		; ADC enabled
ENDM
MACRO Start_Adc
	mov	ADC0CN, #90h	; ADC start
ENDM
MACRO Get_Adc_Status
	mov	A, ADC0CN		
ENDM
MACRO Read_Adc_Result
	mov	Temp1, ADC0L
	mov	Temp2, ADC0H
ENDM
MACRO Stop_Adc
ENDM
