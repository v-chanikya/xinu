#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS
#include <fs.h>

#define FS_DEBUG
static fsystem_t fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0 // Superblock
#define BM_BLK 1 // Bitmapblock

#define NUM_FD 16

filetable_t oft[NUM_FD]; // open file table
#define isbadfd(fd) (fd < 0 || fd >= NUM_FD || oft[fd].in.id == EMPTY)

#define INODES_PER_BLOCK (fsd.blocksz / sizeof(inode_t))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

/**
 * Helper functions
 */
int _fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEDIRECTBLOCKS) {
    errormsg("No indirect block support! (%d >= %d)\n", fileblock, INODEBLOCKS - 2);
    return SYSERR;
  }

  // Get the logical block address
  diskblock = oft[fd].in.blocks[fileblock];

  return diskblock;
}

/**
 * Filesystem functions
 */
int _fs_get_inode_by_num(int dev, int inode_number, inode_t *out) {
  int bl, inn;
  int inode_off;

  if (dev != dev0) {
    errormsg("Unsupported device: %d\n", dev);
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    errormsg("inode %d out of range (> %s)\n", inode_number, fsd.ninodes);
    return SYSERR;
  }

  bl  = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(inode_t);

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(out, &block_cache[inode_off], sizeof(inode_t));

  return OK;

}

int _fs_put_inode_by_num(int dev, int inode_number, inode_t *in) {
  int bl, inn;

  if (dev != dev0) {
    errormsg("Unsupported device: %d\n", dev);
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    errormsg("inode %d out of range (> %d)\n", inode_number, fsd.ninodes);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(inode_t))], in, sizeof(inode_t));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}

int fs_mkfs(int dev, int num_inodes) {
  int i;

  if (dev == dev0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  } else {
    errormsg("Unsupported device: %d\n", dev);
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  } else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) { i++; }
  fsd.freemaskbytes = i / 8;

  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *) SYSERR) {
    errormsg("fs_mkfs memget failed\n");
    return SYSERR;
  }

  /* zero the free mask */
  for(i = 0; i < fsd.freemaskbytes; i++) {
    fsd.freemask[i] = '\0';
  }

  fsd.inodes_used = 0;

  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(fsystem_t));

  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  // Initialize all inode IDs to EMPTY
  inode_t tmp_in;
  for (i = 0; i < fsd.ninodes; i++) {
    _fs_get_inode_by_num(dev0, i, &tmp_in);
    tmp_in.id = EMPTY;
    _fs_put_inode_by_num(dev0, i, &tmp_in);
  }
  fsd.root_dir.numentries = 0;
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    fsd.root_dir.entry[i].inode_num = EMPTY;
    memset(fsd.root_dir.entry[i].name, 0, FILENAMELEN);
  }

  for (i = 0; i < NUM_FD; i++) {
    oft[i].state     = 0;
    oft[i].fileptr   = 0;
    oft[i].de        = NULL;
    oft[i].in.id     = EMPTY;
    oft[i].in.type   = 0;
    oft[i].in.nlink  = 0;
    oft[i].in.device = 0;
    oft[i].in.size   = 0;
    memset(oft[i].in.blocks, 0, sizeof(oft[i].in.blocks));
    oft[i].flag      = 0;
  }

  return OK;
}

int fs_freefs(int dev) {
  if (freemem(fsd.freemask, fsd.freemaskbytes) == SYSERR) {
    return SYSERR;
  }

  return OK;
}

/**
 * Debugging functions
 */
void fs_print_oft(void) {
  int i;

  printf ("\n\033[35moft[]\033[39m\n");
  printf ("%3s  %5s  %7s  %8s  %6s  %5s  %4s  %s\n", "Num", "state", "fileptr", "de", "de.num", "in.id", "flag", "de.name");
  for (i = 0; i < NUM_FD; i++) {
    if (oft[i].de != NULL) printf ("%3d  %5d  %7d  %8d  %6d  %5d  %4d  %s\n", i, oft[i].state, oft[i].fileptr, oft[i].de, oft[i].de->inode_num, oft[i].in.id, oft[i].flag, oft[i].de->name);
  }

  printf ("\n\033[35mfsd.root_dir.entry[] (numentries: %d)\033[39m\n", fsd.root_dir.numentries);
  printf ("%3s  %3s  %s\n", "ID", "id", "filename");
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    if (fsd.root_dir.entry[i].inode_num != EMPTY) printf("%3d  %3d  %s\n", i, fsd.root_dir.entry[i].inode_num, fsd.root_dir.entry[i].name);
  }
  printf("\n");
}

