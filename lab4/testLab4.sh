#!/bin/bash

clear
echo Welcome to Lab 4! You have chosen to test $1
if [ "$1" == "Q1" ]
then
    echo Q1. Implement a user program named fdisk.c to format your disk
    read -p "Press enter to continue"
    echo Building the OS
    cd flat/os/
    make clean
    make
    echo Building the user program fdisk
    cd ../apps/dfsAPItest
    make clean
    make
    clear
    echo Running fdisk.c
    make run
fi
if [ "$1" == "Q234" ]
then
    echo Q2. Implement DFS file system driver in DLXOS
    echo Q3. Implement inode-based functions in the DFS driver
    echo Q4. Write test code in ostests.c to test the full filesystem driver
    read -p "Press enter to continue"
    echo Building the OS
    cd flat/os/
    make clean
    make
    echo Building the user program ostests.c that calls RunOSTests system call
    cd ../apps/ostests
    make clean
    make
    clear
    echo Running ostests.c
    make run
fi
if [ "$1" == "Q56" ]
then
    echo Q5. Implement a file-based API in DLXOS
    echo Q6. Write a user application to test your file API
    read -p "Press enter to continue"
    echo Building the OS
    cd flat/os/
    make clean
    make
    echo Building the user program dfsAPItest.c that tests the API
    cd ../apps/dfsAPItest
    make clean
    make
    clear
    echo Running dfsAPItest.c
    make run
fi
if [ "$1" == "clean" ]
then
    echo CLEANING ALL LAB4 DIRECTORIES FOR COMPILER GENERATED FILES
    read -p "Press enter to continue"
    cd flat/os/
    make clean
    cd ../apps/dfsAPItest
    make clean
    cd ../ostests
    make clean
    cd ../fdisk
    make clean
    clear
fi

echo DONE testing the DFS filesystem driver!
