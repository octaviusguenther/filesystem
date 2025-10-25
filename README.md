# Filesystem
This repository contains a program which mimics a filesystem based on inodes, where you can create folders and files, remove them and write bytes into files. You can also import text and binary files from your windows subsystem and export the files again.  


## Requirements
- WSL: Ubuntu-22.04

## Installation
- copy the repository
- run "make"

## Tutorial
- create a new filesystem called "newfilesystem" containing 20 inodes by running
"./build/ha2 -c newfilesystem 20". After the filesystem has been created a console emerges with prompt "user@SPR".
- to create a folder called "documents" in the root directory, run the command "mkdir /documents"
- to create a file called "file" in the root directory, run the command "mkfile /file"
- to show all folders and files in the root directory, run the command "list /"
- to do above actions in a another folder, you always have to pass the absolute path within the filesystem as an argument, e.g. "mkfile /documents/text.txt"
- to write in an existing enmpty file, run the command "writef /file this is a text"
- to read a file, run the command "readf /file"
- to import a file, for example a .jpg file, first, create an empty file with "mkfile /documents/new.jpg" and then run the command
"import /documents/new.jpg /home/documents/external.jpg". The second argument is always the absolute path to your target file.
- to export a file, create an empty file in your Ubuntu Subsystem called "copy.jpg"  and run the command "export /documents/new.jpg /home/user/documents/copy.jpg".
- to save the the filesystem, run the command "dump".
- to load an existing filesystem , run "./build/ha2 -l newfilesystem"


## Copyright and Attribution

All files in this repository, except for `./src/operations.c`, were provided by the team of the course  
**Systemprogrammierung SS23** at the Technical University of Berlin and are included here for reference only.

The file `./src/operations.c` was fully developed by me, Octavius Guenther,  
and is © 2023 Octavius Guenther. All rights reserved.

This repository is published for demonstration and portfolio purposes only.  
Redistribution or reuse of the code without explicit permission is prohibited.