void fs_print_inode(int fd) {
  int i;

  printf("\n\033[35mInode FS=%d\033[39m\n", fd);
  printf("Name:    %s\n", oft[fd].de->name);
  printf("State:   %d\n", oft[fd].state);
  printf("Flag:    %d\n", oft[fd].flag);
  printf("Fileptr: %d\n", oft[fd].fileptr);
  printf("Type:    %d\n", oft[fd].in.type);
  printf("nlink:   %d\n", oft[fd].in.nlink);
  printf("device:  %d\n", oft[fd].in.device);
  printf("size:    %d\n", oft[fd].in.size);
  printf("blocks: ");
  for (i = 0; i < INODEBLOCKS; i++) {
    printf(" %d", oft[fd].in.blocks[i]);
  }
  printf("\n");
  return;
}

void fs_print_fsd(void) {
  int i;

  printf("\033[35mfsystem_t fsd\033[39m\n");
  printf("fsd.nblocks:       %d\n", fsd.nblocks);
  printf("fsd.blocksz:       %d\n", fsd.blocksz);
  printf("fsd.ninodes:       %d\n", fsd.ninodes);
  printf("fsd.inodes_used:   %d\n", fsd.inodes_used);
  printf("fsd.freemaskbytes  %d\n", fsd.freemaskbytes);
  printf("sizeof(inode_t):   %d\n", sizeof(inode_t));
  printf("INODES_PER_BLOCK:  %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS:  %d\n", NUM_INODE_BLOCKS);

  inode_t tmp_in;
  printf ("\n\033[35mBlocks\033[39m\n");
  printf ("%3s  %3s  %4s  %4s  %3s  %4s\n", "Num", "id", "type", "nlnk", "dev", "size");
  for (i = 0; i < NUM_FD; i++) {
    _fs_get_inode_by_num(dev0, i, &tmp_in);
    if (tmp_in.id != EMPTY) printf("%3d  %3d  %4d  %4d  %3d  %4d\n", i, tmp_in.id, tmp_in.type, tmp_in.nlink, tmp_in.device, tmp_in.size);
  }
  for (i = NUM_FD; i < fsd.ninodes; i++) {
    _fs_get_inode_by_num(dev0, i, &tmp_in);
    if (tmp_in.id != EMPTY) {
      printf("%3d:", i);
      int j;
      for (j = 0; j < 64; j++) {
        printf(" %3d", *(((char *) &tmp_in) + j));
      }
      printf("\n");
    }
  }
  printf("\n");
}

void fs_print_dir(void) {
  int i;

  printf("%22s  %9s  %s\n", "DirectoryEntry", "inode_num", "name");
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    printf("fsd.root_dir.entry[%2d]  %9d  %s\n", i, fsd.root_dir.entry[i].inode_num, fsd.root_dir.entry[i].name);
  }
}

int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
}

int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/**
 * This is maybe a little overcomplicated since the lowest-numbered
 * block is indicated in the high-order bit.  Shift the byte by j
 * positions to make the match in bit7 (the 8th bit) and then shift
 * that value 7 times to the low-order bit to print.  Yes, it could be
 * the other way...
 */
