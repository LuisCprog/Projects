		.def SetASP
.thumb
.text

SetASP:
		MRS R0, CONTROL
		ORR R0, R0, #0x02
		MSR CONTROL, R0
		ISB
		BX LR
