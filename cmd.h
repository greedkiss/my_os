#ifndef _CMD_H
#define _CMD_H
    
#include "myMemory.h"
#include "init.h"
#include "super.h"
#include "fs.h"
#include "namei.h"
#include "open.h"
#include "inode.h"
#include "namei.h"
#include "read_write.h"
#include "sched.h"

extern void list();

extern void mkdir();

extern void touch();

extern void shell_write();

extern void shell_read();

extern void ln();

extern void cd();

extern void rm();

extern void rmdir();

extern void help();

extern void execcmd();
#endif 