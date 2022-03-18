This is an assembler for the SIC computer. This project has been developed and tested in the linux g++ 
environment.
Input is taken from a file. The file should be present in the same directory as the source cpp file.  
The input must strictly adhere to the following format ->
LABEL OPCODE OPERAND
The input must contain label,operand and opcode fields seperated by a space. Suppose any field 
is not present then the space character should be used to represent the field. Two examples are specified below -> 
Example 1 
Consider an instruction which has no label, the opcode is RSUB and there is no operand. It should be written
in the following manner 
  RSUB  
Note that there will be two spaces after RSUB one space is to represent the field seperation and one space
to indicate that the operand field is empty. Also there are two spaces before RSUB one is to indicate that
the label field is empty and the other space is to seperate the fields.
Example 2
Consider an instruction with no label, the operand is RETADR and the opcode is LDL. The corresponding instruction
is the following
  LDL RETADR
Example 3
Consider an instruction with label FIRST , opcode START and operand 1000. The corresponding instruction is 
FIRST START 1000



The hexadecimal constants X'' are assumed to be of even length. 
Program name is assumed to be atmost 6 letters long. 
It is assumed that the length of the constants is less than 30 bytes.
END Assembler directive must be present at the end to signify the end of the assembly code.  
Three files intermediate.txt , output.txt and listing.txt will be generated. 
The output.txt file will contain machine code corresponding to assembly instructions seperated by the '^' symbol. 
A sample assembly code taken from page 49 of the "System Software: An Introduction to Systems Programming" book is already 
present in the input.txt file. It adheres to the input format specified earlier.Refer to it for further clarifications. 
(Note this is the sample input file specified in assignment)


Execution Instructions 
Place the input file in the same directory as the 190101107_assign1.cpp file.Let us say that the input file is called input.txt
First Run -> g++ 190101107_assign1.cpp -o assign1o
After running the previous command run -> ./assign1o input.txt 
