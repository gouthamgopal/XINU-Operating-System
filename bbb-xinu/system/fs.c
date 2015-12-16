#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#if FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

/**************************************
** Create a File
**************************************/
int fs_create(char *filename, int fopen_mode) {
	if (fopen_mode == O_CREAT) {
		int i;
		int opendirent = -1;

		// Check if file exists		
		for(i=0; i<DIRECTORY_SIZE; i++){
			if(fs_compare(filename, fsd.root_dir.entry[i].name) ) {
				printf("File with this name already exists!");
				return SYSERR;
			}
			if(fsd.root_dir.entry[i].inode_num == -1)
				opendirent = i;
		}

		// There are no more files available
		if(opendirent == -1) {
			return SYSERR;
		}	
	
		// Get a free node
		int i_node, k; 
		for(k=0; k<fsd.ninodes; k++) {
			struct inode free;
			fs_get_inode_by_num(0, k, &free);
			if(free.type == -1) {
				i_node = free.id;
				break;
			}
		}

		// Fill in new file details
		struct inode freeNode;
		fs_get_inode_by_num(0, i_node, &freeNode);
		freeNode.size=0;
		freeNode.type=1;
		freeNode.nlink=0;
	
		fsd.root_dir.numentries++;

		fs_copy(fsd.root_dir.entry[opendirent].name, filename);
		fsd.root_dir.entry[opendirent].inode_num = freeNode.id;
	
	
		fs_put_inode_by_num(0, freeNode.id, &freeNode);
	
		return OK;
	} //end-if
}


/**************************************
** Open a File
**************************************/
int fs_open(char *filename, int flags) {
	int i;
	int current_inode_number = -1;

	// search for the inode
	for(i=0; i<DIRECTORY_SIZE; i++){
		if(fs_compare(filename, fsd.root_dir.entry[i].name)) {
			current_inode_number = fsd.root_dir.entry[i].inode_num;
			break; 
		}
	}
	
	// validate the inode information
	if(current_inode_number == -1) {
		printf("File %s does not exist", filename);
		return SYSERR;
	}
	
	struct inode new_node;
	fs_get_inode_by_num(0, current_inode_number, &new_node);
	
	
	// Allocates file descriptor
	int fd = get_file_desc();
	if(fd == -1) {
		return SYSERR;
	}
	
	// Update the filetable
	oft[fd].state = 1;
	oft[fd].fileptr = 0;	
	oft[fd].in = new_node;
	oft[fd].in.nlink++;

	// Flag to append to file
	if(flags == 1) {
		oft[fd].fileptr = oft[fd].in.size;
	}
	
	// Update the inode
	fs_put_inode_by_num(0, current_inode_number, &oft[fd].in);
	return fd;
}

/**************************************
** Close a File
**************************************/
int fs_close(int fd) {
	if(oft[fd].state == -1) {
		printf("Bad file descriptor \n");
		return SYSERR;
	}

	//Update inode and save to memory
	struct inode update;
	fs_get_inode_by_num(0, oft[fd].in.id, &update);
	update.nlink--;
	fs_put_inode_by_num(0,oft[fd].in.id,&update);
	

	oft[fd].state = -1;
	oft[fd].fileptr = 0;
	
	return OK;
}

/**************************************
** Read file contents
**************************************/
int fs_read(int fd, void *buf, int nbytes) {
	
	fs_get_inode_by_num(0, oft[fd].in.id, &oft[fd].in);
	
	// Check nbytes is less than file size	
	if(oft[fd].in.size < nbytes + oft[fd].fileptr) {
		return -1;
	}
	
	char* buff = buf;
	buff[0]='\0';
	int bytesread = 0;
	
	while(bytesread < nbytes) {
		int curBlock = oft[fd].fileptr/fsd.blocksz;
		
		// Check if out of bounds
		int bytesOnCurBlock = fsd.blocksz - (oft[fd].fileptr%fsd.blocksz);
		
		if(bytesOnCurBlock < nbytes - bytesread) {
			char r[fsd.blocksz];
			int btor = fsd.blocksz - oft[fd].fileptr%fsd.blocksz;
			bs_bread(0,oft[fd].in.blocks[curBlock], oft[fd].fileptr%fsd.blocksz, &r, btor);
			oft[fd].fileptr += btor;
			fs_show(buf,r,btor);
			bytesread += btor;
		}
		// Read current block
		else {
			char r[fsd.blocksz];
			bs_bread(0,oft[fd].in.blocks[curBlock],oft[fd].fileptr%fsd.blocksz,&r,nbytes - bytesread);
			oft[fd].fileptr += nbytes - bytesread;
			fs_show(buf,r,nbytes-bytesread);
			bytesread += nbytes - bytesread;
		}	
	}
	
	fs_put_inode_by_num(0,oft[fd].in.id,&oft[fd].in);
	return OK;
}

