//
//  superBlock.cpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#include "superBlock.hpp"
#include "inode.hpp"


void write_superBlock()
{
    lseek(fd, 1 * block_size, SEEK_SET);
    write(fd, &isize, sizeof(isize));
    write(fd, &fsize, sizeof(fsize));
    write(fd, &nfree, sizeof(nfree));
    write(fd, &ninode, sizeof(ninode));
    write(fd, &avail_blocks, sizeof(avail_blocks));
    write(fd, free_blocks, sizeof(free_blocks));
    write(fd, free_inodes, sizeof(free_inodes));
}

void read_superBlock()
{
    lseek(fd, 1 * block_size, SEEK_SET);
    read(fd, &isize, sizeof(isize));
    read(fd, &fsize, sizeof(fsize));
    read(fd, &nfree, sizeof(nfree));
    read(fd, &ninode, sizeof(ninode));
    read(fd, &avail_blocks, sizeof(avail_blocks));
    read(fd, free_blocks, sizeof(free_blocks));
    read(fd, free_inodes, sizeof(free_inodes));
}

int get_offset(int block,int offset=0)
{
    return block * block_size + offset;
}

void fill_free_list(int block_num)
{
    lseek(fd, get_offset(block_num), SEEK_SET);
    read(fd, &nfree, sizeof(nfree));
    
    for(int i = 0; i < 100; i++)
    {
        read(fd, &buf, sizeof(buf));
        free_blocks[i] = buf;
    }
}

int alloc_block()
{
    nfree--;
    if(nfree > 0)
    {
        avail_blocks--;
        return free_blocks[nfree];
    }
    
    //refill list
    unsigned int new_block = free_blocks[0];
    fill_free_list(new_block);
    avail_blocks -- ;
    return new_block;
}


void fill_inode_list()
{
    lseek(fd, 2 * block_size, SEEK_SET);
    int counter = 0;
    for(int i = 0; i < (block_size / sizeof(inode)) * isize; i ++) // 16, 2048 (block size) / 128 (inode size)
    {
        if(counter >= 100)
        {
            break;
        }
        read(fd, &current, sizeof(current));
        if(!flag_isSet(current.flags, ALLOC))
        {
            free_inodes[counter] = i + 1; // since root inode starts at 1
            counter++; 
        }
    }
    ninode = counter;
}

int get_free_inode()
{
    if(ninode > 0)
    {
        ninode--;
        return free_inodes[ninode];
    }
    
    // List is empty, so...
    fill_inode_list();
    if(ninode > 0)
    {
        return get_free_inode();
    }
    
    // No inodes left!
    return -1;
}









