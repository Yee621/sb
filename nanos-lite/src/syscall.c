#include "common.h"
#include "syscall.h"
#include "fs.h"

extern int mm_brk(uint32_t new_brk);

uintptr_t sys_write(int fd, const uint8_t *buf, size_t count){
	//Log();
	uintptr_t i = 0;
	if (fd == 1 || fd == 2){
			for(; i < count; i++){
				_putc(((char*)buf)[i]);
			}
		}
	return i;
}


_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];

  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
	case SYS_none:
		r->eax = 1;
		break;
	case SYS_exit:
		_halt(a[1]);
		break;
	case SYS_write:
		//r->eax = sys_write(a[1], (uint8_t *)a[2], a[3]);
		r->eax = fs_write(a[1], (uint8_t *)a[2], a[3]);
		break;
	case SYS_read:
		r->eax = fs_read(a[1], (uint8_t *)a[2], a[3]);
		break;
	case SYS_brk:
		r->eax = mm_brk(a[1]);
		break;
	case SYS_open:
		r->eax = fs_open((char *)a[1], a[2], a[3]);
	   	break;
	case SYS_close:
		r->eax = fs_close(a[1]);
	   	break;
	case SYS_lseek:
		//Log("11111");
		r->eax = fs_lseek(a[1], a[2], a[3]);
		//Log("sys_lseek: %d", r->eax);
		break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
