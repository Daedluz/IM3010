# IM3010 Final note

## TCP Congestion Control

Definition of congestion:  
Too many sources sending too much data too fast for network to handle  
Could lead to 
1. lost packets (router buffer overflow)
2. long delays (queueing)

Goodput: The rate of **useful** data traverse through a link  

<img src="https://i.imgur.com/m7AllxL.png" width="300px"/>  

When a packet is dropped, the upstream transmission capacity used for 
that packet is wasted.  

Approaches to congestion control: 
1. End-to-end: end-system infer congestion by observing delay and loss
2. Network-assisted: routers send feedback to end-systems indicating congestion

### TCP Congestion Control

- Slow start
  - Increase send rate **exponentially** until first loss
  - Increase congestion window (cwnd) by 1 for every ACK received
- Congestion avoidance
  - Congestion is indicated by a timeout or reception of three duplicate ACKs
  - There's a "slow start thresh", initially 65,535 bytes
  - When congestion window = slow start thresh, the connection enters 
**congestion avoidance phase**, where each ack makes $cwnd$ += 
$\frac{1}{cwnd}$
  - When congestion occurs, ssthresh = cwnd / 2 and cwnd = 1

<img src="https://i.imgur.com/Nvvz1uj.png" width="550px"/>  

TCP Tahoe

Fast Retransmit:
- When 3 duplicate ACKs are received, a retransmission 
is performed without waiting for the retransmission timer to expire.  
- Go to slow start phase after fast retransmit

<img src="https://i.imgur.com/4FWLQek.png" width="600px"/>  

TCP Reno

Fast Recovery: 
- Perform congestion avoidance after fast retransmit instead of slow start
- boot up throughput
- ssthresh = cwnd/2, cwnd = cwnd/2 + 3 segments
- Increase cwnd for each ACK received
- After recovering the lost packet, set cwnd = ssthresh and enter congestion avoidance  

<img src="https://i.imgur.com/HtkIeRG.png" width="600px"/>  

TCP throughput $\approxeq \frac{1.22 \cdot MSS}{RTT \cdot \sqrt{L}}$  
Where MSS = maximum segment size

## Network Layer

### Key functions:

- Forwarding: move packet from router input to appropriate output
- Routing: Determine route with routing algorithms

Network layer: between two **hosts**  
Transpor layer: Between two **processes**

Network service model:
- For **individual** datagrams
  - guaranteed delivery
  - deliver in less than 40 msec delay
