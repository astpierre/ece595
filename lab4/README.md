# [ECE595 Programming Project 4](https://engineering.purdue.edu/~ece595/labs_2018/lab4.html)

### Team members: Andrew St. Pierre
### Due date: Sunday, December 2, 2018

## Q1. Implement a user program named "fdisk" to format your disk   
### Relevent files modified:  
* ```/ece595/lab4/flat/include/dfs_shared.h```  
* ```/ece595/lab4/flat/include/files_shared.h```  
* ```/ece595/lab4/flat/include/os/disk.h```  
* ```/ece595/lab4/flat/apps/fdisk/include/fdisk.h```  
* ```/ece595/lab4/flat/apps/fdisk/fdisk/fdisk.c```  

### Steps to test solution
1.  Enter the ```lab4``` directory  
```
$ cd ~/ece595/lab4  
```  
2.  Build the OS 
```
$ cd /flat/os  
$ make  
```  
3.  Build the user program ```fdisk```  
```
$ cd ~/ece595/lab4  
$ cd /flat/apps/fdisk 
$ make
```
4.  Run user program ```fdisk```  
```
$ make run
```

## Q2. Implement a DFS file system driver in DLXOS     
### Relevent files modified:  
* ```/ece595/lab4/flat/include/os/dfs.h```  
* ```/ece595/lab4/flat/os/dfs.c```  


## References  
1. DLX Instruction Set  
2. DLX Architecture  
3. The DLX Operating System (DLXOS) Design  

# END-OF-README
