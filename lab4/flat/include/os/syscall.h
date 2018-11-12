//
//	syscall.h
//
//	This file declares all of the system calls available as builtins
//	in the simulator.
//

#ifndef	_syscall_h_
#define	_syscall_h_

extern int	open(const char *, int);
extern int	read(int, char *, int);
extern int	write(int, char *, int);
extern int	lseek(int, int, int);
extern int	close(int);

#endif	// _syscall_h_
