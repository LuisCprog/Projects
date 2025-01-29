		.def Push
.thumb
.text

Push:
	MRS R0, PSP ;Get current PSP into R0
   	SUB R0, R0, #36 ; Adjust PSP for 9 registers (4 bytes each) - R4 to R11 + LR
    STR R4, [R0, #32]  ; Store R4 at address R0 + 32	R4
	STR R5, [R0, #28]  ; Store R5 at address R0 + 28	R5
	STR R6, [R0, #24]  ; Store R6 at address R0 + 24	R6
	STR R7, [R0, #20]  ; Store R7 at address R0 + 20	R7
	STR R8, [R0, #16]  ; Store R8 at address R0 + 16	R8
	STR R9, [R0, #12]  ; Store R9 at address R0 + 12	R9
	STR R10, [R0, #8]  ; Store R10 at address R0 + 8	R10
	STR R11, [R0, #4]  ; Store R11 at address R0 + 4	R11
    MOVW R1, #0xFFFD	; Stores LR at R0
    MOVT R1, #0xFFFF
    STR R1, [R0, #0] ;LR
    MSR PSP, R0			;Restores the value of PSP
    BX LR
