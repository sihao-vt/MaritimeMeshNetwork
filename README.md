# Ocean Mesh Network Simulation

## SetUp
First, install ns3.
```
cd
mkdir workspace
cd workspace
wget http://www.nsnam.org/release/ns-allinone-3.24.tar.bz2
tar xjf ns-allinone-3.24.tar.bz2
```
[ns3 Tutorial-Getting Started](https://www.nsnam.org/docs/release/3.24/tutorial/html/getting-started.html#downloading-ns3) is the tutorial page of NS3.

Then, copy `ocean-3d-random-walk.h` and `ocean-3d-random-walk.cc` to ns3/src/mobility/model/. Also use `wscript` to replace ns3/src/mobility/wcsript, which is the registration file and will not affect other funcationalities.

Same for copying`ocean-propagation-model.h` and `ocean-propagation-model.cc` to ns3/src/propagation/model, and copying `wscript` to replace ns3/src/propagation/wsript.

`ocean.cc` is the main function. I perfer to put this file in ns3/scratch/ . Then run
```
./waf --run scratch/ocean
```
will show the result.


## Functionality
### Mobility Model

Vertically, we read height data from ocean data file. Horizontally, it is a 2d random walk model with some adjustment.

In 2d random walk, I need to set the bound for all nodes. All nodes share the same bound, which mean all nodes are walking in 
this scope. However, in my case, I want every node is fixed but moves a little bit around this fixed point. Therefore, I set 
different bounds for different nodes, based on their position. Therefore, each node can walk inside a small range.

**NOTE** : The upper, lower limit of this range and node position should always be positive.

### Propagation Model

We use a two-ray model, which is a common and widely-used propagation model.

Also, this propagation model can check if the ocean wave is blocked by ocean wave. If so, we assume that the communication 
link is corrupted.

### Link-state-aware routing protocol

This protocol is based on OLSR protocol. The message contains the information of how long this node is available among its neighbor nodes(Valid time). The default 
valid time is 6 seconds and 15 seconds for hello message and tc(topology control) message, respectively.

However, the communciation link can be block by ocean wave anytime. So we need to predict how long this link is available. To 
predict this valid time, we need to know the height of the nodes; and to predict nodes, we can use historical height data. So 
nodes should exchange height information between its neighbors, and contruct the routing table and find the best route based 
on height information.
