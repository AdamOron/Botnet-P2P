# Botnet-P2P
This is a decentralized Peer-to-Peer Botnet.  
I began working on this project recently, so it's still in a pretty rough state. There's some junk code used for testing purposes, but the framework itself should be pretty solid.  
This is not yet an actual botnet, but rather the foundation for one. My goals for the next couple of weeks are to make it functional and stable, and then to add some actual malware-like functionality (persistence, privilege escalation, e.t.c).

## The Goal Of This Project
Generally speaking, the purpose of this project is to get into malware development.  
More specifically, my goal is to create a "decentralized" peer-to-peer botnet, where the bots communicate & depend on eachother. Whenever a command is passed to the botnet, instead of having the C&C server communicate directly with each bot, the C&C server contacts a single bot, which then contacts another single bot, and so on.
The purpose of this architecture is to hide the host of the botnet and make him hard to track.

## Project Structure
* The peer-to-peer architecture is built on the networking framework I created, which is located under `src/networking`. This framework contains a Client & Server class, allowing Client-Server communication.  
This networking framework uses the Winsock library.
* The peer-to-peer networking is defined in `src/peer`, where a Peer class is defined alongside the protocol of the architecture itself.
* Like any other networking-related framework, multiple connections need to be managed simultaneously, which requires usage of threads. For this purpose, a basic thread pool/manager is defined under `src/threading`. This is done to keep track of created threads and ensure there's a maximum amount of them allowed.
* I also created an event system that's used to manage all network-related events (data received, connection created, e.t.c). This allows abstraction and separation between the actual network components and the handling of these events.
