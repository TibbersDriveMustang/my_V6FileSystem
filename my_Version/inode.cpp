//
//  inode.cpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#include "inode.hpp"
#include <unistd.h>

extern int fd;
extern short block_size;

inode get_inode(int inum){
    
    lseek(fd, 2 * block_size + (inum - 1) * sizeof(inode), SEEK_SET);
    read(fd, &current, sizeof(current));
    return current;
}


void write_inode(int inum,inode node)
{
    lseek(fd, 2 * block_size + (inum - 1) * sizeof(inode), SEEK_SET);
    write(fd, &node, sizeof(node));

}

void set_flag(unsigned short &flags,inode_flag i_flag)
{
    switch(i_flag)
    {
        case ALLOC: flags |= 0x8000;
            break;
        case PLAIN: flags |= 0x6000;
            break;
        case DIR: flags |= 0x4000;
            flags &= ~(0x2000);
            break;
        case LARGE: flags |= 0x1000;
            break;
    }
}

int flag_isSet(unsigned short flags, inode_flag i_flag)
{
    switch(i_flag)
    {
        case ALLOC: return (flags & 0x8000) == 0x8000;
            break;
        case PLAIN: return (flags & 0x6000) == 0x6000;
            break;
        case DIR: return ((flags & 0x4000) == 0x4000) && ((flags | ~(0x2000)) == ~(0x2000));
            break;
        case LARGE: return (flags & 0x1000) == 0x1000;
            break;
    }
}



