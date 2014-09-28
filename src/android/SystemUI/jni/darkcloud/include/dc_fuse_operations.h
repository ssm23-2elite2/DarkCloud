#define FUSE_USE_VERSION 26
#include <dirent.h>
#include <errno.h>
#include <fuse.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "dc_entry.h"
#include "dc_interface.h"
#include "str.h"
#include "util.h"


/**
 *  Store any state about this mount in this structure.
 *
 *  root: The path to the mount point of the drive.
 *  credentials: Struct which stores necessary credentials for this mount.
 */

struct dc_state {
	char *root;
	struct dci_state dci_data;
};

void *dc_init (struct fuse_conn_info *conn);
int dc_getattr (const char *path, struct stat *statbuf);
int dc_fgetattr (const char *path, struct stat *statbuf, struct fuse_file_info *fileinfo);
int dc_open (const char *path, struct fuse_file_info *fileinfo);
int dc_create (const char *path, mode_t mode, struct fuse_file_info *fileinfo);
int dc_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo);
int dc_opendir (const char *path, struct fuse_file_info *fileinfo);
int dc_readdir (const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileinfo);



int dc_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo);
int dc_statfs (const char *path, struct statvfs *statv);
int dc_flush (const char *path, struct fuse_file_info *fileinfo);
int dc_release (const char *path, struct fuse_file_info *fileinfo);
int dc_fsync (const char *path, int datasync, struct fuse_file_info *fileinfo);
int dc_setxattr (const char *path, const char *name, const char *value, size_t size, int flags);
int dc_getxattr (const char *path, const char *name, char *value, size_t size);
int dc_listxattr (const char *path, char *list, size_t size);
int dc_removexattr (const char *path, const char *name);

int dc_releasedir (const char *path, struct fuse_file_info *fileinfo);
int dc_fsyncdir (const char *path, int datasync, struct fuse_file_info *fileinfo);
void dc_destroy (void *userdata);
int dc_access (const char *path, int mask);
int dc_ftruncate (const char *path, off_t offset, struct fuse_file_info *fileinfo);
int dc_lock (const char *path, struct fuse_file_info *fileinfo, int cmd, struct flock *lock);
int dc_utimes (const char *path, const struct timespec tv[2]);
int dc_ioctl (const char *path, int cmd, void *arg, struct fuse_file_info *fileinfo, unsigned int flags, void *data);
int dc_poll (const char *path, struct fuse_file_info *fileinfo, struct fuse_pollhandle *ph, unsigned *reventsp);

int dc_readlink (const char *path, char *link, size_t size);
int dc_mknod (const char *path, mode_t mode, dev_t dev);
int dc_mkdir (const char *path, mode_t mode);
int dc_rmdir (const char *path);
int dc_rename (const char *path, const char *newpath);
int dc_link (const char *path, const char *newpath);
int dc_unlink (const char *path);
int dc_symlink (const char *path, const char *link);
int dc_chmod (const char *path, mode_t mode);
int dc_chown (const char *path, uid_t uid, gid_t gid);
int dc_truncate (const char *path, off_t newsize);



