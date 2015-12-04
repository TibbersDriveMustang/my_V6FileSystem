//
//  superBlock.hpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#ifndef superBlock_hpp
#define superBlock_hpp

#include <unistd.h>


extern int fd;
unsigned short block_size = 2048;
unsigned short avail_blocks;
unsigned int free_blocks[100];
unsigned int free_inodes[100];

unsigned int isize , fsize, nfree, ninode, buf;


void write_superBlock();

void read_superBlock();

int get_offset(int,int);

void fill_free_list(int);

void fill_free_list(int);


//get free #block from free list of super block
int alloc_block();

//Search and add free inodes to inode list(<=100)
void fill_inode_list();

//Get a free inode number from inode list
//return -1 if none
int get_free_inode();






#endif /* superBlock_hpp */
