//
//  inode.cpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#include "inode.hpp"
#include "directory.hpp"
#include "superBlock.hpp"
#include <unistd.h>

extern int fd;

int get_offset(int block,int offset = 0)
{
    return block * block_size;
}

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

void init_inode(int dir,inode_flag type)
{
    current = get_inode(dir);
    set_flag(current.flags, ALLOC);
    set_flag(current.flags, type);
    write_inode(dir, current);
}

void read_contents(int inum)        //
{
    current = get_inode(inum);
    dir_entry temp_entry;
    int index = 0;
    
    while(index < current.size / sizeof(dir_entry))
    {
        if(index % (block_size / sizeof(dir_entry)) == 0)
        {
            lseek(fd, get_offset(current.addr[index / (block_size / sizeof(dir_entry))]), SEEK_SET);
        }
        read(fd, &temp_entry, sizeof(temp_entry));
        printf("%-8d %s \n", temp_entry.inode_num, temp_entry.name);
        index++;
    }
    
}

void copy_to_inode(int inum,int size)
{
    current = get_inode(inum);
    char buffer[block_size];
    int nread, position;
    current.size = size;
    position = lseek(fd2, 0, SEEK_SET);
    
    // Regular file
    if(flag_isSet(current.flags, PLAIN))
    {
        while((nread=read(fd2, buffer, sizeof(buffer))) > 0)
        {
            current.addr[position / block_size] = alloc_block();
            lseek(fd, get_offset(current.addr[position / block_size]), SEEK_SET);
            write(fd, buffer, sizeof(buffer));
            position+=nread;
        }
    }
    
    // Large file
    else if(flag_isSet(current.flags, LARGE))
    {
        int counter;
        unsigned int new_block;
        // start of indirect blocks loop
        for(int i = 0; i < size / (block_size * block_size / sizeof(new_block))+1; i++)
        {
            counter = 0;
            current.addr[i] = alloc_block();
            // fills each indirect block with the address of the data block
            // then fills the data block with the data
            while(counter < (block_size / sizeof(new_block)) && (nread=read(fd2, buffer, sizeof(buffer))) > 0)
            {
                new_block = alloc_block();
                lseek(fd, get_offset(current.addr[i], sizeof(new_block)*counter), SEEK_SET);
                write(fd, &new_block, sizeof(new_block));
                lseek(fd, get_offset(new_block), SEEK_SET);
                write(fd, buffer, sizeof(buffer));
                counter++;
            }
        }
        
    }
    write_inode(inum, current);
}

void copy_from_inode(int inum)      //
{
    current = get_inode(inum);
    char buffer[block_size];
    int nread;
    unsigned int num_blocks = current.size / block_size + 1;
    lseek(fd2, 0, SEEK_SET);
    
    // Regular file
    if(flag_isSet(current.flags, PLAIN))
    {
        for(int i = 0; i < num_blocks; i++)
        {
            lseek(fd, get_offset(current.addr[i]), SEEK_SET);
            nread = read(fd, buffer, (i < num_blocks - 1) ? sizeof(buffer) : current.size % block_size); // ternary operator to check if it's the last block, in which case it might not have the full block size of data
            write(fd2, buffer, nread);
        }
    }
    
    // Large file
    else if(flag_isSet(current.flags, LARGE))
    {
        unsigned int counter, counter2, second_block, last_block;
        last_block = current.size / block_size;
        counter2 = 0;
        // start of indirect block for loop
        for(int i = 0; i < current.size / (block_size * block_size / sizeof(second_block))+1; i++)
        {
            counter = 0;
            while(counter < (block_size / sizeof(second_block)))
            {
                // gets address of data block
                lseek(fd, get_offset(current.addr[i], sizeof(second_block)*counter), SEEK_SET);
                read(fd, &second_block, sizeof(second_block));
                // goes to data block to read data
                lseek(fd, get_offset(second_block), SEEK_SET);
                counter2++;
                // on last block might not have full block size of data
                if(counter2 > last_block)
                {
                    nread = read(fd, buffer, current.size % block_size);
                    write(fd2, buffer, nread);
                    break;
                }
                nread = read(fd, buffer, sizeof(buffer));
                write(fd2, buffer, nread);
                counter++;
            }
        }
    }
}

int get_inode_number(string name)                   //
{
    current = get_inode(current_inode);
    dir_entry temp_entry;
    
    for(int i = 0; i < current.size / 16; i++)
    {
        if(i % sizeof(inode) == 0)
        {
            lseek(fd, get_offset(current.addr[i / sizeof(inode)]), SEEK_SET);
        }
        read(fd, &temp_entry, sizeof(temp_entry));
        if(strcmp(name.c_str(), temp_entry.name) == 0)
        {
            return temp_entry.inode_num;
        }
    }
    return -1;
}

bool inode_is_dir(int inum)
{
    current = get_inode(inum);
    return flag_isSet(current.flags, DIR);
}



