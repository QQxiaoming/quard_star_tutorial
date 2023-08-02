/* vi: set sw=4 ts=4: */
/*
 * jffs2extract v0.1: Extract the contents of a JFFS2 image file.
 *
 * Based on jffs2reader by Jari Kirma
 *
 * Copyright (c) 2014 Rickard Lyrenius
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the author be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 * not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * 
 *
 * Usage: jffs2extract {-t | -x} [-f imagefile] [-C path] [-v] [file1 [file2 ...]]
 *
 * Options mimic the 'tar' command as close as possible.
 *
 */

#define PROGRAM_NAME "jffs2reader"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "jffs2extract.h"
#include "common.h"

#if defined(__WINDOWS__)
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)
#endif

static uint8_t * ram_disk_data = nullptr;
static uint64_t ram_disk_size = 0;

#define SCRATCH_SIZE (5*1024*1024)

/* macro to avoid "lvalue required as left operand of assignment" error */
#define ADD_BYTES(p, n)		((p) = (typeof(p))((char *)(p) + (n)))

#define DIRENT_INO(dirent) ((dirent) !=NULL ? je32_to_cpu((dirent)->ino) : 0)
#define DIRENT_PINO(dirent) ((dirent) !=NULL ? je32_to_cpu((dirent)->pino) : 0)

int target_endian = __BYTE_ORDER;

void putblock(char *, size_t, size_t *, struct jffs2_raw_inode *);
struct dir *putdir(struct dir *, struct jffs2_raw_dirent *);
void printdir( struct dir *d, const char *path, 
     int verbose);
void freedir(struct dir *);



static int jffs2_rtime_decompress(unsigned char *data_in,
				  unsigned char *cpage_out,
				  uint32_t srclen, uint32_t destlen)
{
	short positions[256];
	int outpos = 0;
	int pos=0;
	memset(positions,0,sizeof(positions));
	while (outpos<destlen) {
		unsigned char value;
		int backoffs;
		int repeat;
		value = data_in[pos++];
		cpage_out[outpos++] = value; /* first the verbatim copied byte */
		repeat = data_in[pos++];
		backoffs = positions[value];
		positions[value]=outpos;
		if (repeat) {
			if (backoffs + repeat >= outpos) {
				while(repeat) {
					cpage_out[outpos++] = cpage_out[backoffs++];
					repeat--;
				}
			} else {
				memcpy(&cpage_out[outpos],&cpage_out[backoffs],repeat);
				outpos+=repeat;
			}
		}
	}
	return 0;
}

/* writes file node into buffer, to the proper position. */
/* reading all valid nodes in version order reconstructs the file. */

/*
   b       - buffer
   bsize   - buffer size
   rsize   - result size
   n       - node
 */

void putblock(char *b, size_t bsize, size_t * rsize,
		struct jffs2_raw_inode *n)
{
    unsigned long dlen = je32_to_cpu(n->dsize);

	if (je32_to_cpu(n->isize) > bsize || (je32_to_cpu(n->offset) + dlen) > bsize)
		errmsg_die("File does not fit into buffer!");

	if (*rsize < je32_to_cpu(n->isize))
		bzero(b + *rsize, je32_to_cpu(n->isize) - *rsize);

	switch (n->compr) {
		//TODO: QQM remove zlib dependency, I think maybe we can use https://github.com/MartinChan3/QZlib
		//case JFFS2_COMPR_ZLIB:
        //    uncompress((Bytef *) b + je32_to_cpu(n->offset), &dlen,
        //    		(Bytef *) ((char *) n) + sizeof(struct jffs2_raw_inode),
        //    		(uLongf) je32_to_cpu(n->csize));
		//	break;

		case JFFS2_COMPR_NONE:
			memcpy(b + je32_to_cpu(n->offset),
					((char *) n) + sizeof(struct jffs2_raw_inode), dlen);
			break;

		case JFFS2_COMPR_ZERO:
			bzero(b + je32_to_cpu(n->offset), dlen);
			break;

			/* [DYN]RUBIN support required! */
		case JFFS2_COMPR_RTIME:
			jffs2_rtime_decompress((unsigned char *) ((char *) n) + sizeof(struct jffs2_raw_inode),
					(unsigned char *) (b + je32_to_cpu(n->offset)),
                    je32_to_cpu(n->csize), je32_to_cpu(n->dsize));
			break;
		default:
			errmsg_die("Unsupported compression method %d!",n->compr);
	}

