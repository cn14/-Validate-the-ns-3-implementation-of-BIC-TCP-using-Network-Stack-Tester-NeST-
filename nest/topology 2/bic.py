import sys
import os

sys.path.append('../')
from nest.topology import *
from nest.experiment import *

##############################
# Topology
#
#   Host 1(h1)---------                    
#                      \                    
#                       \                  
#                        \                
#                        Router ------------- Host 3(h3)
#   .                  /                                    
#   .                 /                                     
#   .                /                                      
#   .               /       	                                
#   Host 2(h2)------                            
#
#
##############################


# Change the parameters below to what you want

# bottleneck = 500
bottleneck = 50
rtt_h1_h3= 12
rtt_h2_h3 = 12
rtt_h1_router = rtt_h1_h3/2
rtt_h2_router = rtt_h2_h3/2
rtt_h3_router = rtt_h1_h3/2
packetSize = 1500
bdp = bottleneck * min(rtt_h1_h3,rtt_h2_h3)
no_of_pkts = int(bdp/packetSize)
startTime = 0
endTime = 20 

bottleneck = str(bottleneck) + 'mbit'
# oneWayDelay=str(int(rtt/2))+'ms'

h1_router_bandwidth = '100mbit'
h2_router_bandwidth = '100mbit'
h3_router_bandwidth = '50mbit'

h1_router_latency = str(rtt_h1_router/2)+'ms'
h2_router_latency = str(rtt_h2_router/2)+'ms'
h3_router_latency = str(rtt_h3_router/2)+'ms'

pfifo_queue_disc = 'pfifo'
pfifo_params = {'limit': str(no_of_pkts)+'p'}

fq_codel_queuedisc = 'fq_codel'

nFlows = 1
tcpVersion = 'bic'

# Set up topology

h1 = Node('Host_1')
h2 = Node('Host_2')
h3 = Node('Host_3')
router= Node('router')

router.enable_ip_forwarding()

(h1_router, router_h1) = connect(h1, router)
(h2_router, router_h2) = connect(h2, router)
(h3_router, router_h3) = connect(h3, router)

h1_router.set_address('10.0.1.1/24')
router_h1.set_address('10.0.1.2/24')
h2_router.set_address('10.0.2.1/24')
router_h2.set_address('10.0.2.2/24')
h3_router.set_address('10.0.3.1/24')
router_h3.set_address('10.0.3.2/24')

h1.add_route('DEFAULT', h1_router)
h2.add_route('DEFAULT', h2_router)
h3.add_route('DEFAULT', h3_router)

router.add_route(h1_router.get_address(), router_h1)
router.add_route(h2_router.get_address(), router_h2)
router.add_route(h3_router.get_address(), router_h3)

h1_router.set_attributes(h1_router_bandwidth, h1_router_latency, fq_codel_queuedisc)
router_h1.set_attributes(h1_router_bandwidth, h1_router_latency, pfifo_queue_disc, **pfifo_params)
h2_router.set_attributes(h2_router_bandwidth, h2_router_latency, fq_codel_queuedisc)
router_h2.set_attributes(h2_router_bandwidth, h2_router_latency, pfifo_queue_disc, **pfifo_params)
h3_router.set_attributes(h3_router_bandwidth, h3_router_latency, fq_codel_queuedisc)
router_h3.set_attributes(h3_router_bandwidth, h3_router_latency, pfifo_queue_disc, **pfifo_params)


flow1 = Flow(h1, h3, h3_router.get_address(), startTime, endTime, nFlows)
flow2 = Flow(h2, h3, h3_router.get_address(), startTime, endTime, nFlows)

exp = Experiment('bic_deepak(low)-Simulation')
exp.add_tcp_flow(flow1, tcpVersion)
exp.add_tcp_flow(flow2, tcpVersion) 
exp.require_qdisc_stats(router_h3)

exp.run()