The code for the given assignment has been developed and tested in the Linux g++ environment. 
The included source files are assembler.cpp and linkingloader.cpp 


Assembler 
This is an assembler for the SIC/XE computer.
Lets call the input file name - input.txt. It should be present in the working directory in
which assembler.cpp is present.
The code can then be run using the following commands 

First Execute -> g++ -o assembly assembler.cpp 
Then execute -> ./assembly input.txt

The input file should contain instructions in the following format
LABEL OPERATION OPERAND
The label operation and operand should be seperated by a space character. 
Empty fields should be denoted using the space character. 


Three files will be generated as the output -> output.txt, intermediate.txt and listing.txt. 
The object code is present in the output.txt file.
In case the program has multiple control sections, the object code correponding to successive control
sections will be seperate by 2 lines that is 2 empty lines will be inserted after each control section.
This is to improve readability, the object code corresponding to seperate instructions are seperated by 
the '^' (caret) symbol.  


The submission already contains an input file which adheres to the mentioned format (input.txt), this is the assembly code
corresponding to the sample input in the assignment. (Fig 2.16, page 91)




Linking Loader 
The code for the  linking loader for the SIC/XE computer is present in linkingloader.cpp 
Lets call the file containing the input object code - input_linking_loader.txt. Move this file to the current working
directory in which linkingloader.cpp is present. Suppose the starting address is 16384 , the code can then be
run by issuing the following commands. 

g++ -o linkingloader linkingloader.cpp
./linkingloader input_linking_loader.txt 16384

The input file should contain the object code which follows the SIC/XE computer format.
The submission already contains an input file ("input_linking_loader.txt"), this object code is taken from 
System Software An Introduction to Systems Programming, Page No - 137. The starting address 
is 16384.
The output corresponding to the object code is present on  Page No - 140. 

1 file will be generated as the output. It will show the representation of the object code in the 
main memory. Each line in the output file will correspond to 16 bytes of memory, divided into 4 chunks.
If the object code at a particular location is unknown it is represented using the '.' (dot) character. 