	*rsize = je32_to_cpu(n->isize);
}

/* adds/removes directory node into dir struct. */
/* reading all valid nodes in version order reconstructs the directory. */

/*
   dd      - directory struct being processed
   n       - node

   return value: directory struct value replacing dd
 */

struct dir *putdir(struct dir *dd, struct jffs2_raw_dirent *n)
{
	struct dir *o, *d, *p;

	o = dd;

	if (je32_to_cpu(n->ino)) {
        if (dd == NULL) {
            d = (struct dir *)xmalloc(sizeof(struct dir));
			d->type = n->type;
			memcpy(d->name, n->name, n->nsize);
			d->nsize = n->nsize;
			d->ino = je32_to_cpu(n->ino);
			d->next = NULL;

			return d;
		}

		while (1) {
			if (n->nsize == dd->nsize &&
					!memcmp(n->name, dd->name, n->nsize)) {
				dd->type = n->type;
				dd->ino = je32_to_cpu(n->ino);

				return o;
			}

			if (dd->next == NULL) {
                dd->next = (struct dir *)xmalloc(sizeof(struct dir));
				dd->next->type = n->type;
				memcpy(dd->next->name, n->name, n->nsize);
				dd->next->nsize = n->nsize;
				dd->next->ino = je32_to_cpu(n->ino);
				dd->next->next = NULL;

				return o;
			}

			dd = dd->next;
		}
	} else {
		if (dd == NULL)
			return NULL;

		if (n->nsize == dd->nsize && !memcmp(n->name, dd->name, n->nsize)) {
			d = dd->next;
			free(dd);
			return d;
		}

		while (1) {
			p = dd;
			dd = dd->next;

			if (dd == NULL)
				return o;

			if (n->nsize == dd->nsize &&
					!memcmp(n->name, dd->name, n->nsize)) {
				p->next = dd->next;
				free(dd);

				return o;
			}
		}
	}
}

#if defined(__WINDOWS__)
#define S_IFSOCK 0140000
#define S_IFLNK	 0120000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)
#endif

#define TYPEINDEX(mode) (((mode) >> 12) & 0x0f)
#define TYPECHAR(mode)  ("0pcCd?bB-?l?s???" [TYPEINDEX(mode)])

/* The special bits. If set, display SMODE0/1 instead of MODE0/1 */
static const mode_t SBIT[] = {
	0, 0, S_ISUID,
	0, 0, S_ISGID,
	0, 0, S_ISVTX
};

/* The 9 mode bits to test */
static const mode_t MBIT[] = {
	S_IRUSR, S_IWUSR, S_IXUSR,
	S_IRGRP, S_IWGRP, S_IXGRP,
	S_IROTH, S_IWOTH, S_IXOTH
};

static const char MODE1[] = "rwxrwxrwx";
static const char MODE0[] = "---------";
static const char SMODE1[] = "..s..s..t";
static const char SMODE0[] = "..S..S..T";

/*
 * Return the standard ls-like mode string from a file mode.
 * This is static and so is overwritten on each call.
 */
const char *mode_string(int mode)
{
	static char buf[12];

	int i;

	buf[0] = TYPECHAR(mode);
	for (i = 0; i < 9; i++) {
		if (mode & SBIT[i])
			buf[i + 1] = (mode & MBIT[i]) ? SMODE1[i] : SMODE0[i];
		else
			buf[i + 1] = (mode & MBIT[i]) ? MODE1[i] : MODE0[i];
	}
	return buf;
}

void freedir(struct dir *d)
{
	struct dir *t;

	while (d != NULL) {
		t = d->next;
		free(d);
		d = t;
	}
}

