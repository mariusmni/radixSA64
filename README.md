# RadixSA64 - Suffix (L-mer) array algorithm

This program computes either the suffix array or the L-mer array of a string. The suffix array gives the lexicographical order of all the suffixes of a string. The L-mer array gives the lexicographical order of all the L-mers in the string (sorts the suffixes with respect to only the first L characters).

This program is based on the suffix array algorithm [RadixSA](https://github.com/mariusmni/radixSA). RadixSA uses 32 bit integers, so the maximum length of the input string is 2^32. RadixSA64 taxes the integer type as a parameter, so it can use either 32 or 64 bit integers.


## To compile:

Make sure you have ```g++``` and ```make``` installed. 
Open a command line and type: 

```
make
```

## To compute suffix array:

```
./radixSA inputFile outputFile
```

## To compute L-mer array: 

```
./radixSA -k <value> inputFile outputFile
```

## To use as library:


```
#include "radix.h"
    
unum *a = Radix<unum>(str, n).build();

// use array a

delete[] a;
```

where unum can be `unsigned int`, `unsigned long` etc.

See [src/runtimes.cpp](src/runtimes.cpp) for actual code usage.


Please report any bugs to mariusmni@gmail.com
