#pragma once

typedef struct _KTRAP_FRAME {


	//
	//  Following 4 values are only used and defined for DBG systems,
	//  but are always allocated to make switching from DBG to non-DBG
	//  and back quicker.  They are not DEVL because they have a non-0
	//  performance impact.
	//

	ULONG   DbgEbp;         // Copy of User EBP set up so KB will work.
	ULONG   DbgEip;         // EIP of caller to system call, again, for KB.
	ULONG   DbgArgMark;     // Marker to show no args here.
	ULONG   DbgArgPointer;  // Pointer to the actual args

	//
	//  Temporary values used when frames are edited.
	//
	//
	//  NOTE:   Any code that want's ESP must materialize it, since it
	//          is not stored in the frame for kernel mode callers.
	//
	//          And code that sets ESP in a KERNEL mode frame, must put
	//          the new value in TempEsp, make sure that TempSegCs holds
	//          the real SegCs value, and put a special marker value into SegCs.
	//

	ULONG   TempSegCs;
	ULONG   TempEsp;

	//
	//  Debug registers.
	//

	ULONG   Dr0;
	ULONG   Dr1;
	ULONG   Dr2;
	ULONG   Dr3;
	ULONG   Dr6;
	ULONG   Dr7;

	//
	//  Segment registers
	//

	ULONG   SegGs;
	ULONG   SegEs;
	ULONG   SegDs;

	//
	//  Volatile registers
	//

	ULONG   Edx;
	ULONG   Ecx;
	ULONG   Eax;

	//
	//  Nesting state, not part of context record
	//

	ULONG   PreviousPreviousMode;

	ULONG	ExceptionList;
	// Trash if caller was user mode.
	// Saved exception list if caller
	// was kernel mode or we're in
	// an interrupt.

	//
	//  FS is TIB/PCR pointer, is here to make save sequence easy
	//

	ULONG   SegFs;

	//
	//  Non-volatile registers
	//

	ULONG   Edi;
	ULONG   Esi;
	ULONG   Ebx;
	ULONG   Ebp;

	//
	//  Control registers
	//

	ULONG   ErrCode;
	ULONG   Eip;
	ULONG   SegCs;
	ULONG   EFlags;

	ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
	ULONG   HardwareSegSs;  // that involve a ring transition.

	ULONG   V86Es;          // these will be present for all transitions from
	ULONG   V86Ds;          // V86 mode
	ULONG   V86Fs;
	ULONG   V86Gs;
} KTRAP_FRAME;

typedef KTRAP_FRAME *PKTRAP_FRAME;

typedef VOID (*KEBUGCHECK2)( IN ULONG BugCheckCode,
						   IN ULONG_PTR BugCheckParameter1,
						   IN ULONG_PTR BugCheckParameter2,
						   IN ULONG_PTR BugCheckParameter3,
						   IN ULONG_PTR BugCheckParameter4,
						   IN PKTRAP_FRAME TrapFrame);

VOID KeBugCheck2_InlineProxy( IN ULONG BugCheckCode,
							  IN ULONG_PTR BugCheckParameter1,
							  IN ULONG_PTR BugCheckParameter2,
							  IN ULONG_PTR BugCheckParameter3,
							  IN ULONG_PTR BugCheckParameter4,
						      IN PKTRAP_FRAME TrapFrame);


BOOLEAN DoBsodHook();
