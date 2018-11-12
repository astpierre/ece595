# TOO MUCH MILK PROBLEM
## Description
> Two roommates take turns purchasing milk for their apartment. They check the refridgerator, and if there is no milk, they leave and purchase the milk. How can we avoid the problem of having too much milk?


## Considerations
1. __Locks:__ one process holds a lock at a time, does its critical section, releases lock
2. __Semaphores:__ more general version of a lock
3. __Monitors:__ connects shared data to syncronization primitives  
> _Note:_ all of these require some hardware support and some waiting


## Solution with locks
__ROOMMATE A__
```C
lock.acquire();
if (!milk)
{
	milk.buy;
}
lock.release();
```  
__ROOMMATE B__  
```C
lock.acquire();
if (!milk)
{
	milk.buy;
}
lock.release;
```

## Implementing locks
__Acquire__  
```C
void aquire(lock)
{
	while (lock == 0); // do nothing if taken
	lock --;
}
```
__Release__  
```C
void release(lock)
{
	if (lock == 0)
	{
		lock ++; 
	}
}
```
