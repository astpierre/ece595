# [ECE595 Programming Project 3](https://engineering.purdue.edu/~ece595/labs_2018/lab3.html)

### Team members: Andrew St. Pierre
### Due date: Sunday, November 4, 2018

## Relevent files modified for each question
#### 1&2. Implement dynamic one-level paging in DLXOS and write test programs 
For this question, I modified the following files:  
* In the ```\os``` directory, ```memory.c```,```process.c```,```traps.c```  
* In the ```\include\os``` directory, ```memory.h```, ```memory_constants.h```, ```process.h```  
* In the ```\apps\example``` directory, the user program tests are found  
#### 3&4. Implement ```fork()``` with one-level page tables using copy-on-write and write a test program
For this question, I modified the following files:  
* In the ```\os``` directory, ```memory.c```,```process.c```,```traps.c```  
* In the ```\include\os``` directory, ```memory.h```, ```memory_constants.h```, ```process.h```  
* In the ```\apps\example``` directory, the user program tests are found  
#### 5. Implement ```malloc()``` and ```free()``` system calls to accomplish heap management  
For this question, I modified the following files:  
* In the ```\os``` directory, ```memory.c```,```process.c```,```traps.c```  
* In the ```\include\os``` directory, ```memory.h```, ```memory_constants.h```, ```process.h```  
* In the ```\apps\example``` directory, the user program tests are found  

## Getting started
1. Enter the ```lab3``` directory  
```
$ cd ~/ece595/lab3  
```  
2. First we will test one-level paging (parts 1 and 2 of the assignment)  
```
$ cd one-level
$ cd os
$ make  
$ cd ../apps/example  
$ make  
$ make run
``` 
3. Next we will test ```fork()``` implementation (parts 3 and 4 of the assignment)
```
$ cd ~/ece595/lab3/fork
$ cd os
$ make  
$ cd ../apps/example  
$ make  
$ make run  
```
4. Finally we can test ```malloc()``` and ```free()``` implementations (part 5 of the assignment)
```
$ cd ~/ece595/lab3/heap-mgmt
$ cd os
$ make  
$ cd ../apps/example  
$ make  
$ make run  
```  

## References  
1. DLX Instruction Set  
2. DLX Architecture  
3. The DLX Operating System (DLXOS) Design  

# END-OF-README
