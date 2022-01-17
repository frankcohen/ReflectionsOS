uzlib - Deflate/Zlib-compatible LZ77 compression library
======================================================

uzlib is a library which can decompress any valid Deflate, Zlib, and Gzip
(further called just "Deflate") bitstream, and compress data to Deflate-
compatible bitstream, albeit with lower compression ratio than Zlib Deflate
algorithm (LZ77 compression is used instead, static Deflate Huffman tree
encoding is used for bitstream).

uzlib aims for minimal code size and runtime memory requirements, and thus
suitable for (deeply) embedded systems.

uzlib is based on:

* tinf library by Joergen Ibsen (Deflate decompression)
* Deflate Static Huffman tree routines by Simon Tatham
* LZ77 compressor by Paul Sokolovsky

Library integrated and maintained by Paul Sokolovsky.

(c) 2014 Paul Sokolovsky

uzlib library is licensed under Zlib license.


The original "tinf" library README follows. NOTE: Some parts may no longer
apply to uzlib.



tinf - tiny inflate library
===========================

Version 1.00

Copyright (c) 2003 Joergen Ibsen

<http://www.ibsensoftware.com/>


About
-----

tinf is a small library implementing the decompression algorithm for the
deflate compressed data format (called 'inflate'). Deflate compression is
used in e.g. zlib, gzip, zip and png.

I wrote it because I needed a small in-memory zlib decompressor for a self-
extracting archive, and the zlib library added 15k to my program. The tinf
code added only 2k.

Naturally the size difference is insignificant in most cases. Also, the
zlib library has many more features, is more secure, and mostly faster.
But if you have a project that calls for a small and simple deflate
decompressor, give it a try :-)

While the implementation should be fairly compliant, it does assume it is
given valid compressed data, and that there is sufficient space for the
decompressed data.

Simple wrappers for decompressing zlib streams and gzip'ed data in memory
are supplied.

tgunzip, an example command-line gzip decompressor in C, is included.

The inflate algorithm and data format are from 'DEFLATE Compressed Data
Format Specification version 1.3' ([RFC 1951][1]).

The zlib data format is from 'ZLIB Compressed Data Format Specification
version 3.3' ([RFC 1950][2]).

The gzip data format is from 'GZIP file format specification version 4.3'
([RFC 1952][3]).

Ideas for future versions:

- the fixed Huffman trees could be built by `tinf_decode_trees()`
  using a small table
- memory for the `TINF_DATA` object should be passed, to avoid using
  more than 1k of stack space
- wrappers for unpacking zip archives and png images
- implement more in x86 assembler
- more sanity checks
- in `tinf_uncompress`, the (entry value of) `destLen` and `sourceLen`
  are not used
- blocking of some sort, so everything does not have to be in memory
- optional table-based huffman decoder

[1]: http://www.rfc-editor.org/rfc/rfc1951.txt
[2]: http://www.rfc-editor.org/rfc/rfc1950.txt
[3]: http://www.rfc-editor.org/rfc/rfc1952.txt


Functionality
-------------

    void tinf_init();

Initialise the global uninitialised data used by the decompression code.
This function must be called once before any calls to the decompression
functions.

    int tinf_uncompress(void *dest,
                        unsigned int *destLen,
                        const void *source,
                        unsigned int sourceLen);

Decompress data in deflate compressed format from `source[]` to `dest[]`.
`destLen` is set to the length of the decompressed data. Returns `TINF_OK`
on success, and `TINF_DATA_ERROR` on error.

    int tinf_gzip_uncompress(void *dest,
                             unsigned int *destLen,
                             const void *source,
                             unsigned int sourceLen);

Decompress data in gzip compressed format from `source[]` to `dest[]`.
`destLen` is set to the length of the decompressed data. Returns `TINF_OK`
on success, and `TINF_DATA_ERROR` on error.

    int tinf_zlib_uncompress(void *dest,
                             unsigned int *destLen,
                             const void *source,
                             unsigned int sourceLen);

Decompress data in zlib compressed format from `source[]` to `dest[]`.
`destLen` is set to the length of the decompressed data. Returns `TINF_OK`
on success, and `TINF_DATA_ERROR` on error.

    unsigned int tinf_adler32(const void *data,
                              unsigned int length);

Computes the Adler-32 checksum of `length` bytes starting at `data`. Used by
`tinf_zlib_uncompress()`.

    unsigned int tinf_crc32(const void *data,
                            unsigned int length);

Computes the CRC32 checksum of `length` bytes starting at `data`. Used by
`tinf_gzip_uncompress()`.


Source Code
-----------

The source code is ANSI C, and assumes that int is 32-bit. It has been
tested on the x86 platform under Windows and Linux.

The decompression functions should be endian-neutral, and also reentrant
and thread-safe (not tested).

In src/nasm there are 32-bit x86 assembler (386+) versions of some of the
files.

Makefiles (GNU Make style) for a number of compilers are included.


Frequently Asked Questions
--------------------------

Q: Is it really free? Can I use it in my commercial ExpenZip software?

A: It's open-source software, available under the zlib license (see
   later), which means you can use it for free -- even in commercial
   products. If you do, please be kind enough to add an acknowledgement.

Q: Did you just strip stuff from the zlib source to make it smaller?

A: No, tinf was written from scratch, using the RFC documentation of
   the formats it supports.

Q: What do you mean by: 'the zlib library .. is more secure'?

A: The zlib decompression code checks the compressed data for validity
   while decompressing, so even on corrupted data it will not crash.
   The tinf code assumes it is given valid compressed data.

Q: I'm a Delphi programmer, can I use tinf?

A: Sure, the object files produced by both Borland C and Watcom C should
   be linkable with Delphi.

Q: Will tinf work on UltraSTRANGE machines running WhackOS?

A: I have no idea .. please try it out and let me know!

Q: Why are all the makefiles in GNU Make style?

A: I'm used to GNU Make, and it has a number of features that are missing
   in some of the other Make utilities.

Q: This is the first release, how can there be frequently asked questions?

A: Ok, ok .. I made the questions up ;-)


License
-------

tinf - tiny inflate library

Copyright (c) 2003 Joergen Ibsen

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
   not claim that you wrote the original software. If you use this
   software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must
   not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
