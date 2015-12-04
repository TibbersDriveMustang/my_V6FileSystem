//
//  directory.cpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#include "directory.hpp"
#include <unistd.h>
#include "superBlock.hpp"
#include "inode.hpp"

extern int fd;


void add_dir_entry(int dir, int inode_num, string name)
{
    dir_entry entry;
    entry.inode_num = inode_num;
    strcpy(entry.name, name.c_str());
    
    lseek(fd, 2 * block_size + (dir - 1) * sizeof(inode), SEEK_SET);
    read(fd, &current, sizeof(current));
    if(current.size % block_size == 0)
    {
        current.addr[current.size / block_size] = alloc_block();
    }
    lseek(fd, get_offset(current.addr[current.size / block_size], current.size % block_size), SEEK_SET);
    write(fd, &entry, sizeof(entry));
    
    current.size += sizeof(entry);
    lseek(fd, 2 * block_size + (dir - 1) * sizeof(inode), SEEK_SET);
    write(fd, &current, sizeof(current));
}


