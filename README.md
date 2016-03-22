------------------------------------------------------------------------------------------------------------------------------
FAILURE DETECTOR using GOSSIP-based HEARTHBEAT algorithm
------------------------------------------------------------------------------------------------------------------------------

Run instructions:
1. execute make before each run
2. ensure endfile is deleted before the run.
3. Usage ./p4 N b c F B P S T


------------------------------------------------------------------------------------------------------------------------------
Sample run outputs
------------------------------------------------------------------------------------------------------------------------------

RUN1.
------------------------------------------------------------------------------------------------------------------------------
When the parameter is 7 2 2 4 1 2 3 15:
Because the number of nodes should be failed is 1, so in those file listX, there should be one list file which has node status FAIL, the other list files
should has node status OK.

The list file output for each nodes:
Node 0  Node 1  Node 2  Node 3  Node 4  Node 5  Node 6
OK      OK      OK		FAIL	OK		OK		OK
0 14    0 15    0 13	0 1		0 14	0 13	0 13
1 15    1 14	1 13	1 1		1 14	1 13	1 14
2 14    2 13	2 15	2 1		2 13	2 13	2 14
3 2     3 2	    3 1		3 2		3 2		3 1		3 3
4 14    4 14	4 13	4 0		4 15	4 13	4 13
5 14    5 14	5 13	5 1		5 13	5 15	5 14
6 14    6 14 	6 13	6 1		6 13	6 13	6 15

Because number of seconds to wait between failures is 3, we can see that in those list files, the dead node has timeStamp less than 3.
So it is clear that the node 3 is dead in 3 seconds

Because gossip parameter b and gossip parameter c are both 2. So during each node`s running, we can see that each node will send 2 other nodes messages
and it will do 2 iterations. In each iteration, the heartbeat will increase 1.

Also, when each node receives messages from other nodes, it will update its list. We can see that during each node running, the list will be updated when
node receives messages from other nodes. Then it will send the updated list messages to its neighbors.

Because number of seconds after which a node is considered dead is 4, we can see that from node 3, it will receive message for 4 seconds when it is failed.
Then it will stop receiving message because it is dead.

RUN2
------------------------------------------------------------------------------------------------------------------------------
When the parameter is 8 3 2 3 2 3 3 20:
Because the number of nodes should be failed is 2, so in those file listX, there should be two list file which has node status FAIL, the other list files
should has node status OK.

The list file output for each nodes:
Node 0  Node 1  Node 2  Node 3  Node 4  Node 5  Node 6  Node 7
OK      FAIL    OK		OK	    OK		OK		OK		FAIL
0 20    0 2     0 18	0 19	0 19	0 19	0 18    0 5
1 3     1 3	    1 2	    1 3		1 2	    1 2	    1 2		1 3 
2 19    2 2	    2 20	2 19	2 18	2 19	2 18	2 5
3 19    3 2	    3 18	3 20	3 19	3 19	3 18	3 5 
4 18    4 2	    4 18	4 19	4 20	4 18	4 18	4 5
5 19    5 2	    5 18	5 19	5 19	5 20	5 18	5 5 
6 19    6 2 	6 18	6 19	6 18	6 19	6 20	6 5 
7 4		7 2		7 4		7 4		7 4		7 4		7 4		7 6


Because number of seconds to wait between failures is 3, we can see that in those list files, the first dead node(node 1) has timeStamp 
less than 3. The second dead node(node 7) has timeStamp less than 6.

Because gossip parameter b is 3 and gossip parameter c is 2. So during each node`s running, we can see that each node will send 3 other nodes messages
and it will do 2 iterations. In each iteration, the heartbeat will increase 1.

Also, when each node receives messages from other nodes, it will update its list. We can see that during each node running, the list will be updated when
node receives messages from other nodes. Then it will send the updated list messages to its neighbors.

Because number of seconds after which a node is considered dead is 3, we can see that from node 1, it will receive message for 3 seconds from
the time it is failed at 3 seconds. Then it will stop receiving message because it is dead. We also can see that from node 7, 
it will receive message for 3 seconds from the time it is failed at 6 second. Then it will stop receiving message because it is dead

RUN3
------------------------------------------------------------------------------------------------------------------------------
When the parameter is 6 2 2 4 3 3 3 30:
Because the number of nodes should be failed is 3, so in those file listX, there should be three list file which has node status FAIL, the other list files
should has node status OK.

The list file output for each nodes:
Node 0  Node 1  Node 2  Node 3  Node 4  Node 5  
OK      FAIL    FAIL	OK	    OK		FAIL			
0 30    0 3     0 8	    0 29	0 29	0 5	
1 3     1 3	    1 2	    1 2		1 2	    1 2	    
2 8     2 2	    2 9	    2 9 	2 8 	2 4	
3 28    3 2	    3 8	    3 30	3 29	3 5	
4 28    4 2	    4 8	    4 29	4 30	4 5	
5 4     5 2	    5 4	    5 4 	5 4	    5 6			


Because the number of seconds after which node is considered dead is 3, we can see that in those list files, the first dead node(node 1) has timeStamp 
less than 4. The second dead node(node 5) has timeStamp less than 6. The third dead node(node 2) has timeStamp less than 9.

Because gossip parameter b is 2 and gossip parameter c is 2. So during each node`s running, we can see that each node will send 2 other nodes messages
and it will do 2 iterations. In each iteration, the heartbeat will increase 1.

Also, when each node receives messages from other nodes, it will update its list. We can see that during each node running, the list will be updated when
node receives messages from other nodes. Then it will send the updated list messages to its neighbors.

Because number of seconds after which a node is considered dead is 4, we can see that from node 1, it will receive message for 4 seconds from
the time it is failed at 3 seconds. Then it will stop receiving message because it is dead. We also can see that from node 5, 
it will receive message for 4 seconds from the time it is failed at 6 second. Then it will stop receiving message because it is dead
We also can see that from node 2, it will receive message for 4 seconds from the time it is failed at 9 second. 
Then it will stop receiving message because it is dead


