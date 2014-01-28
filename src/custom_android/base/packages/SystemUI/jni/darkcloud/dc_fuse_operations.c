/*
	DarkCloud System
	Copyright (C) 2013 Jake Yoon
*/

#define FUSE_USE_VERSION 26
#include <dirent.h>
#include <errno.h>
#include <fuse.h>
#include <string.h>
#include <sys/stat.h>

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
	char* root;
	struct dci_state dci_data;
};

/** Get file attributes.
 *
 */
int dc_getattr (const char *path, struct stat *statbuf)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	struct fuse_context *fc = fuse_get_context();

	if( strcmp("/", path) == 0 )
	{
		statbuf->st_mode = S_IFDIR | 0755;
		statbuf->st_nlink = 2;
	}
	else
	{
		const char *fileName = strip_path(path);
		struct dc_entry_t *entry = find_entry_from_name(state, fileName);
		if(entry->isFolder == 1) {
			// folder
			statbuf->st_mode = S_IFDIR | 0755;
			statbuf->st_nlink = 2;
			
			statbuf->st_atime = entry->uploadDate;
			statbuf->st_mtime = entry->uploadDate;
			statbuf->st_ctime = entry->uploadDate;
		} else {
			memset(statbuf, 0, sizeof(struct stat));
			
			if(!entry)
				return -ENOENT;

			if(entry) 
			{
				statbuf->st_size = entry->fileSize;
				statbuf->st_atime = entry->uploadDate;
				statbuf->st_mtime = entry->uploadDate;
				statbuf->st_ctime = entry->uploadDate;
			}

			statbuf->st_mode = S_IFREG | 0755;
			statbuf->st_nlink = 1;
		}
	}
	statbuf->st_uid = fc->uid;
	statbuf->st_gid = fc->gid;

	return 0;
}

int dc_readlink (const char *path, char *link, size_t size)
{
	return 0;
}

int dc_mknod (const char *path, mode_t mode, dev_t dev)
{
	return 0;
}

int dc_mkdir (const char *path, mode_t mode)
{
	return 0;
}

int dc_unlink (const char *path)
{
	return 0;
}

int dc_rmdir (const char *path)
{
	return 0;
}

int dc_symlink (const char *path, const char *link)
{
	return 0;
}

int dc_rename (const char *path, const char *newpath)
{
	return 0;
}

int dc_link (const char *path, const char *newpath)
{
	return 0;
}

int dc_chmod (const char *path, mode_t mode)
{
	return 0;
}

int dc_chown (const char *path, uid_t uid, gid_t gid)
{
	return 0;
}

int dc_truncate (const char *path, off_t newsize)
{
	return 0;
}

int dc_open (const char *path, struct fuse_file_info * fileinfo)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;

	int flags = fileinfo->flags;
	
	if(flags & O_RDONLY)
	{

	}

	if(flags & O_WRONLY)
	{

	}

	if(flags & O_RDWR)
	{

	}

	const char* fileName = strip_path(path);
	struct dc_entry_t *entry = find_entry_from_name(state, fileName);
	int load = dci_load(state, entry);
	if(load)
		return -1;

	return 0;
}

/** Read data from an open file.
 *
 */
int dc_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	const char* fileName = strip_path(path);
	struct dc_entry_t *entry = find_entry_from_name(state, fileName);
	if(!entry)
		return 0;
	size_t length = size;
	const char const* chunk = dci_read(&length, entry, offset);
	memcpy(buf, chunk, length);
	return length;
}

/** Write data to an open file.
 *
 */
int dc_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Get file system statistics.
 *
 */
int dc_statfs (const char *path, struct statvfs *statv)
{
	return 0;
}

/** Possibly flush cached data.
 *
 */
