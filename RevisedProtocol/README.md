### Message Header

In message header, we should send predicted ocean elevation information besides default valid time information. 
Then the receiver will determine the valid time based on the packet type. 
If it is a hello message, the valid time information should be calculated based on predicted nodes elevation and sea state. 
Otherwise, we use default valid time information.

### Link-state-aware Routing Protocol

Sensors on buoy will measure wind speed and record node elevation data. 
Based on historical elevation information, routers will predict their elevation in the future. 
This is called predicted node elevation information. 
The predicted node elevation information should be included in hello message. 
After received a hello message from its neighborhood, 
the receiver will calculate the link stability according to the sea states and predicted elevation information 
of both transmitter and receiver. 
we need to choose appropriate link stability as the threshold in different sea states. 
In link-state-aware routing protocol, if link stability between two nodes is above this threshold,
we assume the communication link is available and it is a potential routing path; 
otherwise, we assume this link is unstable and we will not choose it when calculating routing table.
Routing messages should be exchanged more frequently due to dynamic ocean wave. 
The information holding time should also be shortened.
