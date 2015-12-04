//
//  inode.hpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#ifndef inode_hpp
#define inode_hpp



using namespace std;

#include <string>

struct inode{
    /*
        flag bits:
            0 : allocated if 1
           1-2:   11(plain file)   10(directory)
            3 : large file if 1
     */
    unsigned short flags;
    char nlinks;        //number of links to file
    char uid;           //user id of owner
    char gid;           //group id of owner
    unsigned int size;  //size of bytes
    unsigned int addr[27];  //addresses of direct or indirect data blocks
    unsigned int actime;    //time of last access
    unsigned int modtime;   //time of last modification
}initial,current;

inode get_inode(int);

void write_inode(int,inode);

enum inode_flag{
    
    ALLOC,      //is allocated
    PLAIN,      //is regular file
    DIR,        //is directory
    LARGE       //is large file(> 55 KB)
};


void set_flag(unsigned short&,inode_flag);

int flag_isSet(unsigned short,inode_flag);

//write new inode into given directory
//type:PLAIN,LARGE,DIR
void init_inode(int,inode_flag);

//print the contents of directory
void read_contents(int);

//copy external file into internal file
void copy_to_inode(int,int);

//copy internal file to external file
void copy_from_inode(int);


//get #inode correspond to the name
//-1 if not found
int get_inode_number(string);

//check if inode is directory
bool inode_is_dir(int);





#endif /* inode_hpp */
