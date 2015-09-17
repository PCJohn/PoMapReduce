PoMapReduce
===========

Po is a simple MapReduce implementation in C. The DL library is used to execute user defined Map(), Reduce() methods. A concurrent server distributes tasks to worker nodes.
It uses a custom protocol for the master-worker communication. Protocol.h provides users with the ability to use functions with these protocols in their programs (command all workers- brodacstCmd(), check if command- checkCmd(), sendFile(), etc.).
The results returned from the workers are compiled as a single result file. The user may implement his/her own Combine() function to override this behviour.
A sample user program can be viewed in temp.c (Shows how to implement the Map(), Reduce() and Combine() methods).