void fs_printfreemask(void) { // print block bitmask
  int i, j;

  for (i = 0; i < fsd.freemaskbytes; i++) {
    for (j = 0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    printf(" ");
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}


/**
 * TODO: implement the functions below
 */
int fs_open(char *filename, int flags) {

    int filename_length = strlen(filename);
    if ((flags != O_RDWR &&
            flags != O_WRONLY &&
            flags != O_RDONLY ) ||
            filename_length > FILENAMELEN)
        return SYSERR;

    // Check if file already exists
    for (int i = 0; i < fsd.root_dir.numentries; i++){
        if (strncmp(fsd.root_dir.entry[i].name, filename, filename_length) == 0){
            inode_t fnode;
            _fs_get_inode_by_num(dev0, fsd.root_dir.entry[i].inode_num, &fnode);

            // Check if fd already exists
            if (fnode.id != EMPTY){
                if (oft[fnode.id].state == FSTATE_OPEN){
                    return SYSERR;
                }
                else{
                    oft[fnode.id].state = FSTATE_OPEN;
                    return fnode.id;
                }
            }

            // Create new fd
            for (int j = 0; j < NUM_FD; j++){
                if(oft[j].in.id == EMPTY){
                    fnode.id = i;
                    oft[j].state    = FSTATE_OPEN;
                    oft[j].fileptr  = 0;
                    oft[j].de       = &fsd.root_dir.entry[i];
                    oft[j].flag     = flags;
                    memcpy(&oft[j].in, &fnode, sizeof(inode_t));
                    return j;
                }
            }
            break;
        }
    }
    return SYSERR;
}

int fs_close(int fd) {
    if (isbadfd(fd))
        return SYSERR;
    if (oft[fd].state == FSTATE_CLOSED)
        return SYSERR;
    oft[fd].state   = FSTATE_CLOSED;
    oft[fd].flag    = 0;
    oft[fd].fileptr = 0;
    return OK;
}

int fs_create(char *filename, int mode) {
    int filename_length = strlen(filename);
    // Check if directory is full
    if (fsd.ninodes == fsd.inodes_used ||
            mode != O_CREAT ||
            filename_length > FILENAMELEN)
        return SYSERR;

    // Check if file already exists
    for (int i = 0; i < fsd.root_dir.numentries; i++){
        if (strncmp(fsd.root_dir.entry[i].name, filename, filename_length) == 0){
            for (int j = 0; j < NUM_FD; j++){
                if (oft[j].de == &fsd.root_dir.entry[i]){
                    if (oft[j].state == FSTATE_CLOSED){
                        oft[j].state    = FSTATE_OPEN;
                        oft[j].flag     = O_RDWR;
                        return j;
                    }
                    break;
                }
            }
            return SYSERR;
        }
    }

    // Find free inode
    int free_node_id = EMPTY;
    inode_t free_node;
    for (int i = 0; i < fsd.ninodes; i++){
        _fs_get_inode_by_num(dev0, i, &free_node);
        if(free_node.id == EMPTY){
            free_node_id = i;
            break;
        }
    }
    if (free_node_id == EMPTY)
        return SYSERR;

    // Add inode to fsd
    fsd.root_dir.entry[fsd.inodes_used].inode_num = free_node_id;
    memcpy(fsd.root_dir.entry[fsd.inodes_used].name, filename, filename_length);
    fsd.inodes_used += 1;
    fsd.root_dir.numentries += 1;
    bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(fsystem_t));

    // Initialize inode
    free_node.type      = INODE_TYPE_FILE;
    free_node.nlink     = 1;
    free_node.device    = dev0;
    free_node.size      = 0;
    free_node.id        = free_node_id;

    // Get free fd
    int free_fd = EMPTY;
    for (int i = 0; i < NUM_FD; i++){
        /* if(oft[i].in.id == EMPTY){ */
        if(oft[i].state == FSTATE_CLOSED){
            free_fd = i;
            free_node.id = i;
            // Open file with O_RDWR
            oft[i].state    = FSTATE_OPEN;
            oft[i].fileptr  = 0;
            oft[i].de       = &fsd.root_dir.entry[fsd.inodes_used - 1];
            oft[i].flag     = O_RDWR;
            memcpy(&oft[i].in, &free_node, sizeof(inode_t));
            break;
        }
    }
    _fs_put_inode_by_num(dev0, free_node_id, &free_node);
    // Return FD
    if (free_fd == EMPTY)
        return SYSERR;
    else
        return free_fd;
}

inline int check_fd(int fd){
    if (isbadfd(fd))
        return SYSERR;
    if (oft[fd].in.id == EMPTY)
        return SYSERR;
    if (oft[fd].state == FSTATE_CLOSED)
        return SYSERR;
    return OK;
}

int fs_seek(int fd, int offset) {
    if (check_fd(fd) == SYSERR ||
            offset < 0 ||
            offset > oft[fd].in.size)
        return SYSERR;
    
    oft[fd].fileptr = offset;
    return OK;
}


/* int fs_read(int fd, void *buf, int nbytes) */
/* { */
/*     if (isbadfd(fd) || oft[fd].state == FSTATE_CLOSED || oft[fd].flag == O_WRONLY) */
/*     { */
/*         return SYSERR; */
/*     } */
/*     char buffer[fsd.blocksz * INODEBLOCKS]; */
/*     int size = 0; */
/*     int blocks_to_read = 0; */
/*     int already_read = 0; */
/*     int index = 0; */

/*     while (size != oft[fd].in.size) */
/*     { */
/*         int size_diff = oft[fd].in.size - size; */
/*         blocks_to_read = size_diff < fsd.blocksz ? size_diff : fsd.blocksz; */

/*         index = oft[fd].in.blocks[already_read++]; */

/*         fs_clearmaskbit(index); */
/*         bs_bread(0, index, 0, buf + size, blocks_to_read); */
/*         size += blocks_to_read; */
/*     } */

/*     memcpy(buf, &buffer[oft[fd].fileptr], nbytes); */
/*     oft[fd].fileptr += nbytes; */

/*     return nbytes; */
/* } */

int fs_write(int fd, void *buf, int nbytes)
{
    if (oft[fd].state == FSTATE_CLOSED || oft[fd].flag == O_RDONLY || isbadfd(fd))
    {
        return SYSERR;
    }
    char buffer[fsd.blocksz * INODEBLOCKS];

    int fp = oft[fd].fileptr;
    int size = 0;
    int blocks_to_read = 0;
    int already_read = 0;
    int index = 0;

    if (fp + nbytes > fsd.blocksz * INODEBLOCKS){
        nbytes = (fsd.blocksz * INODEBLOCKS) - fp;
    }
    while (size != oft[fd].in.size)
    {
        int size_diff = oft[fd].in.size - size;
        blocks_to_read = size_diff < fsd.blocksz ? size_diff : fsd.blocksz;

        index = oft[fd].in.blocks[already_read++];

        fs_clearmaskbit(index);
        bs_bread(0, index, 0, buf + size, blocks_to_read);
        size += blocks_to_read;
    }

    memcpy(&buffer[fp], buf, nbytes);
    fp += nbytes;

    int free_bytes = nbytes;
    int free_block = fsd.nblocks + 1;
    int bytes;
    void *bufptr = buf;
    int i = INODEBLOCKS + 2;
    int block_index = 0;
    oft[fd].in.size = 0;
    oft[fd].fileptr = 0;
    while (free_bytes > 0)
    {
        if (block_index >= INODEBLOCKS)
            break;
        for (; i <= fsd.nblocks; i++)
        {
            if (fs_getmaskbit(i) == 0)
            {
                free_block = i;
                break;
            }
        }

        if (i > fsd.nblocks)
        {
            break;
        }

        if (free_bytes >= fsd.blocksz)
        {
            bytes = fsd.blocksz;
            free_bytes -= fsd.blocksz;
        }
        else
        {
            bytes = free_bytes;
            free_bytes = 0;
        }

        bs_bwrite(0, free_block, 0, bufptr, bytes);
        bufptr += bytes;
        oft[fd].fileptr += bytes;
        oft[fd].in.blocks[block_index++] = free_block;
        oft[fd].in.size += bytes;
        fs_setmaskbit(free_block);
        free_block = fsd.nblocks + 1;
    }

    _fs_put_inode_by_num(0, oft[fd].de->inode_num, &oft[fd].in);

    oft[fd].fileptr = fp;

    return nbytes;
}

int fs_read(int fd, void *buf, int nbytes) {
    if (check_fd(fd) == SYSERR ||
            buf == NULL ||
            nbytes <= 0 ||
            oft[fd].flag == O_WRONLY)
        return SYSERR;

    int initfp = oft[fd].fileptr == 0 ? 0 : oft[fd].fileptr;
    int starting_block = initfp / MDEV_BLOCK_SIZE;
    int offset = initfp % MDEV_BLOCK_SIZE;
    int bytes_read = 0;
    int read_size = 0;

    // read more than present data
    if (initfp + nbytes > oft[fd].in.size)
        nbytes = oft[fd].in.size - initfp;
    
    // read block to memory and copy appropriate location in buf
    while (nbytes > 0){
        if (starting_block >= INODEBLOCKS)
            break;
        bs_bread(dev0, oft[fd].in.blocks[starting_block], 0, block_cache, fsd.blocksz);
        if (MDEV_BLOCK_SIZE - offset > nbytes)
            read_size = nbytes;
        else
            read_size = MDEV_BLOCK_SIZE - offset;
        memcpy(((char*)buf) + bytes_read, block_cache + offset, read_size);

        bytes_read += read_size;
        starting_block++;
        offset = 0;
        nbytes -= read_size;
        /* printf("read_size: %d\tbytes read : %d\tstarting_block : %d\tnbytes : %d\toffset : %d\n", read_size, bytes_read, starting_block, nbytes, offset); */
    }

    // update fileptr for next use
    oft[fd].fileptr += bytes_read;
    return bytes_read;
}

/* int fs_write(int fd, void *buf, int nbytes) */
/* { */
/*     if (check_fd(fd) == SYSERR || */
/*             buf == NULL || */
/*             nbytes <= 0 || */
/*             oft[fd].flag == O_RDONLY) */
/*         return SYSERR; */


/*     int starting_block = oft[fd].fileptr / MDEV_BLOCK_SIZE; */
/*     // adjust blocks */
/*     int new_blocks = (oft[fd].fileptr + nbytes) / MDEV_BLOCK_SIZE; */
/*     int old_blocks = oft[fd].in.size / MDEV_BLOCK_SIZE; */

/*     // Allocate first block */
/*     if (oft[fd].in.size == 0){ */
/*         for (int i = 0; i < MDEV_NUM_BLOCKS; i++){ */
/*             if (fs_getmaskbit(i) == 0){ */
/*                 fs_setmaskbit(i); */
/*                 oft[fd].in.blocks[0] = i; */
/*                 break; */
/*             } */
/*         } */
/*     } */
/*     if (new_blocks > old_blocks){ */
/*         if (new_blocks >= INODEBLOCKS) */
/*             new_blocks = INODEBLOCKS - 1; */
/*         int no_new_blocks = new_blocks - old_blocks; */
/*         starting_block = old_blocks + 1; */
/*         for (int i = 0; i < MDEV_NUM_BLOCKS && no_new_blocks != 0; i++){ */
/*             if (fs_getmaskbit(i) == 0){ */
/*                 fs_setmaskbit(i); */
/*                 oft[fd].in.blocks[starting_block] = i; */
/*                 starting_block++; */
/*                 no_new_blocks--; */
/*             } */
/*         } */
/*     } */
/*     else if (new_blocks < old_blocks){ */
/*         starting_block = new_blocks + 1; */
/*         int no_blocks_to_free = old_blocks - new_blocks; */
/*         for (int i = starting_block; i < no_blocks_to_free; i++){ */
/*             fs_clearmaskbit(oft[fd].in.blocks[starting_block]); */
/*         } */
/*     } */

/*     starting_block = oft[fd].fileptr / MDEV_BLOCK_SIZE; */
/*     int offset = oft[fd].fileptr % MDEV_BLOCK_SIZE; */
/*     int bytes_written = 0; */
/*     int written_size = 0; */
    
/*     // write to file */
/*     while (nbytes > 0){ */
/*         if (starting_block == INODEBLOCKS) */
/*             break; */
/*         bs_bread(dev0, oft[fd].in.blocks[starting_block], 0, block_cache, fsd.blocksz); */
/*         if (MDEV_BLOCK_SIZE - offset > nbytes) */
/*             written_size = nbytes; */
/*         else */
/*             written_size = MDEV_BLOCK_SIZE - offset; */
/*         memcpy(block_cache + offset, buf + bytes_written, written_size); */
/*         bs_bwrite(dev0, oft[fd].in.blocks[starting_block], 0, block_cache, fsd.blocksz); */

/*         bytes_written += written_size; */
/*         starting_block++; */
/*         offset = 0; */
/*         nbytes -= written_size; */
/*     } */

/*     // update fileptr for next use */
/*     oft[fd].fileptr += bytes_written; */
/*     oft[fd].in.size = oft[fd].fileptr; */
    
/*     // write back inode */
/*     /1* _fs_put_inode_by_num(dev0, oft[fd].in.id, &oft[fd].in); *1/ */
/*     return bytes_written; */
/* } */

int fs_link(char *src_filename, char* dst_filename) {
  return SYSERR;
}

int fs_unlink(char *filename) {
  return SYSERR;
}

#endif /* FS */
