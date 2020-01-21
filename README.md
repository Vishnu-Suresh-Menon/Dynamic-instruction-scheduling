# Dynamic-instruction-scheduling

Built simulator for an out-of-order superscalar processor based on Tomasulo’s algorithm that fetches, dispatches, and issues N instructions per cycle. Here simulator reads a trace file in folllowing format :

 <PC> <operation type> <dest reg #> <src1 reg #> <src2 reg #> <mem address> , where
  
 <PC> is the program counter of the instruction (in hex)
 <operation type> is either “0”, “1”, or “2”
 <dest reg #> is the destination register of the instruction. If it is -1, then the instruction does not have a destination register (for   example, a conditional branch instruction). Otherwise, it is between 0 and 127
 <src1 reg #> is the first source register of the instruction. If it is -1, then the instruction does not have a first source register.     Otherwise, it is between 0 and 127. 
 <src2 reg #> is the second source register of the instruction. If it is -1, then the instruction does not have a second source register.   Otherwise, it is between 0 and 127.
 <mem address> is the memory address for memory access instructions. If it is 0, then it’s not a memory access instruction. Otherwise, it   is a hex address.
  
The simulator outputs the following measurements after completion of the run: 1. Total number of instructions in the trace. 2. Total number of cycles to finish the program. 3. Average number of instructions completed per cycle (IPC). The simulator also outputs the timing information for every instruction in the trace, in a format that is used as input to the scope too

