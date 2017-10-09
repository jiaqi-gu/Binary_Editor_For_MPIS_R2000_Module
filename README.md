This is a binary editor for R2000 object and load modules.

To run the program:

    lmedit filename
    
    e.g.: lmedit sample.obj

Once the file has been loaded, the program begins printing reading editing commands from the standard input. Before each command is read in, the program prints the following prompt:

    section[n] >
    
where section is the (lowercase) name of the current section, n is the sequence number of the command being entered (beginning at 1), and there is a single space immediately before the > prompt character.

Your program must accept the following commands from the standard input:
    
    quit
    size
    write
    section name
    A[,N][:T][=V]

name	A lowercase section name (text, data, etc.)
[ ]	Enclose an optional component
A	An address within the current section
N	A count
T	A type (b, h, w)
V	A replacement value

![alt tag](https://raw.githubusercontent.com/qyqzyd/R2000_Module_Editor/master/R2K_img.png)
