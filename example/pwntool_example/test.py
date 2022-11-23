import base64
import hashlib
import time
from pwn import *

hostname = 'inp111.zoolab.org'
port = 10011
# r = remote(hostname, port)
r = remote("mercury.picoctf.net", 64260)
r.recvline()
r.recvline()
flag_enc = bytes.fromhex(r.recvline().decode())
fl = len(flag_enc)
# >>> Opening connection to inp111.zoolab.org on port 10011: Done

# r.recv(n)
# r.recvline()
# r.recvuntil(delim)
# r.clean()
# r.send(data)
# r.sendline(line)
# r.sendlineafter(b"What data would you like to encrypt? ", line)

def enc(m):
    r.sendlineafter("What data would you like to encrypt? ", m)
    r.recvline()
    return bytes.fromhex(r.recvline().decode())

enc("a" * (50000 - fl))
keyxor = enc("a" * fl)

def xor(x, y):
    return bytes(a ^ b for a, b in zip(x, y))

key = xor(keyxor, b"a" * fl)
flag = xor(flag_enc, key)
print("picoCTF{%s}" % flag.decode())