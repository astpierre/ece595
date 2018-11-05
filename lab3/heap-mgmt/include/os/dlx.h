//
// Definitions of flags and other things defined by the DLX processor.
//

#ifndef	_dlx_h_
#define	_dlx_h_

#define	DLX_STATUS_INTRMASK	0x0f	// up to 16 interrupt levels
#define	DLX_STATUS_FPTRUE	0x20	// Set if last FP comparison was true
#define	DLX_STATUS_SYSMODE	0x40	// Set if CPU is in system mode
#define	DLX_STATUS_PAGE_TABLE	0x100	// Set -> use a page table
#define	DLX_STATUS_TLB		0x200	// Set -> use a software-loaded TLB

#endif	// _dlx_h_