struct jffs2_raw_inode *find_raw_inode(uint32_t ino, 
	uint32_t vcur)
{
	/* aligned! */
	union jffs2_node_union *n;
	union jffs2_node_union *e = (union jffs2_node_union *) (ram_disk_data + ram_disk_size);
	union jffs2_node_union *lr;	/* last block position */
	union jffs2_node_union *mp = NULL;	/* minimum position */

	uint32_t vmin, vmint, vmaxt, vmax, v;

	vmin = 0;					/* next to read */
	vmax = ~((uint32_t) 0);		/* last to read */
	vmint = ~((uint32_t) 0);
	vmaxt = 0;					/* found maximum */

    n = (union jffs2_node_union *) ram_disk_data;
	lr = n;

	do {
		while (n < e && je16_to_cpu(n->u.magic) != JFFS2_MAGIC_BITMASK)
			ADD_BYTES(n, 4);

		if (n < e && je16_to_cpu(n->u.magic) == JFFS2_MAGIC_BITMASK) {
			if (je16_to_cpu(n->u.nodetype) == JFFS2_NODETYPE_INODE &&
				je32_to_cpu(n->i.ino) == ino && (v = je32_to_cpu(n->i.version)) > vcur) {
				/* XXX crc check */

				if (vmaxt < v)
					vmaxt = v;
				if (vmint > v) {
					vmint = v;
					mp = n;
				}

				if (v == (vcur + 1))
					return (&(n->i));
			}

			ADD_BYTES(n, ((je32_to_cpu(n->u.totlen) + 3) & ~3));
		} else
			n = (union jffs2_node_union *) ram_disk_data;	/* we're at the end, rewind to the beginning */

		if (lr == n) {			/* whole loop since last read */
			vmax = vmaxt;
			vmin = vmint;
			vmint = ~((uint32_t) 0);

			if (vcur < vmax && vcur < vmin)
				return (&(mp->i));
		}
	} while (vcur < vmax);

	return NULL;
}

struct dir *collectdir(uint32_t ino, struct dir *d)
{
	/* aligned! */
	union jffs2_node_union *n;
	union jffs2_node_union *e = (union jffs2_node_union *) (ram_disk_data + ram_disk_size);
	union jffs2_node_union *lr;	/* last block position */
	union jffs2_node_union *mp = NULL;	/* minimum position */

	uint32_t vmin, vmint, vmaxt, vmax, vcur, v;

	vmin = 0;					/* next to read */
	vmax = ~((uint32_t) 0);		/* last to read */
	vmint = ~((uint32_t) 0);
	vmaxt = 0;					/* found maximum */
	vcur = 0;					/* XXX what is smallest version number used? */
	/* too low version number can easily result excess log rereading */

    n = (union jffs2_node_union *) ram_disk_data;
	lr = n;

	do {
		while (n < e && je16_to_cpu(n->u.magic) != JFFS2_MAGIC_BITMASK)
			ADD_BYTES(n, 4);

		if (n < e && je16_to_cpu(n->u.magic) == JFFS2_MAGIC_BITMASK) {
			if (je16_to_cpu(n->u.nodetype) == JFFS2_NODETYPE_DIRENT &&
				je32_to_cpu(n->d.pino) == ino && (v = je32_to_cpu(n->d.version)) >= vcur) {
				/* XXX crc check */

				if (vmaxt < v)
					vmaxt = v;
				if (vmint > v) {
					vmint = v;
					mp = n;
				}

				if (v == (vcur)) {
					d = putdir(d, &(n->d));

					lr = n;
					vcur++;
					vmint = ~((uint32_t) 0);
				}
			}

			ADD_BYTES(n, ((je32_to_cpu(n->u.totlen) + 3) & ~3));
		} else
			n = (union jffs2_node_union *) ram_disk_data;	/* we're at the end, rewind to the beginning */

		if (lr == n) {			/* whole loop since last read */
			vmax = vmaxt;
			vmin = vmint;
			vmint = ~((uint32_t) 0);

			if (vcur <= vmax && vcur <= vmin) {
				d = putdir(d, &(mp->d));

				lr = n =
					(union jffs2_node_union *) (((char *) mp) +
							((je32_to_cpu(mp->u.totlen) + 3) & ~3));

				vcur = vmin;
			}
		}
    } while (vcur <= vmax);

	return d;
}



/* resolve dirent based on criteria */
/*
   o       - filesystem image pointer
   size    - size of filesystem image
   ino     - if zero, ignore,
   otherwise compare against dirent inode
   pino    - if zero, ingore,
   otherwise compare against parent inode
   and use name and nsize as extra criteria
   name    - name of wanted dirent, used if pino!=0
   nsize   - length of name of wanted dirent, used if pino!=0

   return value: pointer to relevant dirent structure in
   filesystem image or NULL
 */