/**************************************
** Write to a file
**************************************/
int fs_write(int fd, void *buf, int nbytes) {
	
	fs_get_inode_by_num(0,oft[fd].in.id,&oft[fd].in);
	
	int block_num=(oft[fd].fileptr+nbytes)/fsd.blocksz+1;
	int cur_block_num = oft[fd].in.size/fsd.blocksz;
	
	if(oft[fd].in.size%fsd.blocksz != 0) {
		cur_block_num++;
	}

	int alloc_num;
	int i;
	
	if((alloc_num = block_num - cur_block_num) > 0){
		for(i = 0; i < alloc_num; i++){
			if(alloc_block(fd, cur_block_num) != -1) {
				cur_block_num++;
			}
			else {
				fs_put_inode_by_num(0, oft[fd].in.id, &oft[fd].in);
				return SYSERR;
			}
		}
	}
	
    
	int byteswrite = 0;
	while(byteswrite < nbytes) {
		int curBlock = oft[fd].fileptr/fsd.blocksz;
		int offset = oft[fd].fileptr % fsd.blocksz;
		
		// Check if out of bounds
		int bytesOnCurBlock = fsd.blocksz - (oft[fd].fileptr%fsd.blocksz);
		
		if(bytesOnCurBlock < nbytes - byteswrite) {
			char* buff = ((char*) buf);
			bs_bwrite(0,oft[fd].in.blocks[curBlock], offset, &buff[byteswrite], bytesOnCurBlock);
			oft[fd].fileptr += bytesOnCurBlock;
			byteswrite += bytesOnCurBlock;
			if(oft[fd].fileptr > oft[fd].in.size) {
				oft[fd].in.size = oft[fd].fileptr;
			}
		}
		// Read on current block
		else {
			char* buff = ((char*) buf);
			bs_bwrite(0,oft[fd].in.blocks[curBlock], offset, &buff[byteswrite], nbytes - byteswrite);
			oft[fd].fileptr += nbytes - byteswrite;
			byteswrite += nbytes - byteswrite;
			
			if(oft[fd].fileptr > oft[fd].in.size) {
				oft[fd].in.size = oft[fd].fileptr;
			}
		
		}	
	}

	fs_put_inode_by_num(0,oft[fd].in.id,&oft[fd].in);
	return byteswrite;
}

/**************************************
** Reposition the file pointer
**************************************/
int fs_seek(int fd, int offset) {
	// Get the new file pointer position
	int fileptr = oft[fd].fileptr + offset;

	// Check if it goes beyond the size
	if(fileptr > oft[fd].in.size) {
		return SYSERR;
	}

	// Update the file pointer
	if(fileptr/fsd.blocksz <= INODEBLOCKS) {
		oft[fd].fileptr = fileptr;
	}

	return OK;
}

/**************************************
** Compare two strings
**************************************/
int fs_compare(char* a, char* b) {
	int index = 0;
	while(a[index] == b[index] && a[index] != '\0' && b[index] != '\0') {
		index++;
	}
	if(a[index] == '\0' && b[index] == '\0') {
		return 1;
	}
	return 0;
}

/**************************************
** Copy string contents
**************************************/
int fs_copy(char* a, char* b) {
	int i = 0;
	while(b[i]!='\0' && i<FILENAMELEN-1) {
		a[i]=b[i];
		i++;
	}
	a[i]='\0';
	return 0;
}

/**************************************
** Get the file descriptor
**************************************/
int get_file_desc() {
	int i;
	for(i = 0;i<DIRECTORY_SIZE*2;i++) {
		if(oft[i].state == -1) {
			return i;
		}
	}
	printf("No File Descriptor Free\n");
	return -1;
}

int fs_show(char* a, char* b, int length) {
	int len = 0;
	while(a[len]!='\0')
		len++;
		
	int i = 0;
	while(i<length) {
		a[len++]=b[i++];
	}
	a[len]='\0';
	return 0;
}

int alloc_block(int fd, int num){
	int i;
	for(i = 0; i < fsd.nblocks; i++){
		if(fs_getmaskbit(i) == 0){
			oft[fd].in.blocks[num] = i;
			fs_setmaskbit(i);
			return 0;
		} 
	} 
	return -1; 
}

/* MY CODE ENDS */

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int
fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    //printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int
fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    //printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

void
fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}

#endif /* FS */