/*
 * (c) Copyright 1997 Sun Microsystems, Inc.
 */

#include <sys/stat.h>

extern char *DtDtsDataToDataType64(
		const char		*filepath,
		const void		*buffer,
		const int		size,
		const struct stat64	*stat_buff,
		const char		*link_name,
		const struct stat64	*link_stat_buff,
		const char		*opt_name);

char	*
DtDtsDataToDataType(const char *path,
	const void		*buffer,
	const int		size,
	const struct stat	*fileStat,
	const char		*linkName,
	const struct stat	*linkStat,
	const char		*name)
{
	struct stat64 fileStat64, linkStat64;
	int i = _FILE_OFFSET_BITS;

	if (fileStat != 0) {
		fileStat64.st_dev	= fileStat->st_dev;
		for (i = 0; i < 3; i++) {
			fileStat64.st_pad1[i]	= fileStat->st_pad1[i];
		}
		fileStat64.st_ino._l[0]	= 0;
		fileStat64.st_ino._l[1]	= fileStat->st_ino;
		fileStat64.st_mode	= fileStat->st_mode;
		fileStat64.st_nlink	= fileStat->st_nlink;
		fileStat64.st_uid	= fileStat->st_uid;
		fileStat64.st_gid	= fileStat->st_gid;
		fileStat64.st_rdev	= fileStat->st_rdev;
		for (i = 0; i < 2; i++) {
			fileStat64.st_pad2[i]	= fileStat->st_pad2[i];
		}
		fileStat64.st_size._l[0]= 0;
		fileStat64.st_size._l[1]= fileStat->st_size;
		fileStat64.st_atim	= fileStat->st_atim;
		fileStat64.st_mtim	= fileStat->st_mtim;
		fileStat64.st_ctim	= fileStat->st_ctim;
		fileStat64.st_blksize	= fileStat->st_blksize;
		fileStat64.st_blocks._l[0] = 0;
		fileStat64.st_blocks._l[1] = fileStat->st_blocks;
		for (i = 0; i < _ST_FSTYPSZ; i++) {
			fileStat64.st_fstype[i]	= fileStat->st_fstype[i];
		}
		for (i = 0; i < 4; i++) {
			fileStat64.st_pad4[i]	= fileStat->st_pad4[i];
		}
	}
	if (linkStat != 0) {
		linkStat64.st_dev	= linkStat->st_dev;
		for (i = 0; i < 3; i++) {
			linkStat64.st_pad1[i]	= linkStat->st_pad1[i];
		}
		linkStat64.st_ino._l[0]	= 0;
		linkStat64.st_ino._l[1]	= linkStat->st_ino;
		linkStat64.st_mode	= linkStat->st_mode;
		linkStat64.st_nlink	= linkStat->st_nlink;
		linkStat64.st_uid	= linkStat->st_uid;
		linkStat64.st_gid	= linkStat->st_gid;
		linkStat64.st_rdev	= linkStat->st_rdev;
		for (i = 0; i < 2; i++) {
			linkStat64.st_pad2[i]	= linkStat->st_pad2[i];
		}
		linkStat64.st_size._l[0]= 0;
		linkStat64.st_size._l[1]= linkStat->st_size;
		linkStat64.st_atim	= linkStat->st_atim;
		linkStat64.st_mtim	= linkStat->st_mtim;
		linkStat64.st_ctim	= linkStat->st_ctim;
		linkStat64.st_blksize	= linkStat->st_blksize;
		linkStat64.st_blocks._l[0] = 0;
		linkStat64.st_blocks._l[1] = linkStat->st_blocks;
		for (i = 0; i < _ST_FSTYPSZ; i++) {
			linkStat64.st_fstype[i]	= linkStat->st_fstype[i];
		}
		for (i = 0; i < 4; i++) {
			linkStat64.st_pad4[i]	= linkStat->st_pad4[i];
		}
	}
	return DtDtsDataToDataType64( path, buffer, size,
				      fileStat ? &fileStat64 : 0,
				      linkName,
				      linkStat ? &linkStat64 : 0, 
				      name );
}
