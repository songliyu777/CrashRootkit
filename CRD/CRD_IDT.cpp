#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_IDT.h"

VOID InitIDT()
{
	IDTINFO		idt_info;
	IDTENTRY*	idt_entries;
	char _t[255];

	__asm	sidt	idt_info

	
	idt_entries = (IDTENTRY*) MAKELONG(idt_info.LowIDTbase,idt_info.HiIDTbase);

	ULONG_PTR ISR_pointer = MAKELONG(idt_entries[0x16].LowOffset,idt_entries[0x16].HiOffset);

	KdPrint(("µ±Ç°IDTµØÖ· %X\r\n",ISR_pointer));

	//__asm cli
	//idt_entries[0x16].LowOffset = (unsigned short)my_interrupt_hook;
	//idt_entries[0x16].HiOffset = (unsigned short)((unsigned long)my_interrupt_hook >> 16);
	//__asm sti
}