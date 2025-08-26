#ifndef IOCTL_CONTROL_H
#define IOCTL_CONTROL_H

#define IOCTL_MAGIC  'm'
struct user_metadata {
	int count;
	char name[100];
};

#define IOCTL_RD_DATA		_IOR(IOCTL_MAGIC, 'r', int *)
#define IOCTL_WR_DATA		_IOW(IOCTL_MAGIC, 'w', int *)
#define IOCTL_WR_METADATA	_IOW(IOCTL_MAGIC, 'd', struct user_metadata *)

#endif
