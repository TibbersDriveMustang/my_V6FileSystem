//
//  directory.hpp
//  my_Version
//
//  Created by Hongyi Guo on 12/3/15.
//  Copyright © 2015 Hongyi Guo. All rights reserved.
//

#ifndef directory_hpp
#define directory_hpp

#include <string>
using namespace std;

//16 bytes directory entry
struct dir_entry{
    
    unsigned short inode_num;
    char name[14];
};

//make new entry in given directory with given #inode and name
void add_dir_entry(int,int,string);


#endif /* directory_hpp */
