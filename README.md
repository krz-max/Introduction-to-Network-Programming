# NetProgIntro

## lab01

## lab02

## lab03

## lab04

## lab05


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
## lab06



Our next few labs may require the `pwntools` package. 
You can install it by reading the installation document listed below.
Some labs also require `proof of work (PoW)` before accessing the lab servers. 
To install pwntools :
```
virtualenv -p python3 ~/pwntools
. ~/pwntools/bin/activate
pip3 install --upgrade pwntools
```
## lab07



## lab08

## lab09

## lab10




# References
The entry for the course page is [here](https://people.cs.nctu.edu.tw/~chuang/courses/netprog/)
