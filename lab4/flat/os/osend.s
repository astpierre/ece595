;;; This file just includes a single variable that contains the last address
;;; in the operating system.  The rest of memory is available for use by
;;; processes.
	.data
	.align	2
	.global _lastosaddress
_lastosaddress:
	.word	_lastosaddress+8
