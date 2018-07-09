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

copy `ocean-3d-random-walk.h` and `ocean-3d-random-walk.cc` to ns3/src/mobility/model/. Also use `wscript` to replace ns3/src/mobility/wcsript, which is the registration file and will not affect other funcationalities.

copy `ocean-propagation-model.h` and `ocean-propagation-model.cc` to ns3/src/propagation/model, and copy `wscript` to replace ns3/src/propagation/wsript.

copy `ocean-olsr-helper.h` and `ocean-olsr-helper.cc` to ns3/src/olsr/helper, and copy `wscript` to replace ns3/src/propagation/wsript, and copy other file to ns3/src/olsr/model.

`oceansimulation.cc` is the main function. I perfer to put this file in ns3/scratch/ . Then run
```
./waf --run scratch/oceansimulation
```
will show the result.


## Functionality

### Mobility Model

Vertically, we read height data from ocean data file. Horizontally, the position of each buoy is fixed.


### Propagation Model

We use a Friis/two-ray propagation model.

Also, this propagation model can check if the communication link is blocked by ocean wave. If so, we assume that the communication 
link is corrupted.

### Link-state-aware routing protocol

This protocol is based on OLSR protocol. 

The routing protocol can predict the communication stability and calculate the routing protocol.
