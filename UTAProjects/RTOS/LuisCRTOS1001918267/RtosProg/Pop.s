		.def Pop
.thumb
.text

Pop:
		MRS R0, PSP;puts PSP into R0 to point into the current postion in the stack
		LDR LR,[R0],#4  ;restores the value of LR
		LDR R11,[R0],#4 ;restores the value of R11
		LDR R10,[R0],#4 ;restores the value of R10
		LDR R9,[R0],#4 ;restores the value of R9
		LDR R8,[R0],#4 ;restores the value of R8
		LDR R7,[R0],#4 ;restores the value of R7
		LDR R6,[R0],#4 ;restores the value of R6
		LDR R5,[R0],#4 ;restores the value of R5
		LDR R4,[R0],#4 ;restores the value of R4
		MSR PSP,R0 ;restores the values of PSP after restoring
		BX LR
