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

#define _GNU_SOURCE
#include <stdint.h>
#include <scsi/scsi.h>
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
#include <glob.h>
#include <stdarg.h>

#define BASE       0x40000 /* 256kiB */

#define JQ_ERASE 0xfbd8
#define JQ_WRITE 0xfbd9
#define JQ_READ  0xfd03
#define BLKSZ 0x1000
#define TIMEOUT 30000

struct jqcmd {
    uint16_t j_cmd;
    uint32_t j_off;
    uint32_t j_foo;
    uint32_t j_len;
    uint16_t j_bar;
} __attribute__((packed));

int flashsize = 0x1000000; /* 16Mib */
int maxsize;
uint8_t *buf;
int force = 0;
int verbosity = 1;
int cmd = 0;

void
p(int level, const char* format, ...) {
    if (verbosity >= level) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
    }
}

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
    uint32_t le = htole32(val);
    memcpy(*loc, &le, 4);
    (*loc) += 4;
}

void
loadbar(unsigned int x, unsigned int n)
{
    int i;
    unsigned int w = 50;
    float ratio    =  x/(float)n;
    int   c        =  ratio * w;
    
    p(1, "\r%7d (%3d%%) [", x, (int)(ratio*100));
    for (i=0; i<c; i++) p(1, "-");
    for (i=c; i<w; i++) p(1, " ");
    p(1, "]");
    fflush(stderr);
}

void
usage(void)
{
    printf("Usage: jq6500 <command> [OPTION]... [FILE]...\n"
	   "\n"
	   "Write files to an MP3 player based on the JQ6500 chip that is connected\n"
	   "via USB. Files can be written in raw mode or as a simple file system which\n"
	   "is created on the fly.\n"
	   "The file system is needed by the device in order to play MP3 or WAV\n"
	   "files. Raw mode can be used to overwrite the ISO image in the first\n"
	   "256kiB of the device or to flash an existing file system image with audio\n"
	   "files.\n"
	   "\n"
	   "Commands:\n"
	   "  -L          List available JQ6500 devices\n"
	   "  -S          detect and print the size of the onboard flash chip\n"
	   "  -R          read from flash\n"
	   "  -W          write to flash\n"
	   "  -E          erase flash\n"
	   "\n"
	   "Options:\n"
	   "  -d device   SCSI generic device or file to write to (default: autodetect)\n"
	   "  -r          raw mode: read/write flash contents to/from a single file\n"
	   "  -o offset   default: 0x40000\n"
	   "  -s size     default: 4096 for read and erase, file size for write\n"
	   "  -f          force operation (disables some sanity checks, use with care!)\n"
	   "  -v level    set the verbosity level (default: 1)\n"
	   "  -p          patch the detected size into config.ini of the given image before flashing\n"
	   "  -q          be quiet\n"
	   "\n");
    
    exit(1);
}

