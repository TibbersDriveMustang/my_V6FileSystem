//
//  main.cpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//
//#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cmath>
#include <cstring>
#include <stdlib.h>

#include "superBlock.hpp"
#include "inode.hpp"
#include "directory.hpp"
using namespace std;


unsigned int temp;

int get_offset(int block,int offset = 0)
{
    return block * block_size;
}

int main()
{
    // Main loop for reading commands
    cout << "Type 'help' to see list of available commands" << endl;
    string command = "";
    bool fs_active = false; // checks if there is a file system active
    string first, filename, dirname;
    
    stringstream ss;
    do
    {
        cout << "> ";
        getline(cin, command);
        ss.str(command);
        ss >> first;
        if(first == "initfs")
        {
            if(fs_active)
            {
                cout << "There is already an active file system. Please quit and start again." << endl;
                ss.clear();
                continue;
            }
            ss >> filename >> fsize >> isize;
            if(filename == "" || fsize < 6 || isize < 1 || fsize < isize)
            {
                cout << "Please input a filename for the disk, a number of disk blocks (6+), and a number of inode blocks (1+)." << endl;
            }
            else
            {
                fd = open(filename.c_str(), O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
                
                // Set length of file
                ftruncate(fd, (off_t)fsize * block_size);
                
                // Initialize free list
                temp = (fsize - isize - 3) / 100;
                for(int i = 0; i < temp; i++)
                {
                    lseek(fd, (2 + isize + 100*(i+1)) * block_size, SEEK_SET);
                    buf = 100;
                    write(fd, &buf, sizeof(buf));
                    for(int j = 0; j < 100; j++)
                    {
                        buf = 2 + isize + 100 * i + j;
                        write(fd, &buf, sizeof(buf));
                    }
                }
                nfree = (fsize - isize - 2) % 100;
                temp *= 100;
                temp += (2 + isize);
                for(int j = 0; j < nfree; j++)
                {
                    free_blocks[j] = temp + j;
                }
                avail_blocks = fsize - isize - 2;
                
                // Initialize root directory
                inode initial = {0};
                set_flag(initial.flags, ALLOC);
                set_flag(initial.flags, DIR);
                current_inode = 1;
                
                lseek(fd, 2 * block_size, SEEK_SET);
                write(fd, &initial, sizeof(initial));
                
                add_dir_entry(1, 1, ".");
                add_dir_entry(1, 1, "..");
                
                // Find free inodes and add to inode list
                fill_inode_list();
                fs_active = true;
                
            }
        }
        else if(first == "cpin") // Copy an external file into the filesystem
        {
            if(fs_active)
            {
                string external_file, v6_file;
                ss >> external_file >> v6_file;
                if(v6_file == "" || external_file == "")
                {
                    cout << "Please input a source file and a destination file." << endl;
                }
                else
                {
                    fd2 = open(external_file.c_str(), O_RDONLY);
                    if(fd2 == -1)
                    {
                        cout << "The source file couldn't be opened for reading." << endl;
                    }
                    else
                    {
                        // Get filesize
                        struct stat stats;
                        fstat(fd2, &stats);
                        int size = stats.st_size;
                        /*
                         Max size:
                         27 addr blocks * 512 indirect blocks
                         - 1 indirect block for every 512 data blocks
                         - 1 possible block to extend the directory listing
                         */
                        if(size > (27 * block_size / 4 * block_size) / (1 + 1 / (block_size / 4)) - block_size)
                        {
                            cout << "The file is too large for the filesystem, max is " << 27 * (block_size / 4) * block_size * (1 - 1 / (block_size / 4)) - block_size << " bytes." << endl;
                        }
                        else if(size > (avail_blocks * block_size) / (1 + 1 / (block_size / 4)) - block_size)
                        {
                            cout << "The file is too large, we need " << ceil(size / block_size) + (size / block_size) / (block_size / 4) + 1 << " free blocks to store it but we only have " << avail_blocks / (1 + 1 / (block_size / 4)) - 1 <<  "." << endl;
                        }
                        else
                        {
                            int inum = get_free_inode();
                            if(inum == -1)
                            {
                                cout << "There are no more inodes available." << endl;
                            }
                            else
                            {
                                // Allocate and report size of file
                                init_inode(inum, (size > 27 * block_size) ? LARGE : PLAIN);
                                add_dir_entry(current_inode, inum, v6_file);
                                copy_to_inode(inum, size);
                            }
                        }
                    }
                    close(fd2);
                }
            }
            else
            {
                cout << "There is not a file system active right now. Please use 'initfs' or 'use' to start one." << endl;
            }
        }
        else if(first == "cpout") // Copy an internal file out of the filesystem
        {
            if(fs_active)
            {
                string external_file, v6_file;
                ss >> v6_file >> external_file;
                if(v6_file == "" || external_file == "")
                {
                    cout << "Please input a source file and a destination file." << endl;
                }
                else
                {
                    int inum = get_inode_number(v6_file);
                    if(inum == -1)
                    {
                        cout << "The source file is invalid." << endl;
                    }
                    else
                    {
                        fd2 = open(external_file.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
                        if(fd2 == -1)
                        {
                            cout << "The destination file couldn't be opened for writing." << endl;
                        }
                        else
                        {
                            copy_from_inode(inum);
                        }
                        close(fd2);
                    }
                }
            }
            else
            {
                cout << "There is not a file system active right now. Please use 'initfs' or 'use' to start one." << endl;
            }
        }
        else if(first == "ls") // List contents of the current directory
        {
            if(fs_active)
            {
                read_contents(current_inode);
            }
            else
            {
                cout << "There is not a file system active right now. Please use 'initfs' or 'use' to start one." << endl;
            }
        }
        else if(first == "mkdir") // Make a new directory in the root directory
        {
            if(fs_active)
            {
                ss >> dirname;
                if(dirname == "")
                {
                    cout << "Please input a name for the directory." << endl;
                }
                else if(avail_blocks - 1 == 0)
                {
                    cout << "We need at least one block to make a new directory." << endl;
                }
                else
                {
                    // Create new directory inode
                    int inum = get_free_inode();
                    init_inode(inum, DIR);
                    
                    // Add to current directory
                    add_dir_entry(current_inode, inum, dirname);
                    add_dir_entry(inum, inum, ".");
                    add_dir_entry(inum, current_inode, "..");
                }
            }
            else
            {
                cout << "There is not a file system active right now. Please use 'initfs' or 'use' to start one." << endl;
            }
        }
        else if(first == "cd") // Changes current directory
        {
            if(fs_active)
            {
                ss >> dirname;
                if(dirname != "")
                {
                    int inum = get_inode_number(dirname);
                    if(inum!= -1 && inode_is_dir(inum))
                    {
                        current_inode = inum;
                    }
                    else
                    {
                        cout << "Invalid directory" << endl;
                    }
                }
            }
            else
            {
                cout << "There is not a file system active right now. Please use 'initfs' or 'use' to start one." << endl;
            }
            
        }
        else if(first == "q") // Quit
        {
            if(fs_active)
            {
                write_superBlock();
                close(fd);
            }
            cout << "Goodbye." << endl;
        }
        else if(first == "use") // Open existing file system and use it
        {
            if(!fs_active)
            {
                ss >> filename;
                if(filename == "")
                {
                    cout << "Please specify a file name" << endl;
                }
                else
                {
                    fd = open(filename.c_str(), O_RDWR);
                    if(fd == -1)
                    {
                        cout << "The file could not be opened for reading" << endl;
                    }
                    else
                    {
                        read_superBlock();
                        current_inode = 1;
                        fs_active = true;
                    }
                }
            }
            else
            {
                cout << "There is already an active file system, please quit and start again" << endl;
            }
            
            
        }
        else if(first == "help") // Lists available commands
        {
            cout << endl << "Available commands: " << endl;
            cout << "initfs <file name of file system> <# of total blocks> <# of inode blocks>" << endl;
            cout << "cpin <external file> <v6 file>" << endl;
            cout << "cpout <v6 file> <external file>" << endl;
            cout << "mkdir <v6 dir>" << endl;
            cout << "use <file name of file system> (use existing file system file)" << endl;
            cout << "ls (lists current directory entries)" << endl;
            cout << "cd <v6 dir> (changes directories)" << endl;
            cout << "q" << endl << endl;
        }
        else
        {
            cout << "Sorry, I don't recognize that command." << endl;
        }
        
        ss.clear();
    }while(command != "q");
    
    return 0;
    
}