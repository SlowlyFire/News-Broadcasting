# News Broadcasting  

**The overall scenario we are simulating is that of news broadcasting.  
Different types of stories are produced and the system sorts them and displays them to the public.  
In this project, the ‘new stories’ are simulated by simple strings which should be displayed to the screen in the order they arrive.**  

## The System Design:    
<img src="https://user-images.githubusercontent.com/83518959/193197203-ef6eb02a-164e-446e-9cb8-13ed389d1783.png" width="600" height="450" />  

Three producers communicate with the dispatcher via their Producer queues. The Dispatcher communicates with the Co-Editors via three queues corresponding to the three types of messages. The Co- Editors communicate with the Screen-Manager via a single shared queue, and the Screen manager displays the systems output.  

## There are 4 types of active actors, according to the chart above:   
  
## - Producer:   
Each producer creates a number of strings in the following format:  
“Producer < i >  < TYPE >  < j >”  
where ‘i‘ is the producers ID, ‘TYPE’ is a random type it chooses which can be ‘SPORTS’, ‘NEWS’, WEATHER’, and ‘j’ is the number of strings of type ‘TYPE’ this producer has already produced.  
The number of such products that a producer produces is received via its constructor.  

For example if producer 2 should create 3 strings a possible outcome of its products would be:  
**Producer 2 SPORTS 0  
Producer 2 SPORTS 1  
Producer 2 WEATHER 0**  

**Each of the producers passes its information to the Dispatcher (introduced below) via its own private queue**. Each of the Producers private queue is shared between the Producer and the Dispatcher. Each of the string products is inserted by the Producer to its ‘producers queue’. **After inserting all the products, the Producer sends a ‘DONE’ string through its Producers queue.**  
 
## - Dispatcher:  
**The Dispatcher continuously accepts messages from the Producers queues.** It scans the Producers queue using a Round Robin algorithm.  
*The Dispatcher does not block when the queues are empty.*  
**Each message is "sorted" by the Dispatcher and inserted to a one of the Dispatcher queues** which includes strings of a single type.  
When the Dispatcher receives a "DONE" message from all Producers, it sends a "DONE" message through each of its queues.  

For example:  
![image](https://user-images.githubusercontent.com/83518959/193199294-753d21ad-6fea-45f8-b41b-0bb9ccd348e8.png)  

## - Co-Editors:  
**For each type of possible messages there is a Co-Editor that receives the message through the Dispatchers queue, "edits" it, and passes it to the screen manager via a single shared queue.**  
The editing process will be simulated by the Co-Editors by blocking for one tenth (0.1) of a second. When a Co-Editor receives a "DONE" message, it passes it without waiting through the shared queue.  

## - Screen-manager:  
**The Screen-manager displays the strings it receives via the Co-Editors queue to the screen (std-output).** After printing all messages to the screen and receiving three "DONE" messages, the Screen manager displays a ‘DONE’ statement.  

----------------------------------------------------------------------------------------------------------------

- **Bounded and Unbounded Buffers:**   
We should notice that the Producer queues in this project and the Co-Editors shared queue are a **bounded buffer** that supports the following operations:  
· Bounded_Buffer (int size) – (constructor) create a new bounded buffer with size places to store objects.  
· void insert (char * s) – insert a new object into the bounded buffer.   
· char * remove ( ) - Remove the first object from the bounded buffer and return it to the user.  

  The dispatcher queues are unbounded buffers.  

  
  
- **Example of the configuration file:**  
![image](https://user-images.githubusercontent.com/83518959/193200846-41e4ea92-c7fc-4aa2-bb42-3c148e783be4.png)  
That means that for producer 1, we have 30 news, and its private queue size is 5.  
As well, for producer 2 we have 25 news, and its private queue size is 3, etc.  
17 is the Co-Editor queue size (which is shared with the screen).  
  
  
- **How to run?**  
g++ -o ex3.out ex3.cpp  
ex3.out config.txt  
