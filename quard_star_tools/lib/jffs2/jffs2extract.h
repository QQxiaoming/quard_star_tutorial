#ifndef JFFS2EXTRACT_H
#define JFFS2EXTRACT_H

#include <stdint.h>
#include "jffs2-user.h"

struct dir {
	struct dir *next;
	uint8_t type;
	uint8_t nsize;
	uint32_t ino;
	char name[256];
};

void putblock(char *b, size_t bsize, size_t * rsize,
        struct jffs2_raw_inode *n);

struct jffs2_raw_inode *find_raw_inode(uint32_t ino, uint32_t vcur);
struct jffs2_raw_dirent *resolvedirent(uint32_t, uint32_t,
		char *, uint8_t);
struct jffs2_raw_dirent *resolvename(uint32_t, char *, uint8_t);
struct jffs2_raw_dirent *resolveinode(uint32_t);

struct jffs2_raw_dirent *resolvepath0(uint32_t, const char *,
		uint32_t *, int);
struct jffs2_raw_dirent *resolvepath(uint32_t, const char *,
		uint32_t *);
struct dir *collectdir(uint32_t ino, struct dir *d);
void freedir(struct dir *);

void find_free(uint32_t *ino, uint64_t *offset);
void write_dir(const char *name, uint32_t pino, uint32_t ino, uint32_t timestamp,
				uint64_t offset, int add_cleanmarkers,int erase_block_size );
void write_file(const char *name, const unsigned char *buff, size_t size, 
				uint32_t pino, uint32_t ino, uint32_t timestamp,
				uint64_t offset, int add_cleanmarkers,int erase_block_size );
int deletenode(uint32_t ino);

void jffs2_init(uint8_t * data, uint64_t data_size);

#endif // JFFS2EXTRACT_H
