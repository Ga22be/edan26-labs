.globl tbr
tbr:
	mftb 3
	blr

.globl tbr32
tbr32:
	mftbu 0
	mftb 4
	mftbu 3
	cmpw 0,3
	beqlr- 
	b tbr32
