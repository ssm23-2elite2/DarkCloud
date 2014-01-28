/*
	DarkCloud System
	Copyright (C) 2013 Jake Yoon
*/

#include "dc_fuse_operations.h"

/** Get file attributes.
 *
 */
int dc_getattr (const char *path, struct stat *statbuf)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	struct fuse_context *fc = fuse_get_context();

	if( strcmp("/", path) == 0 )
	{
		statbuf->st_mode = S_IFDIR | 0075;
		statbuf->st_nlink = 2;

		statbuf->st_atime = time(0);
		statbuf->st_mtime = time(0);
		statbuf->st_ctime = time(0);
	}
	else
	{
		struct dc_entry_t *entry = find_entry_from_path(state, path);
		if(!entry) {
			// folder
			statbuf->st_mode = S_IFDIR | 0075;
			statbuf->st_nlink = 2;

			return -ENOENT;
		} else {

			if(entry->isFolder == 1) {
				// folder
				statbuf->st_mode = S_IFDIR | 0075;
				statbuf->st_nlink = 2;

				statbuf->st_atime = entry->uploadDate;
				statbuf->st_mtime = entry->uploadDate;
				statbuf->st_ctime = entry->uploadDate;
			} else {
				if(!entry)
					return -ENOENT;

				memset(statbuf, 0, sizeof(struct stat));

				if(entry) 
				{
					statbuf->st_size = entry->fileSize;
					statbuf->st_atime = entry->uploadDate;
					statbuf->st_mtime = entry->uploadDate;
					statbuf->st_ctime = entry->uploadDate;
			}

			statbuf->st_mode = S_IFREG | 0075;
			statbuf->st_nlink = 1;
			}
		}
	}
	statbuf->st_uid = 2000;
	statbuf->st_gid = 2000;

	return 0;
}

int dc_fgetattr (const char *path, struct stat *statbuf, struct fuse_file_info *fileinfo)
{
	return dc_getattr(path, statbuf);
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
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	const char *fileName = strip_path(path);

	char only_path[2048];
	strcpy(only_path, path);
	set_only_path(only_path);
	struct dc_entry_t *entry = find_entry_from_path(state, path);

	if(entry) {
		fprintf(stderr, "Jake : already exist file %s\n", path);
		return -EPERM;
	}

	return dci_mkdir(only_path, fileName, state);
}

int dc_rmdir (const char *path)
{
	if(strcmp(path, "/") == 0)	// 루트인 경우 삭제 불가능
		return -EACCES;
	
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	struct dc_entry_t *entry = find_entry_from_path(state, path);

	if(!entry) 					// 존재하지 않을 경우
		return -EACCES;

	if(entry->isFolder == 0)	// 디렉토리가 아닐 경우
		return -EPERM;
	
	return dci_rmdir(path, state);
}

int dc_link (const char *path, const char *newpath)
{
	return 0;
}

int dc_unlink (const char *path)
{
	if(strcmp(path, "/") == 0)	// 루트인 경우 삭제 불가능
		return -EACCES;
	
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	struct dc_entry_t *entry = find_entry_from_path(state, path);

	if(!entry) 					// 존재하지 않을 경우
		return -EACCES;

	if(entry->isFolder == 1)	// 디렉토리가 아닐 경우
		return -EPERM;
	return dci_unlink(path, state);
}

int dc_symlink (const char *path, const char *link)
{
	return 0;
}

int dc_rename (const char *path, const char *newpath)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	struct dc_entry_t *entry = find_entry_from_path(state, path);
	struct dc_entry_t *newentry = find_entry_from_path(state, newpath);

	printf("path[%s] newpath[%s]\n", path, newpath);
	if(!entry)
		return -EACCES;
	
	if(newentry)
		return -EPERM;		// 이미 존재
	
	const char *newname = strip_path(newpath);
	
	str_clear(&entry->fileName);
	str_init_create(&entry->fileName, newname, 0);
	return 0;
//dci_rename(path, newpath, state);
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

	if(flags & O_CREAT)
	{
		dc_create(path, 0755, fileinfo);	
	}

	struct dc_entry_t *entry = find_entry_from_path(state, path);
	int load = dci_load(state, entry);
	if(load)
		return -1;

	return 0;
}

int dc_create (const char *path, mode_t mode, struct fuse_file_info *fileinfo)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;

	int flags = fileinfo->flags;
	
	const char *fileName = strip_path(path);
	char only_path[2048];
	strcpy(only_path, path);
	set_only_path(only_path);
	struct dc_entry_t *entry = find_entry_from_path(state, path);

	if(entry != NULL)
		return -EPERM;

	return dci_create(fileName, only_path, state);
}

/** Read data from an open file.
 *
 */
int dc_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	struct dc_entry_t *entry = find_entry_from_path(state, path);
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
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;

	int flags = fileinfo->flags;
	
	const char *fileName = strip_path(path);
	char only_path[2048];
	strcpy(only_path, path);
	set_only_path(only_path);
	struct dc_entry_t *entry = find_entry_from_path(state, path);

	if(entry == NULL)
		return -EACCES;

	dci_write(only_path, buf, size, offset, fileName, state, entry);
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
	fprintf(stderr, "getxattr %s %s %s %d\n", path, name, value, size);
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
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;

	dci_opendir(path, state);

	return 0;
}

int dc_readdir (const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileinfo)
{
	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	struct dc_entry_t *iter = state->head;
	while(iter != NULL)
	{
		if(strcmp(iter->path.str, path) == 0)
		{
			if(filler(buf, iter->fileName.str, NULL, 0))
			{
				fprintf(stderr, "readdir() filler()\n");
				return -ENOMEM;
			}
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
/*	struct dci_state *state = &((struct dc_state*)fuse_get_context()->private_data)->dci_data;
	const char* fileName = strip_path(path);

	if(!strcmp(fileName, "/")) {
		return 0;
	}

	struct dc_entry_t *entry = find_entry_from_name(state, fileName);
	if(!entry)
		return -1;
*/	return 0;
}

int dc_ftruncate (const char *path, off_t offset, struct fuse_file_info *fileinfo)
{
	return 0;
}


int dc_lock (const char *path, struct fuse_file_info *fileinfo, int cmd, struct flock *lock)
{
	return 0;
}

int dc_utimens (const char *path, const struct timespec tv[2])
{
	return 0;//utimens(path, tv);
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
	.getattr	= dc_getattr,
	.getdir		= NULL,
	.mknod		= dc_mknod,
	.mkdir		= dc_mkdir,
	.unlink		= dc_unlink,
	.rmdir		= dc_rmdir,
	.rename		= dc_rename,
	.chmod		= dc_chmod,
	.chown		= dc_chown,
	.truncate	= dc_truncate,
	.utime		= NULL,
	.open		= dc_open,
	.read		= dc_read,
	.write		= dc_write,
	.flush		= dc_flush,
	.release	= dc_release,
	.setxattr	= dc_setxattr,
	.getxattr	= dc_getxattr,
	.opendir	= dc_opendir,
	.readdir	= dc_readdir,
	.init		= dc_init,
	.access		= dc_access,
	.create		= dc_create,
	.fgetattr	= dc_fgetattr,
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