- For a **flow** of datagrams
  - in-order delivery
  - restrictions on changes in inter-packet spacing (i.e. jitter)
    - jitter: packets arrive in different delay times (?
  - guaranteed minimum bandwidth
  
### Virtual Circuit:

It's a means of transporting data over a packet-switched network in a
way that it appears as though there is a deidcated **physical link**
betweenthe source and destination.

Complexity **inside** network

VC router maintain connection **state information**

- path: from source to destination
- VC numbers: one for each link along path
- Entries in forwarding tables

Packets belonging to VC carries VC number rather than dest addr  
link, router resources (bandwidth, buffers) may be allocated to VC 
(dedicated resources = predictable service)

<img src="https://i.imgur.com/bKSdWEA.png" width="500px"/>  

### Datagram networks

Simple inside network, complexity at edge

- No call setup 
- No state about end to end connection in router
- Packet forwarded using destination host address

<img src="https://i.imgur.com/ETa1JK9.png" width="500px"/>  

Forwarding table: 
Use **Longest prefix matching** to find the output interface

| Destination Address Range            | Link Interface |
|--------------------------------------|----------------|
| 11001000 00010111 0010**** ********* | 0              |
| 11001000 00010111 00011000 ********* | 1              |

### Router Architecture

- Input ports: decentralized switching
  - Lookup output port using forwarding table
  - Goal: complete input port processing at line speed
  - Queuing happens if datagrams arrive faster than forwarding rate
- High-speed switching fabric
  - switching rate: rate at which packets can be transfer from inputs to outputs
  - Memory: speed limited by memory bandwidth
  - Bus: 
    - From input port memory to output port memory by a shared bus
    - Limited by bus bandwidth
    - Now using
  - Interconnection network: Packets can't be too big

<img src="https://i.imgur.com/VDaxx9C.png" width="450px" style="margin-left:50px"/>  

- Output ports
  - Head-of-the-Line(HOL) blocking: Queued datagram at front of queue prevents others in queue from moving forward

## IP: Internet Protocol

Reassembly: 

- length
- ID
- fragflag: 0 means it's the last one
- offset * 8 = Number of bit in the original data

CIDR: Class Inter Domain Routing  
Subnet can consist **arbitrary length**  
Address format: a.b.c.d/x, where x is the number of bits in subnet portion of address

### DHCP: Dynamic Host Configuration Protocol

Dynamically get IP address from some server  
A "plug and play" protocol

1. Host broadcast "DHCP discover" msg
2. DHCP server responds with "DHCP offer"
3. Host request IP address with "DHCP request" msg
4. DHCP server sends address with "DHCP ack" msg

DHCP can return more than just allocated IP:

- address of first-hop router for client
- name and IP address of DNS sever
- network mask (indicating network versus host portion of address)

Local network get subnet part given by ISP,  
ISP get block of address by ICANN

### NAT: Network address translation

Cause: Local network uses just one IP address

- More IP from ISP costs more money
- Change addresses of devices in local network without notifying outsode world
- Change ISP without changing address in local network
- devices inside local net not explicitly addressable, visible by outside world

A NAT router must:

- Replace (source IP, port num) of outgoing datagram to (NAT IP, port num)
- Keep a NAT translation table to keep track of every (source IP, port num) pair to (NAT IP, port num) 
- Replace (NAT IP, port num) of incoming datagram to (source IP, port num)

NAT is controversial:

- Router should only process up to layer 3
- violates end-to-end argument
  - NAT possibility must be taken into account by app designers, e.g., P2P applications
- address shortage should instead be solved by IPv6

NAT traversal problem

- statically configure NAT to forward incoming connection requests at given port to server
- Universal Plug and Play (UPnP) Internet Gateway Device (IGD) Protocol to let NATed host to learn public IP and add/remove port mappings. i.e., automate static NAT port map configuration
- relaying (used in Skype)

<img src="https://i.imgur.com/3GZ2V9k.png" width="500px" style="margin-left:40px"/>  

### ICMP: Internet Control Message Protocol

Used by hosts and routers to communicate network-level information
- Error report: unreachable host, network, port, protocol
- echo request/reply (used by ping)

### IPv6

IPv6 datagram format:

- Priority
- Flow label: identify datagrams in same "flow"
- next header: Identify upper layer protocol for data, something like a pointer
- Checksum: removed entirely to reduce processing time
- Options
- ICMPv6: new version of ICMP

Tunneling: 

IPv6 datagram carried as **payload** in IPv4 datagram among IPv4 routers.  
Just like VPN uses tunneling to add VPN header to the original payload

<img src="https://i.imgur.com/QOgl6Ce.png" width="450px"/>  

### Routing Algorithms

- Flooding Algorithm
  - used in emergency because it's fast
  - Throw every packet to every outgoing link
  - Drawback: generate countless duplicate packet
- Link-State Routing Algorithm
  - Dijkstra's algorithm
- Distance Vector Routing Algorithm
  - Bellman-Ford equation (can be solved by dynamic programming)  
    $d_x(y) = \min {c(x, v) + d_v(y)}$  
    $d_x(y)$ = cost of least-cost path from x to y  
    min over all neighbors v of x  
    $c(x, v)$ is the cost to neighbor v
  - Every node sends its distance vector to every other adjacent node
  - Then each node updates its routing table according to Bellman-Ford equation
  - Link cost changes
    - Good news travels fast
    - Bad news travels slow: "count to infinity" problem
    - Can be solved by poisoned reverse
      - If Z routes through Y to get to X, Z tells Y its (Z's) distance to X is infinite
- Hierarchical Routing Algorithm
  - Aggregate routers into regions, "Autonomous systems" (AS)
  - Router in same AS run same routing protocol (intra-AS routing)
  - Gateway router: Run intra-AS and inter-AS at the same time

Link State vs Distinct Vector 

- Message complexity
  - LS: n node E link, O(nE) msgs sent
  - DV: exchange between neighbors
- speed of convergence
  - LS: $O(n^2)$, may have oscillations
  - DV: Varies, may be routing loops and count to infinity problem
- Robustness
  - LS
    - node could advertise wrong link cost
    - each node computes its own table
  - DV
    - node could advertise incorrect path cost
    - table is used by others → error propagate thru network

### Routing in the Internet

Intra-AS Routing: also known as Interior Gateway Protocols (IGP)

- RIP: Routing Information Protocol
  - Distance Vector Algorithm
  - Routing table managed by application-level
- OSPF: Open Source Shortest Path First
  - Uses link state algorithm: Dijkstra's algorithm
  - Advertisements flooded to **entire** AS directly over IP
  - IS-IS routing protocol: nearly identical to OSPF
  - Hierarchical OSPF:
    - two-level hierarchy: local area, backbone
    - boundary routers: connect to other ASs
- BGP: Border Gateway Protocol
  - Inter-domain routing protocol
  - Glue that holds the Internet together
  - Allows subnet to advertise its existence to rest of Internet
- IGRP: Interior Gateway Routing Protocol

## Data Link Layer

Packets are called frame in data-link layer  
data-link layer has the responsibility of transferring datagram from one node to a **physically adjacent** node over a link  
Each link protocol provides different services (may or may not provide rdt)  
Datagram transferred by different link protocols over different links  
"MAC" (Media Access Control) address to identify source, destination

**Reliable** delivery between adjacent nodes  

### Link Layer Services

- Flow Control
  - pacing between adjacent sending and receiving nodes
- Error Detection
  - Receiver detects presence of errors
  - Signals sender for retransmission or drops frame
- Error Correction
  - Receiver identifies and **corrects** bit errors **without** resorting to retransmission
- Half-duplex and Full-duplex
  - Half-duplex: Sender can send and receive data but one at a time
  - Full-duplex: Sender can send the data and also can receive the data simultaneously

Link layer implemented on network interface card (NIC) or on a chip  
Combination of hardware, soft, firmware  

EDC = Error Detection and Correction bits (redundancy)  
EDC could go wrong, too  
Larger EDC field yields better detection and correction  

Parity checking  
- Single bit parity: Detect single bit errors
- Two-dimensional Bit Parity: detect and correct single bit errors

<img src="https://i.imgur.com/IDo5a9G.png" width="300px"/>  

- Cyclic Redundancy Check
  - Data will be divided by a divisor agreed by both sender and receiver
  - The redundant will be returned

<img src="https://i.imgur.com/MKbT467.png" width="300px"/>  

### Multiple Access Protocols

Single shared broadcast channel

- Channel partitioning
  - TDMA (time)
    - Access to channel in rounds
    - Each station gets fixed length slot in each round
  - FDMA (Frequency)
    - Channel spectrum divided into frquency bands
    - Each station assigned fixed grequency band
  - CDMA (code)
- Random Access
  - Channel not divided, allow collisions, and recover from collisions
  - Slotted ALOHA (Basically TDMA)
    - assume all frames are same size
    - time divided into equal size slots
    - nodes start to transmit only at slot beginning
    - nodes are synchronized so that they know when to transmit
    - if collision, node retransmits frame in each subsequent slot with probability $p$ until success
    - Efficiency: max $\frac{1}{e}$(37%), so bad
  - ALOHA
    - Unslotted
    - Efficiency: $\frac{1}{2e}$ (18%), even worse !
  - CSMA/CD
    - Listen before transmit: idle ? transmit : defer transmission
    - collisions can still occur, because of propagation delay
    - Can't listen when sending frames
    - Collision detection
      - Wired LANs: easy, compare transmitted, received signal strength
      - Wireless LANs: hard,  received signal strength overwhelmed by local transmission strength
    - Binary (exponential) backoff: After m collisions, NIC chooses L at random from 0 to $2^{m-1}$ waits 512K bit times

- Taking turns
  - look for best of both worlds
  - polling: master node invites slave node to transmit in turn
  - token passing: control token passed from one node to next

### LAN Address and ARP (Address Resolution Protocol)

each adapter on LAN has unique LAN address  

ARP table: each IP node on LAN has a table of IP/MAC address mappings

| IP address | MAC address | Time to live |
|------------|-------------|--------------|

ARP Protocol: same LAN
1. A wants to send datagram to B, but doesn't have B's MAC address
2. A broadcasts ARP query, containing B's IP
3. B receives ARP query, reply with it's MAC address
4. A caches IP/MAC pair in its ARP table until TTL expires

ARP Protocol: different LAN  

### Ethernet

- Connectionless
- Unreliable
- CSMA/CD with Binary backoff

Ethernet Switch:

- transparent
- self-learning
  - Switch learns MAC of sender and record in switch table
  - If entry found for destination $\rightarrow$ forward to destination
  - else flood

### Switch vs Routers

| Router | Switch |
|--------|--------|
| Network layer| Link layer |
| Routing algorithms | self-learning & flooding |
| IP address | MAC address |