# [ECE595 Programming Project 4](https://engineering.purdue.edu/~ece595/labs_2018/lab4.html)

### Team members: Andrew St. Pierre
### Due date: Sunday, December 2, 2018

## Testing tool  
I wrote a small shell script to aid in testing the lab. Usage: ```./testLab4 <Q1,Q234,Q56,Q7>```

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
2.  Run the test script
```
$ ./testLab4.sh Q1
```  

## Q2. Implement a DFS file system driver in DLXOS     
### Relevent files modified:  
* ```/ece595/lab4/flat/include/os/dfs.h```  
* ```/ece595/lab4/flat/os/dfs.c```  

## Q3. Implement inode-based functions in the DFS driver     
### Relevent files modified:  
* ```/ece595/lab4/flat/include/os/dfs.h```  
* ```/ece595/lab4/flat/os/dfs.c```  

## Q4. Write test code in ostests.c to test the full file system driver  
### Relevent files modified:  
* ```/ece595/lab4/flat/include/os/ostests.c```  
### Steps to test solution
1.  Enter the ```lab4``` directory  
```
$ cd ~/ece595/lab4  
```  
2.  Run the test script
```
$ ./testLab4.sh Q234
```  

## Q5. Implement a file-based API  
### Relevent files modified:  
* ```/ece595/lab4/flat/include/os/files.h```  
* ```/ece595/lab4/flat/os/files.c```  

## Q6. Write a user application to test your file system API    
### Relevent files modified:  
* ```/ece595/lab4/flat/apps/dfsAPItest/dfsAPItest/dfsAPItest.c```  
* ```/ece595/lab4/flat/apps/dfsAPItest/include/dfsAPItest.h```  
### Steps to test solution
1.  Enter the ```lab4``` directory  
```
$ cd ~/ece595/lab4  
```  
2.  Execute the bash script  
```
$ ./testLab4.sh Q56 
```

## Q7. Write a disk buffer cache in your DFS file system driver    


## References  
1. DLX Instruction Set  
2. DLX Architecture  
3. The DLX Operating System (DLXOS) Design  
4. MIT Open Courseware OS Course
5. IITD Open Courseware OS Course
6. xv6-public OS distribution architecture

# END-OF-README