struct jffs2_raw_dirent *resolvedirent(
		uint32_t ino, uint32_t pino,
		char *name, uint8_t nsize)
{
	/* aligned! */
	union jffs2_node_union *n;
	union jffs2_node_union *e = (union jffs2_node_union *) (ram_disk_data + ram_disk_size);

	struct jffs2_raw_dirent *dd = NULL;

	uint32_t vmax, v;

	if (!pino && ino <= 1)
		return dd;

	vmax = 0;

    n = (union jffs2_node_union *) ram_disk_data;

	do {
		while (n < e && je16_to_cpu(n->u.magic) != JFFS2_MAGIC_BITMASK)
			ADD_BYTES(n, 4);

		if (n < e && je16_to_cpu(n->u.magic) == JFFS2_MAGIC_BITMASK) {
			if (je16_to_cpu(n->u.nodetype) == JFFS2_NODETYPE_DIRENT &&
					(!ino || je32_to_cpu(n->d.ino) == ino) &&
					(v = je32_to_cpu(n->d.version)) >= vmax &&
					(!pino || (je32_to_cpu(n->d.pino) == pino &&
							   nsize == n->d.nsize &&
							   !memcmp(name, n->d.name, nsize)))) {
				/* XXX crc check */

				if (vmax <= v) {
					vmax = v;
					dd = &(n->d);
				}
			}

			ADD_BYTES(n, ((je32_to_cpu(n->u.totlen) + 3) & ~3));
		} else
			return dd;
	} while (1);
}

/* resolve name under certain parent inode to dirent */

/*
   o       - filesystem image pointer
   size    - size of filesystem image
   pino    - requested parent inode
   name    - name of wanted dirent
   nsize   - length of name of wanted dirent

   return value: pointer to relevant dirent structure in
   filesystem image or NULL
 */

struct jffs2_raw_dirent *resolvename( uint32_t pino,
		char *name, uint8_t nsize)
{
	return resolvedirent(0, pino, name, nsize);
}

/* resolve inode to dirent */

/*
   o       - filesystem image pointer
   size    - size of filesystem image
   ino     - compare against dirent inode

   return value: pointer to relevant dirent structure in
   filesystem image or NULL
 */

struct jffs2_raw_dirent *resolveinode(uint32_t ino)
{
	return resolvedirent(ino, 0, NULL, 0);
}

struct jffs2_raw_dirent *resolvepath0(uint32_t ino,
		const char *p, uint32_t * inos, int recc)
{
	struct jffs2_raw_dirent *dir = NULL;

	int d = 1;
	uint32_t tino;

	char *next;

	char *path, *pp;

	char symbuf[1024];
	size_t symsize;

	if (recc > 16) {
		/* probably symlink loop */
		*inos = 0;
		return NULL;
	}

	pp = path = strdup(p);

	if (*path == '/') {
		path++;
        ino = 1;
    }

	if (ino > 1) {
		dir = resolveinode(ino);

		ino = DIRENT_INO(dir);
	}

    next = path - 1;
	while (ino && next != NULL && next[1] != 0 && d) {
		path = next + 1;
		next = strchr(path, '/');
		
		if (next != NULL)
			*next = 0;

		if (*path == '.' && path[1] == 0)
			continue;
		if (*path == '.' && path[1] == '.' && path[2] == 0) {
			if (DIRENT_PINO(dir) == 1) {
				ino = 1;
				dir = NULL;
			} else {
				dir = resolveinode(DIRENT_PINO(dir));
				ino = DIRENT_INO(dir);
			}

			continue;
		}

		dir = resolvename(ino, path, (uint8_t) strlen(path));

		if (DIRENT_INO(dir) == 0 ||
				(next != NULL &&
				 !(dir->type == DT_DIR || dir->type == DT_LNK))) {
			free(pp);

			*inos = 0;

			return NULL;
		}

		if (dir->type == DT_LNK) {
			struct jffs2_raw_inode *ri;
			ri = find_raw_inode(DIRENT_INO(dir), 0);
			putblock(symbuf, sizeof(symbuf), &symsize, ri);
			symbuf[symsize] = 0;

			tino = ino;
			ino = 0;

			dir = resolvepath0(tino, symbuf, &ino, ++recc);

			if (dir != NULL && next != NULL &&
					!(dir->type == DT_DIR || dir->type == DT_LNK)) {
				free(pp);

				*inos = 0;
				return NULL;
			}
		}
		if (dir != NULL)
			ino = DIRENT_INO(dir);
	}

	free(pp);

	*inos = ino;

	return dir;
}

struct jffs2_raw_dirent *resolvepath(uint32_t ino,
		const char *p, uint32_t * inos)
{
	return resolvepath0(ino, p, inos, 0);
}

void jffs2_init(uint8_t * data, uint64_t data_size) {
	ram_disk_data = data;
	ram_disk_size = data_size;
}
