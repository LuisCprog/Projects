		.def SetTMPL
.thumb
.text

SetTMPL:
		MRS R0,CONTROL
		ORR R0,#1
		MSR CONTROL , R0
		ISB
		BX LR
