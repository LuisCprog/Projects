		.def SetPSP
.thumb
.text

SetPSP:
		MSR PSP , R0
		ISB
		BX LR

