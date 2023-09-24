# NYCU Introduction to Network Programming 111-1
* Prof. 黃俊穎

## lab01 : docker & simple packet analsys

## lab02 : binary file format handling

## lab03 : TCP clients - simple & constant bitrate

## lab04 : nkat TCP server

## lab05 : simple TCP chat server


lab06 requires the `netem` module in your Linux environment.
If you are running docker on Windows, please ensure you use `WSL2` and replace the default kernel with the one built with netem module.
You may follow the instructions below to install a customized Linux kernel.
download the bzImage [here](https://inp111.zoolab.org/wsl/kernel.bzImage)
1. assume you place the kernel file in the `C:\wsl2\` directory, and the kernel is renamed to `kernel.bzImage`.
2. Shutdown WSL2 using the command `wsl --shutdown`. If you have running docker instances, you should terminate your docker first.
3. Create (or modify) the `%UserProfile%\.wslconfig` file. The `%UserProfile%` path is typically at `C:\Users\<username>`. 
  - If you are unsure, type the command `echo %UserProfile%` in a DOS command line prompt, or simply type `%UserProfile%` in the location bar of your File Explorer. 
  - A sample `.wslconfig` file is available here. You can use it directly if you don’t have one.
4. Restart your WSL2 system and docker, or reboot your computer. You can run the command `uname -a` in your WSL or Linux container to verify if you are running the new kernel. The reported Linux kernel version should be `5.15.68.1-microsoft-standard-WSL2` built on Oct 6, 2022.
## lab06 : traffic shaping & TCP performance



Our next few labs may require the `pwntools` package. 
You can install it by reading the installation document listed below.
Some labs also require `proof of work (PoW)` before accessing the lab servers. 
To install pwntools :
```
virtualenv -p python3 ~/pwntools
. ~/pwntools/bin/activate
pip3 install --upgrade pwntools
```
## lab07 : Reentrant CTF challenges

## lab08 : Robust UDP challenge
* This lab aims to practice implementing a robust file transmission protocol using UDP. The objective is to send 1000 files from a client to a server over a lossy link. You have to implement both the server and the client. We do not have a spec for the protocol. You can decide how to transmit the files by yourself. The only limitation is that you must transmit the files over UDP.
* The challenge server checks the transmitted files right after your client program terminates. It then reports how many files have been correctly transmitted over the lossy link. 
* The settings for the lossy link emulated by netem is `limit 1000 delay 100ms 50ms loss 40% corrupt 10% rate 10Mbit.`
* To run the program:
  * For the server:
  * ./server \<path-to-store-files\> \<total-number-of-files\> \<port\>
  ```console
  ./server files 1000 6000
  ```
  * For the client:
  * ./client \<path-to-read-files\> \<total-number-of-files\> \<port\> \<server-ip-address\>
  ```console
  ./client files 1000 6000 127.0.0.1
  ```
* Suppose the files are only stored in the directory where the `client` is, after the sending process, the files should also present in the `server` side
* You could test it by placing `client` and `server` in different directory and run both programs(run `server` first)

## lab09 : UNIX Domain Sudoku
* This lab aims to practice implementing a UNIX domain socket client that interacts with a server. You must implement a Sudoku solver and upload the solver executable to the challenge server. To solve the puzzle, the solver must interact with the Sudoku server using a UNIX domain socket connection stored at `/sudoku.sock`.
* The challenge server has two channels to receive commands. One receives commands from the terminal, and the other receives commands from a stream-based UNIX domain socket created at the path `/sudoku.sock` (on the server). 
* The commands and the corresponding responses are the same for both channels, but no welcome messages are sent via the UNIX domain socket.
* The commands used for the challenge server are listed below for your reference.
```
- H: Show help message.
- P: Print the current board.
- V: Set value for a cell. Usage: V [row] [col] [val]
- C: Check the answer.
- S: Serialize the current board.
- Q: Quit.
```

## lab10 : Broadcast XDP File Transfer & Raw Socket
* This lab aims to practice implementing a broadcast and raw socket program. The objective is to send 1000 files from a client to a server, just like the scenario we used in the Robust UDP Challenge. 
* The link quality between the client and the server is good. However, the network settings reject all packets delivered using well-known transport protocols, including TCP, UDP, and SCTP. Packets sent to unicast addresses are dropped as well. 
* You have to implement both the server and the client. We do not have a spec for the protocol. You can decide how to transmit the files by yourself. Once you have completed your implementations, you can upload your compiled server and client to our challenge server for evaluation. Good luck, and have fun!
* You have to specify your selected protocl number when creating a raw socket. My sample code is socket(AF_INET, SOCK_RAW, IPPROTO_RAW).
* IP header is filled by myself. The source IP should be set to the broadcast address for the packet to be correctly sent.(Server won't receive packet from the same IP it has.)
* To fill the IP header by yourself, the IP_HDRINCL option for a created raw socket is enabled. 
* Don’t forget to ensure that your IP packet has a correct checksum.
* The data structure of the IP header can be found in `/usr/include/netinet/ip.h`. Please `#include <netinet/ip.h>` and then you can use the `structure ip` declared in the header file. Here is an online version `ip.h` for your convenience.
* One final note: You have to enable S`O_BROADCAST` option for your raw socket(requires root permission).
---
* You could look up your broadcast address by `ip addr show`
  * Example:
  * ![image](https://user-images.githubusercontent.com/79355721/210176806-0a2ce91a-3a4e-4dff-b592-c00d7b486b52.png)
* To run the program:
  * For the server:
  * ./server \<path-to-store-files\> \<total-number-of-files\> \<port\>
  ```console
  ./server files 1000 6000
  ```
  * For the client:
  * ./client \<path-to-read-files\> \<total-number-of-files\> \<port\> \<server-ip-address\>
  ```console
  ./client files 1000 6000 xxx.xxx.xxx.xxx
  ```
* Suppose the files are only stored in the directory where the `client` is, after the sending process, the files should also present in the `server` side
* You could test it by placing `client` and `server` in different directory and run both programs(run `server` first)

## HW1 : Real-world TCP chat server - IRC protocol with weechat client
[RFC 1459](https://www.rfc-editor.org/rfc/rfc1459)



# References
The entry for the course page is [here](https://people.cs.nctu.edu.tw/~chuang/courses/netprog/)
You may need NYCU ip to have access to the website
