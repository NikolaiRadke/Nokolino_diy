/*
 * jq6500 - Program the SPI flash of a JQ6500 MP3 player module.
 *
 * Copyright (C) 2017 Reinhard Max <reinhard@m4x.de>
 *
 * "THE BEER-WARE LICENSE" (Revision 42): As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a
 * beer in return.
 */

#include <stdint.h>
#include <scsi/sg.h>
#include <unistd.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define FLASHSIZE 0x200000 /* 2MiB   */
#define BASE       0x40000 /* 256kiB */
#define MAXSIZE (FLASHSIZE - BASE)

#define ERASE 0xfbd8
#define WRITE 0xfbd9
#define BLKSZ 0x1000
#define TIMEOUT 30000

struct jqcmd {
    uint16_t j_cmd;
    uint32_t j_off;
    uint32_t j_foo;
    uint32_t j_len;
    uint16_t j_bar;
} __attribute__((packed));

uint8_t buf[FLASHSIZE];
uint8_t force = 0;

off_t
filesize(char *file)
{
    int result;
    struct stat buf;
    result = stat(file, &buf);
    if (result != 0) {
	err(1, "cannot stat %s", file);
    }
    return buf.st_size;
}

off_t
isdevice(char *file)
{
    int result;
    struct stat buf;
    result = stat(file, &buf);
    if (result != 0) {
	err(1, "cannot stat %s", file);
    }
    return (buf.st_rdev != 0);
}

void
w32le(uint8_t **loc, uint32_t val)
{
    memcpy(*loc, &htole32(val), 4);
    (*loc) += 4;
}

void
loadbar(unsigned int x, unsigned int n)
{
    int i;
    unsigned int w = 50;
    float ratio    =  x/(float)n;
    int   c        =  ratio * w;
    
    fprintf(stderr, "\r%7d (%3d%%) [", x, (int)(ratio*100));
    for (i=0; i<c; i++) fprintf(stderr, "-");
    for (i=c; i<w; i++) fprintf(stderr, " ");
    fprintf(stderr, "]");
    fflush(stderr);
}

void
usage(void)
{
    printf("Usage: jq6500 [OPTION]... [FILE]...\n"
	   "\n"
	   "Write files to an MP3 player based on the JQ6500 chip that is connected\n"
	   "via USB. Files can be written in raw mode or as a simple file system which\n"
	   "is created on the fly.\n"
	   "The file system is needed by the device in order to play MP3 or WAV\n"
	   "files. Raw mode can be used to overwrite the ISO image in the first\n"
	   "256kiB of the device or to flash an existing file system image with audio\n"
	   "files.\n"
	   "\n"
	   "Options:\n"
	   "  -d device   SCSI generic device or file to write to (mandatory)\n"
	   "  -r          write a single file in raw mode without creating a file system\n"
	   "  -o offset   default: 0x40000\n"
	   "  -s prefix   save the content of the pseudo file system on the device\n"
	   "              to a set of files named prefix-dirno-fileno\n"
	   "  -f          force operation (disable sanity checks).\n");
    
    exit(1);
}

int
mkjqfs(int count, char **files, int offset)
{
    int size, i, len;
    uint8_t *dir, *data;
    memset(buf, 0xff, FLASHSIZE);

    dir = buf;
    /* 2 for main dir, 1 for length of subdir, 2 for each file */
    size = (2 + 1 + 2 * count) * sizeof(uint32_t);
    data = dir + size;

    w32le(&dir, 1);		/* Number of Subdirs */
    w32le(&dir, offset + 8);	/* Offset of first subdir */
    w32le(&dir, count);		/* Number of files in subdir 1 */

    for (i = 0; i < count; i++) {
	int fd;
	char *fname = files[i];
	len = filesize(fname);
	size += len;
	if (!force && size >= MAXSIZE) {
	    errx(1, "length %d exceeds maximum of %d\n", size, MAXSIZE);
	}
	fd = open(fname, O_RDONLY);
	if (fd < 0) 
	    err(1, "cannot open %s", fname);
	if (read(fd, data, len) != len) 
	    err(1, "cannot read %s", fname);
	close(fd);
	w32le(&dir, offset + data - buf); /* File offset */
	w32le(&dir, len);                 /* File length */
	data += len;
    }
    return size;
}