int dc_flush (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Release an open file.
 *
 */
int dc_release (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Synchronize file contents.
 *
 */
int dc_fsync (const char *path, int datasync, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Set extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int dc_setxattr (const char *path, const char *name, const char *value, size_t size, int flags)
{
	return 0;
}

/** Get extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int dc_getxattr (const char *path, const char *name, char *value, size_t size)
{
	return 0;
}

/** List extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int dc_listxattr (const char *path, char *list, size_t size)
{
	return 0;
}

/** Remove extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int dc_removexattr (const char *path, const char *name)
{
	return 0;
}

/** Open a directory.
 *
 */
int dc_opendir (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

int dc_readdir (const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileinfo)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	
	if(strcmp(path, "/")) {
		dci_get_file_list(path, state);
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	struct dc_entry_t *iter = state->head;
	while(iter != NULL)
	{

		if(filler(buf, iter->fileName.str, NULL, 0))
		{
			fprintf(stderr, "readdir() filler()\n");
			return -ENOMEM;
		}
		iter = iter->next;
	}

	return 0;
}

int dc_releasedir (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

int dc_fsyncdir (const char *path, int datasync, struct fuse_file_info *fileinfo)
{
	return 0;
}

void *dc_init (struct fuse_conn_info *conn)
{
	return ((struct dc_state *) fuse_get_context()->private_data);
}

void dc_destroy (void *userdata)
{
}

int dc_access (const char *path, int mask)
{
	return 0;
}

int dc_create (const char *path, mode_t mode, struct fuse_file_info *fileinfo)
{
	return 0;
}

int dc_ftruncate (const char *path, off_t offset, struct fuse_file_info *fileinfo)
{
	return 0;
}

int dc_fgetattr (const char *path, struct stat *statbuf, struct fuse_file_info *fileinfo)
{
	return 0;
}

int dc_lock (const char *path, struct fuse_file_info *fileinfo, int cmd, struct flock *lock)
{
	return 0;
}

int dc_utimens (const char *path, const struct timespec tv[2])
{
	return 0;
}

int dc_ioctl (const char *path, int cmd, void *arg, struct fuse_file_info *fileinfo, unsigned int flags, void *data)
{
	return 0;
}

int dc_poll (const char *path, struct fuse_file_info *fileinfo, struct fuse_pollhandle *ph, unsigned *reventsp)
{
	return 0;
}

struct fuse_operations dc_oper = {
	.getattr     = dc_getattr,
	//.readlink    = dc_readlink,
	// getdir() deprecated, use readdir()
	.getdir        = NULL,
	//.mknod       = dc_mknod,
	//.mkdir       = dc_mkdir,
	//.unlink      = dc_unlink,
	//.rmdir       = dc_rmdir,
	//.symlink     = dc_symlink,
	//.rename      = dc_rename,
	//.link        = dc_link,
	//.chmod       = dc_chmod,
	//.chown       = dc_chown,
	//.truncate    = dc_truncate,
	// utime() deprecated, use utimens
	.utime         = NULL,
	.open        = dc_open,
	.read        = dc_read,
	//.write       = dc_write,
	//.statfs      = dc_statfs,
	//.flush       = dc_flush,
	.release     = dc_release,
	//.fsync       = dc_fsync,
	//.setxattr    = dc_setxattr,
	//.getxattr    = dc_getxattr,
	//.listxattr   = dc_listxattr,
	//.removexattr = dc_removexattr,
	//.opendir     = dc_opendir,
	.readdir     = dc_readdir,
	//.releasedir  = dc_releasedir,
	//.fsyncdir    = dc_fsyncdir,
	.init        = dc_init,
	//.destroy     = dc_destroy,
	//.access      = dc_access,
	//.create      = dc_create,
	//.ftruncate   = dc_ftruncate,
	//.getattr     = dc_fgetattr,
	//.lock        = dc_lock,
	//.utimens     = dc_utimens,
	//.ioctl       = dc_ioctl,
	//.poll        = dc_poll,
};

int main(int argc, char* argv[])
{
	int fuse_stat;
	struct dc_state dc_data;

	int ret = dci_init(&dc_data.dci_data);
	if(ret != 0)
		return ret;

	// Start fuse
	fuse_stat = fuse_main(argc, argv, &dc_oper, &dc_data);
	/*  When we get here, fuse has finished.
	 *  Do any necessary cleanups.
	 */
	dci_destroy(&dc_data.dci_data);
	return fuse_stat;
}

