This is my personal toolchain for developing PS1 code.

Code and scripts are in the public domain EXCEPT:
* iso2raw.c which is a modified mashup of two pieces of code and is in an unknown state. It is provided because it is VERY USEFUL.
* nullmon-part.c which uses a part of nullmon.c from newlib. It modifies `get_mem_info` to give the correct memory information so we can use more than 1MB of RAM. It is available under an attribution-style licence.

rawcga.bin is the IBM CGA ROM font.

rawhead2.bin is a PAL CD-ROM header. It is required for PS1 development and ultimately cannot be replaced.

s2-cez.s3m is my cover of Crystal Egg Zone from Sonic 2 Master System. Please ask before using this in your own projects. (But I really don't care if you use it as a placeholder while testing things.)