void
jqwrite(char *device, uint8_t *buf, int offset, int size)
{
    int dev, i;

    dev = open(device, O_RDWR);
    if (dev < 0)
	err(1, "cannot open device %s", device);

    fprintf(stderr, "Uploading %d bytes...\n", size);

    for (i = 0; i < size; i += BLKSZ) {
	
	struct jqcmd cmd;
	struct sg_io_hdr hdr;

	loadbar(i, size);

	memset(&cmd, 0, sizeof(cmd));
	cmd.j_cmd = htobe16(ERASE);
	cmd.j_off = htobe32(offset + i);

	memset(&hdr, 0, sizeof(hdr));
	hdr.interface_id = 'S';
	hdr.timeout = TIMEOUT;
	hdr.cmdp = (unsigned char *) &cmd;
	hdr.cmd_len = sizeof(cmd);
	hdr.dxfer_direction = SG_DXFER_NONE;

	if (ioctl(dev, SG_IO, &hdr) < 0) {
	     fprintf(stderr, "\n");
	     err(1, "erase failed at offset %x", offset + i);
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.j_cmd = htobe16(WRITE);
	cmd.j_off = htobe32(offset + i);
	cmd.j_len = htobe32(BLKSZ);

	memset(&hdr, 0, sizeof(hdr));
	hdr.interface_id = 'S';
	hdr.timeout = TIMEOUT;
	hdr.cmdp = (unsigned char *) &cmd;
	hdr.cmd_len = sizeof(cmd);
	hdr.dxfer_direction = SG_DXFER_TO_DEV;
	hdr.dxferp = buf + i;
	hdr.dxfer_len = BLKSZ;
	
	if (ioctl(dev, SG_IO, &hdr) < 0) {
	    fprintf(stderr, "\n");
	    err(1, "\nwrite failed at offset %x", offset + i);
	}
    }
    loadbar(size, size);
    fprintf(stderr, "\n");
    close(dev);
}

int
main(int argc, char **argv)
{
    int size, opt;
    char *device = NULL;
    char *endptr;
    int offset = BASE;
    int rawmode = 0;

    if (argc == 1) {
	usage();
    }
    
    while ((opt = getopt(argc, argv, "d:o:rs:fh")) != -1) {
	switch (opt) {
	case 'd':
	    device = optarg;
	    break;
	case 'o':
	    errno = 0;
	    offset = strtol(optarg, &endptr, 0);
	    if (errno != 0 || *optarg == 0 || *endptr != 0) {
		errx(1, "Invalid number: '%s'", optarg);
	    }
	    break;
	case 'r':
	    rawmode = 1;
	    break;
	case 'f':
	    force = 1;
	    break;
	case 's':
	    errx(1, "-%c: Option not implemented.", opt);
	    break;
	case 'h':
	default:
	    usage();
	}
    }

    if (!force) {
	if (offset < 0) {
	    errx(1, "Offset out of range [0..0x2000].");
	}
	if (offset % BLKSZ != 0) {
	    errx(1, "Offset must be a multiple of %d.", BLKSZ);
	}
    }

    if (device == NULL || optind == argc) {
	errx(1, "Missing file operand.");
    }

    if (rawmode) {
	char *fname = argv[optind];
	int fd;
	size = filesize(fname);

	if (!force && size > FLASHSIZE) {
	    errx(1, "length %d exceeds maximum of %d\n", size, MAXSIZE);
	}
	fd = open(fname, O_RDONLY);
	if (read(fd, buf, size) != size) {
	    err(1, "cannot read %s", fname);
	}
    } else {
	size = mkjqfs(argc - optind, argv + optind, offset);
    }

    if (isdevice(device)) {
	jqwrite(device, buf, offset, size);
    } else {	
	int fd = open(device, O_WRONLY | O_CREAT, "0666");
	if (write(fd, buf, size) != size) {
	    err(1, "write failed in %s", device);
	}
    }
    
    return 0;
}

/* Local Variables:  */
/* mode: c           */
/* c-basic-offset: 4 */
/* End:              */
