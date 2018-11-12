;;;
;;; Stub functions for DLX traps.
;;;
;;; Ethan L. Miller, 1999.  Released to the public domain.
;;;
;;; The traps such as Open and Close
;;; (note the capital letters) are for use by user programs.  The traps
;;; with names such as open and close (lower case) are for use by the OS.
;;;

	.text
	.align 2

;;; The following are the traps to be used by user-level programs
;;;
		.text
	.align 2
.proc _Open
.global _Open
_Open:		
;;; Note that trap #0x213 actually causes a trap vector in the OS of
;;; 0x1213.  The same is true for other user traps.  Thus, a user trap
;;; with trap #0x240 will result in a trap vector of 0x1240 in the OS.
	trap	#0x213
	jr	r31
	nop
.endproc _Open

.proc _Close
.global _Close
_Close:		
	trap	#0x214
	jr	r31
	nop
.endproc _Close

.proc _Read
.global _Read
_Read:		
	trap	#0x210
	jr	r31
	nop
.endproc _Read

.proc _Write
.global _Write
_Write:		
	trap	#0x211
	jr	r31
	nop
.endproc _Write

.proc _Lseek
.global _Lseek
_Lseek:		
	trap	#0x212
	jr	r31
	nop
.endproc _Lseek

.proc _Putchar
.global _Putchar
_Putchar:		
	trap	#0x280
	jr	r31
	nop
.endproc _Putchar

.proc _Printf
.global _Printf
_Printf:		
	trap	#0x201
	jr	r31
	nop
.endproc _Printf

.proc _getpid
.global _getpid
_getpid:
	trap 	#0x431
	jr	r31
	nop
.endproc _getpid

.proc _process_create
.global _process_create
_process_create:
	trap	#0x432
	jr	r31
	nop
.endproc _process_create

.proc _shmget
.global _shmget
_shmget:
	trap	#0x440
	jr	r31
	nop
.endproc _shmget

.proc _shmat
.global _shmat
_shmat:
	trap	#0x441
	jr	r31
	nop
.endproc _shmat

.proc _sem_create
.global _sem_create
_sem_create:
	trap	#0x450
	jr	r31
	nop
.endproc _sem_create

.proc _sem_wait
.global _sem_wait
_sem_wait:
	trap	#0x451
	jr	r31
	nop
.endproc _sem_wait

.proc _sem_signal
.global _sem_signal
_sem_signal:
	trap	#0x452
	jr	r31
	nop
.endproc _sem_signal

.proc _lock_create
.global _lock_create
_lock_create:
	trap	#0x453
	jr	r31
	nop
.endproc _lock_create

.proc _lock_acquire
.global _lock_acquire
_lock_acquire:
	trap	#0x454
	jr	r31
	nop
.endproc _lock_acquire

.proc _lock_release
.global _lock_release
_lock_release:
	trap	#0x455
	jr	r31
	nop
.endproc _lock_release

.proc _cond_create
.global _cond_create
_cond_create:
	trap	#0x456
	jr	r31
	nop
.endproc _cond_create

.proc _cond_wait
.global _cond_wait
_cond_wait:
	trap	#0x457
	jr	r31
	nop
.endproc _cond_wait

.proc _cond_signal
.global _cond_signal
_cond_signal:
	trap	#0x458
	jr	r31
	nop
.endproc _cond_signal

.proc _cond_broadcast
.global _cond_broadcast
_cond_broadcast:
	trap	#0x459
	jr	r31
	nop
.endproc _cond_broadcast

.proc _mbox_create
.global _mbox_create
_mbox_create:
	trap	#0x460
	jr	r31
	nop
.endproc _mbox_create

.proc _mbox_open
.global _mbox_open
_mbox_open:
	trap	#0x461
	jr	r31
	nop
.endproc _mbox_open

.proc _mbox_close
.global _mbox_close
_mbox_close:
	trap	#0x462
	jr	r31
	nop
.endproc _mbox_close

.proc _mbox_send
.global _mbox_send
_mbox_send:
	trap	#0x463
	jr	r31
	nop
.endproc _mbox_send

.proc _mbox_recv
.global _mbox_recv
_mbox_recv:
	trap	#0x464
	jr	r31
	nop
.endproc _mbox_recv

.proc _sleep
.global _sleep
_sleep:
	trap	#0x465
	jr	r31
	nop
.endproc _sleep

.proc _yield
.global _yield
_yield:
	trap	#0x466
	jr	r31
	nop
.endproc _yield

.proc _disk_write_block
.global _disk_write_block
_disk_write_block:
	trap	#0x467
	jr	r31
	nop
.endproc _disk_write_block

.proc _disk_size
.global _disk_size
_disk_size:
	trap	#0x468
	jr	r31
	nop
.endproc _disk_size

.proc _disk_blocksize
.global _disk_blocksize
_disk_blocksize:
	trap	#0x469
	jr	r31
	nop
.endproc _disk_blocksize

.proc _disk_create
.global _disk_create
_disk_create:
	trap	#0x470
	jr	r31
	nop
.endproc _disk_create

.proc _dfs_invalidate
.global _dfs_invalidate
_dfs_invalidate:
	trap	#0x471
	jr	r31
	nop
.endproc _dfs_invalidate

.proc _file_open
.global _file_open
_file_open:
	trap	#0x472
	jr	r31
	nop
.endproc _file_open

.proc _file_close
.global _file_close
_file_close:
	trap	#0x473
	jr	r31
	nop
.endproc _file_close

.proc _file_delete
.global _file_delete
_file_delete:
	trap	#0x474
	jr	r31
	nop
.endproc _file_delete

.proc _file_read
.global _file_read
_file_read:
	trap	#0x475
	jr	r31
	nop
.endproc _file_read

.proc _file_write
.global _file_write
_file_write:
	trap	#0x476
	jr	r31
	nop
.endproc _file_write

.proc _file_seek
.global _file_seek
_file_seek:
	trap	#0x477
	jr	r31
	nop
.endproc _file_seek

.proc _run_os_tests
.global _run_os_tests
_run_os_tests:
	trap	#0x4FF
	jr	r31
	nop
.endproc _run_os_tests


.proc _Exit
.global _Exit
_Exit:
	trap	#0x500
	jr	r31
	nop
.endproc _Exit
