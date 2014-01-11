#ifndef _TAR_H
#define _TAR_H

#include <types.h>

struct header_gnu_tar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char atime[12];
	char ctime[12];
	char offset[12];
	char longnames[4];
	char unused[1];
	struct {
		char offset[12];
		char numbytes[12];
	} sparse[4];
	char isextended[1];
	char realsize[12];
	char pad[17];
};

size_t tar_size(struct header_gnu_tar *p);
void tar_demo(uintptr_t addr);

#endif
