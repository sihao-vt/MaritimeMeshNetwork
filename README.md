# OceanWave

##SetUp
First, you need to install ns3.
```
cd
mkdir workspace
cd workspace
wget http://www.nsnam.org/release/ns-allinone-3.24.tar.bz2
tar xjf ns-allinone-3.24.tar.bz2
```
[ns3 Tutorial-Getting Started](https://www.nsnam.org/docs/release/3.24/tutorial/html/getting-started.html#downloading-ns3)

Then, copy `ocean-3d-random-walk.h` and `ocean-3d-random-walk.cc` to ns3/src/mobility/model. Also replace ns3/src/mobility/wcsript with wscript, which is the registration file and will not affect other funcationalities.
Same for copying`ocean-propagation-model.h` and `ocean-propagation-model.cc` to ns3/src/propagation/model, and copying wscript to replace ns3/src/propagation/wsript.

`ocean.cc` is the main function. In my preference, I put this file in scratch/ . Then run
```
./waf --run scratch/ocean
```
will show the result.