int
mkjqfs(int count, char **files, int offset)
{
    int size, i, len;
    uint8_t *dir, *data;

    dir = buf;
    /*
     * 2 for main dir
     * 2 for flash size
     * 1 for length of subdir
     * 2 for each file
     */
    size = (2 + 2 + 1 + 2 * count) * sizeof(uint32_t);
    data = dir + size;

    w32le(&dir, 1);		/* Number of Subdirs */
    w32le(&dir, offset + 16);	/* Offset of first subdir */
    w32le(&dir, flashsize);     /* Flash size */
    w32le(&dir, ~flashsize);    /* Inverse of the flash size */
    w32le(&dir, count);		/* Number of files in subdir 1 */

    for (i = 0; i < count; i++) {
	int fd;
	char *fname = files[i];
	len = filesize(fname);
	size += len;
	if (!force && size >= maxsize) {
	    errx(1, "length %d exceeds maximum of %d\n", size, maxsize);
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

int
jq_erase(int dev, int offset)
{
    struct jqcmd cmd;
    struct sg_io_hdr hdr;
    
    memset(&cmd, 0, sizeof(cmd));
    cmd.j_cmd = htobe16(JQ_ERASE);
    cmd.j_off = htobe32(offset);
    
    memset(&hdr, 0, sizeof(hdr));
    hdr.interface_id = 'S';
    hdr.timeout = TIMEOUT;
    hdr.cmdp = (unsigned char *) &cmd;
    hdr.cmd_len = sizeof(cmd);
    hdr.dxfer_direction = SG_DXFER_NONE;
    
    return ioctl(dev, SG_IO, &hdr);
}

int
jq_write(int dev, uint8_t *buf, int offset, int size)
{
    struct jqcmd cmd;
    struct sg_io_hdr hdr;

    memset(&cmd, 0, sizeof(cmd));
    cmd.j_cmd = htobe16(JQ_WRITE);
    cmd.j_off = htobe32(offset);
    cmd.j_len = htobe32(size);
    
    memset(&hdr, 0, sizeof(hdr));
    hdr.interface_id = 'S';
    hdr.timeout = TIMEOUT;
    hdr.cmdp = (unsigned char *) &cmd;
    hdr.cmd_len = sizeof(cmd);
    hdr.dxfer_direction = SG_DXFER_TO_DEV;
    hdr.dxferp = buf;
    hdr.dxfer_len = size;
    
    return ioctl(dev, SG_IO, &hdr);
}

int
jq_read(int dev, uint8_t *buf, int offset, int size)
{
    struct jqcmd cmd;
    struct sg_io_hdr hdr;

    memset(&cmd, 0, sizeof(cmd));
    cmd.j_cmd = htobe16(JQ_READ);
    cmd.j_off = htobe32(offset);
    cmd.j_len = htobe32(size);
    
    memset(&hdr, 0, sizeof(hdr));
    hdr.interface_id = 'S';
    hdr.timeout = TIMEOUT;
    hdr.cmdp = (unsigned char *) &cmd;
    hdr.cmd_len = sizeof(cmd);
    hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    hdr.dxferp = buf;
    hdr.dxfer_len = size;
    
    return ioctl(dev, SG_IO, &hdr);
}

void
jq_flash(char *device, uint8_t *buf, int offset, int size)
{
    int dev, i;

    dev = open(device, O_RDWR);
    if (dev < 0)
	err(1, "cannot open device %s", device);

    p(1, "uploading %d bytes at offset 0x%x ...\n", size, offset);

    for (i = 0; i < size; i += BLKSZ) {
	
	loadbar(i, size);
	if (jq_erase(dev, offset + i)) {
	    err(1, "erase failed at offset %x", offset + i);
	}

	if (jq_write(dev, buf + i, offset + i, BLKSZ)) {
	    err(1, "\nwrite failed at offset %x", offset + i);
	}
    }
    loadbar(size, size);
    p(1, "\n");
    close(dev);
}

int
jq_identify(char *devname)
{
    int dev;
    int ret = 0;
    struct sg_io_hdr hdr;
    char buf[36];
    char vendor[9], product[17], revision[5];
    uint8_t cmd[6] = {INQUIRY, 0, 0, 0, sizeof(buf), 0};
    
    p(3, "checking %s\n", devname);

    dev = open(devname, O_RDONLY);
    if (dev < 0) {
	p(3, "  open failed on %s: %s\n", devname, strerror(errno));
	goto error2;
    }

    memset(&hdr, 0, sizeof(hdr));
    hdr.interface_id = 'S';
    hdr.timeout = TIMEOUT;
    hdr.cmdp = cmd;
    hdr.cmd_len = sizeof(cmd);
    hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    hdr.dxfer_len = sizeof(buf);
    hdr.dxferp = buf;
    if (ioctl(dev, SG_IO, &hdr) != 0) {
	p(4, "  INQUIRY failed on %s: %s", devname, strerror(errno));
	goto error1;
    }

    strncpy(vendor, buf + 8, sizeof(vendor)-1);
    vendor[sizeof(vendor)-1] = '\0';
    strncpy(product, buf + 16, sizeof(product)-1);
    product[sizeof(product)-1] = '\0';
    strncpy(revision, buf + 32, sizeof(revision)-1);
    revision[sizeof(revision)-1] = '\0';
    
    p(4, "  VENDOR   = '%s'\n", vendor);
    p(4, "  PRODUCT  = '%s'\n", product);
    p(4, "  REVISION = '%s'\n", revision);
    
    ret = (strcmp(vendor, "YULIN   ") == 0 &&
	   strcmp(product, " PROGRAMMER     ") == 0);

 error1:
    close(dev);
    
 error2: 
    p(2, "found %s JQ6500 module on %s\n", ret ? "a" : "no", devname);
    p(3, "\n");

    return ret;
}

char *
jq_find(const char *pattern)
{
    glob_t gb;
    int i, ret;
    char *devname = NULL;
    
    ret = glob(pattern, GLOB_NOSORT | GLOB_ERR, NULL, &gb);   
    if (ret != 0) {
	return NULL;
    }
    for (i = 0; i < gb.gl_pathc; i++) {
	if (jq_identify(gb.gl_pathv[i])) {
	    devname = strdup(gb.gl_pathv[i]);
	}
    }
    globfree(&gb);

    if (devname == NULL) {
	warnx("cannot find a JQ6500 module.");
    }
    if (verbosity < 2) {
	p(1, "found a JQ6500 module on %s\n", devname);
    } else {
	p(1, "using %s\n", devname);
    }
    return devname;
}


int
powerof2(unsigned int v)
{
    return v && !(v & (v - 1));
}

/*
 * Read the size from the modified jqfs, if it exists there but
 * only accept it after some sanity tests.
 */
int
jq_readsize(char *device)
{
    int dev;
    uint32_t size;
    union {
	uint8_t bytes[8];
	struct {
	    uint32_t size;
	    uint32_t check;
	};
    } sz;

    dev = open(device, O_RDWR);
    if (dev < 0)
	err(1, "cannot open device %s", device);

    jq_read(dev, (uint8_t*)&sz.bytes, 0x40008, 8);
    close(dev);
    size = le32toh(sz.size);

    /* Sanity checks ... */
    if (size != ~le32toh(sz.check)) return 0;
    if (size < 0x100000) return 0;
    if (!powerof2(size)) return 0;
    
    p(1, "flash size was saved on the module: %d MiB\n", size / 0x100000);
    return size;
}

int
jq_flashdetect(char *device, int offset)
{
    int dev;
    int i;
    int size = 0;
    int found = -1;
    uint8_t bufa[BLKSZ];
    uint8_t bufb[BLKSZ];
    uint8_t nulls[BLKSZ];
    uint8_t ffs[BLKSZ];
    dev = open(device, O_RDWR);
    if (dev < 0)
	err(1, "cannot open device %s", device);

    memset(nulls, 0, BLKSZ);
    memset(ffs, 0xff, BLKSZ);

    /*
     * Search the first megabyte starting at offset for a block with
     * non-trivial content.
     */
    for (i = offset; i < 1024 * 1024; i += BLKSZ) {
	if (jq_read(dev, bufa, i, BLKSZ)) {
	    err(1, "cannot read %d bytes from %s, offset 0x%x",
		BLKSZ, device, i);
	}
	if (memcmp(bufa, ffs, BLKSZ) == 0) {
	    p(3, "block %d is all ones, skipping...\n", i/BLKSZ);
	} else if (memcmp(bufa, nulls, BLKSZ) == 0) {
	    p(3, "block %d is all zeroes, skipping...\n", i/BLKSZ);
	} else {
	    found = i;
	    break;
	}
    }
    if (found < 0) {
	errx(1, "flash content is too trivial for size detection");
    }
    /*
     * Chcek whether the content of the non-trivial block we found
     * repeats after 1, 2, 4, 8, or 16 Megabytes.
     */
    for (i = 1; i <= 16; i *= 2) {
	int sz = i * 1024 * 1024;
	if (jq_read(dev, bufb, sz + found, BLKSZ)) {
	    err(1, "cannot read %d bytes from %s, offset 0x%x",
		BLKSZ, device, sz);
	}
	p(2, "comparing contents at 0x%x and 0x%x... ", found, sz + found);
	if (memcmp(bufa, bufb, BLKSZ) == 0) {
	    p(2, "OK\n");
	    p(1, "detected a flash size of %d MiB\n", size / (1024*1024));
	    size = sz;
	    break;
	} else {
	    p(2, "FAIL\n", i);
	}
    }
    close(dev);
    if (size == 0) {
	errx(1, "Cannot detect flash size. Is it larger than 16MiB?\n");
    }
    return size;
}

int
jq_flashsize(char *device, int offset)
{
    int size;

    size = jq_readsize(device);
    if (size == 0) {
	size = jq_flashdetect(device, offset);
    }
    return size;
}

unsigned int
parseint(char *str)
{
    unsigned int val;
    char *endptr;
    errno = 0;
    val = strtol(str, &endptr, 0);
    if (errno != 0 || *str == 0) {
	errx(1, "invalid numsber: '%s'", str);
    }
    switch (*endptr | ('a' - 'A')) {
    case 'm':
	val *= 256;
    case 'p':
	val *= 4;
    case 'k':
	val *= 1024;
    case 'b':
	endptr++;
    }
    if (*endptr != '\0') {
	errx(1, "invalid number: '%s' '%s'", endptr, str);
    }
    return val;
}

int
main(int argc, char **argv)
{
    int opt;
    char *device = NULL;
    int offset = BASE;
    int size = -1;
    int rawmode = 0;
    int patchmode = 0;
    int dev;

    if (argc == 1) {
	usage();
    }

    cmd = getopt(argc, argv, "LSRWEh");
    if (cmd == -1 || cmd == '?' || cmd == 'h') {
	usage();
    }
    while ((opt = getopt(argc, argv, "d:o:rs:fv:pqh")) != -1) {
	switch (opt) {
	case 'd':
	    device = optarg;
	    break;
	case 'o':
	    offset = parseint(optarg);
	    break;
	case 's':
	    size = parseint(optarg);
	    break;
	case 'r':
	    rawmode = 1;
	    break;
	case 'f':
	    force = 1;
	    break;
	case 'v':
	    verbosity = parseint(optarg);
	    break;
	case 'p':
	    patchmode = 1;
	    break;
	case 'q':
	    verbosity = 0;
	    break;
	default:
	    usage();
	}
    }

    if (optind == argc && cmd == 'W') {
	errx(1, "missing file operand.");
    }

    if (device) {
	if (force) {
	    p(0, "using %s as a JQ6500 module without check.\n", device);
	} else if (!jq_identify(device)) {
	    errx(1, "%s does not look like a JQ6500 module, add -f to use it anyway.", device);
	}
    } else {
	device = jq_find("/dev/sg*");
	if (!device) {
	    exit(1);
	}
	if (cmd == 'L') {
	    exit(0);
	}
    }
    if (!force && isdevice(device)) {
	flashsize = jq_flashsize(device, 0);
    }
    maxsize = flashsize - offset;
    buf = malloc(flashsize);
    memset(buf, 0xff, flashsize);

    if (!force) {
	if (offset < 0 || offset >= flashsize) {
	    errx(1, "offset out of range [0..0x%x].", flashsize);
	}
	if (offset % BLKSZ != 0) {
	    errx(1, "offset must be a multiple of %d.", BLKSZ);
	}
    }

    switch (cmd) {

    case 'W':
	if (rawmode) {
	    char *fname = argv[optind];
	    int fd;

	    if (size < 0) {
		size = flashsize;
	    }
	    
	    p(3, "reading %s\n", fname);
	    fd = open(fname, O_RDONLY);
	    if (fd < 0) 
		err(1, "cannot open %s", fname);
	    if ((size = read(fd, buf, size)) < 0) {
		err(1, "cannot read %s", fname);
	    }	    
	    close(fd);

	    if (patchmode) {
		p(3, "Patching the image\n", fname);
		
		/* Write the ISO size in 2k blocks to the start of the image */
		uint32_t *isosize = (uint32_t *)buf;
		*isosize = size / 2048 - 1;
		
		/* Patch the flash size into the volume name */
		char volname[33];
		int len = snprintf(volname, 33, "JQ6500 %dM", flashsize / (1024*1024));
		memset (volname+len, ' ', 33-len);
		memcpy(buf + 0x8028, volname, 32);
		
		/* Patch the flash size into the ini file */
		char *ptr = memmem(buf, size, "Val=0x", 6);
		
		if (ptr != NULL) {
		    ptr += sprintf(ptr, "Val=%#09x", flashsize);
		    *ptr = 0x0d; /* Overwrite the terminating null byte */
		} else {
		    p(3, "cannot patch config.ini\n");
		}

#if 0
		/* Write out the patched ISO for debugging */
		fname = "a.out";
		fd = open(fname, O_WRONLY);
		if ((size = write(fd, buf, size)) < 0) {
		    err(1, "cannot write %s", fname);
		}
		close(fd);
		exit(0);
#endif
	    }
	} else {
	    size = mkjqfs(argc - optind, argv + optind, offset);
	}
	
	if (isdevice(device)) {
	    jq_flash(device, buf, offset, size);
	} else {	
	    int fd = open(device, O_WRONLY | O_CREAT, "0666");
	    if (fd < 0) 
		err(1, "cannot open %s", device);
	    if (write(fd, buf, size) != size) {
		err(1, "write failed in %s", device);
	    }
	    close(fd);
	}
	break;

    case 'R':
	if (size < 0) {
	    size = BLKSZ;
	}
	dev = open(device, O_RDWR);
	if (dev < 0)
	    err(1, "cannot open %s", device);
	p(1, "reading %d bytes from offset 0x%x.\n", size, offset);
	for (int i = 0; i<size; i += BLKSZ) {
	    int n = (size - i) > BLKSZ ? BLKSZ : (size - i);
	    if (jq_read(dev, buf, offset + i, n)) {
		err(1, "cannot read");
	    }
	    write(1, buf, n);
	}
	break;

    case 'E':
	if (size < 0) {
	    size = BLKSZ;
	}
	dev = open(device, O_RDWR);
	if (dev < 0) 
	    err(1, "cannot open %s", device);
	if (size < BLKSZ) size = BLKSZ;
	p(1, "erasing %d pages from offset 0x%x\n", size/BLKSZ, offset);
	for (int i = 0; i < size; i += BLKSZ) {
	    loadbar(i/BLKSZ, size/BLKSZ);
	    if (jq_erase(dev, offset + i)) {
		err(1, "erase failed at 0x%x", offset + i);
	    }
	}
	loadbar(size/BLKSZ, size/BLKSZ);
	p(1, "\n");
	break;

    case 'S':
	printf("%#09x\n", flashsize);
	break;
    }
    
    return 0;
}

/* Local Variables:  */
/* mode: c           */
/* c-basic-offset: 4 */
/* End:              */
