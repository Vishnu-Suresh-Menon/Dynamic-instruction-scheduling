#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include<bits/stdc++.h> 

using namespace std;

char* trace_file;
int S, N, BLOCKSIZE, L1_size, L1_ASSOC, L2_size, L2_ASSOC, fetch_bandwidth, cyclecount, tag,
issue_invalid_count, execute_invalid_count, dispatch_invalid_count, fake_ROB_head, fake_ROB_tail, ins_count;
string file, instruction;
stringstream ss;
ifstream readfile;
int *dispatch_queue, *issue_queue, *execute_queue;
bool fetchFlag;
struct RF_entry{
int newname;
bool readybit;   
}RF[128];

struct fake_ROB_entry{
string PC, memory_address, instruction_state;
int operation_type, destination_reg, destination_tag, source1_reg, source2_reg, source1_tag, source2_tag, 
begin_cycle_IF, begin_cycle_ID, begin_cycle_IS, begin_cycle_EX, begin_cycle_WB, begin_cycle_CMP, timer, tag, fake_ROB_head, fake_ROB_tail;
bool source1_readybit, source2_readybit;
} fake_ROB[1024];

void initialize_queue(int[], int);
bool arraycomparison(int, int);
void fakeretire();
void execute();
void issue();
void dispatch();
void fetch();
bool advance_cycle();


int main(int argc, char* argv[])
{
  S          = stoi(argv[1]); 
  N          = stoi(argv[2]);
  BLOCKSIZE  = stoi(argv[3]);
  L1_size    = stoi(argv[4]);
  L1_ASSOC   = stoi(argv[5]);
  L2_size    = stoi(argv[6]);
  L2_ASSOC   = stoi(argv[7]);
  trace_file = argv[8];

  ss << trace_file; ss >> file;

  fake_ROB_head = 0;
  fake_ROB_tail = 0;
  ins_count = 0;
  fetch_bandwidth = N;
  execute_invalid_count = N;
  issue_invalid_count = S;
  dispatch_invalid_count = 2*N;

  readfile.open(trace_file);

   dispatch_queue = new int[2*N];
   issue_queue = new int[S];
   execute_queue = new int[20*N];

   for(int i=0; i<1024; i++)
   {
      fake_ROB[i].tag = -1;
      fake_ROB[i].source1_readybit=true;
      fake_ROB[i].source2_readybit=true;
   }

   for(int i=0;i<128;i++)
   {
      RF[i].readybit=true;
   }

   initialize_queue(dispatch_queue, 2*N);
   initialize_queue(issue_queue, S);
   initialize_queue(execute_queue, 20*N);

  do
  {
     fakeretire();
     execute();
     issue();
     dispatch();
     fetch();

  }while(advance_cycle());

  cout << "CONFIGURATION"<<endl;
  cout <<" "<<"superscalar bandwidth (N) = "<<N<<endl;
  cout <<" "<<"dispatch queue size (2*N) = "<<2*N<<endl;
  cout <<" "<<"schedule queue size (S)   = "<<S<<endl;
  cout <<" "<<"RESULTS"<<endl;
  cout <<" "<<"number of instructions = "<<ins_count<<endl;
  cout <<" "<<"number of cycles       = "<<cyclecount<<endl;
  printf(" IPC                    = %0.2f\n", (float)ins_count/(float)cyclecount);

  return 0;

}


void initialize_queue(int array[], int size)
{
   for(int i=0; i<size; i++)
   array[i] = -1;
}

bool arraycomparison(int startindex, int endindex)
{
   return(fake_ROB[startindex].tag<fake_ROB[endindex].tag);
}

void fakeretire()
{

  while(fake_ROB[fake_ROB_head].instruction_state == "WB")
  {
     fake_ROB[fake_ROB_head].instruction_state = "NA";
     fake_ROB[fake_ROB_head].begin_cycle_CMP = 1;

     cout<<fake_ROB[fake_ROB_head].tag<<" "
     << "fu{"<<fake_ROB[fake_ROB_head].operation_type<<"}"<<" "
     << "src{"<<fake_ROB[fake_ROB_head].source1_reg<<","<<fake_ROB[fake_ROB_head].source2_reg<<"}"<<" "
     << "dst{"<<fake_ROB[fake_ROB_head].destination_reg<<"}"<<" "
     << "IF{"<<fake_ROB[fake_ROB_head].begin_cycle_IF<<","<<fake_ROB[fake_ROB_head].begin_cycle_ID-fake_ROB[fake_ROB_head].begin_cycle_IF<<"}"<<" " 
     << "ID{"<<fake_ROB[fake_ROB_head].begin_cycle_ID<<","<<fake_ROB[fake_ROB_head].begin_cycle_IS-fake_ROB[fake_ROB_head].begin_cycle_ID<<"}"<<" "
     << "IS{"<<fake_ROB[fake_ROB_head].begin_cycle_IS<<","<<fake_ROB[fake_ROB_head].begin_cycle_EX-fake_ROB[fake_ROB_head].begin_cycle_IS<<"}"<<" "
     << "EX{"<<fake_ROB[fake_ROB_head].begin_cycle_EX<<","<<fake_ROB[fake_ROB_head].begin_cycle_WB-fake_ROB[fake_ROB_head].begin_cycle_EX<<"}"<<" "
     << "WB{"<<fake_ROB[fake_ROB_head].begin_cycle_WB<<","<<fake_ROB[fake_ROB_head].begin_cycle_CMP<<"}"<<endl;
     
     fake_ROB_head++;

     if(fake_ROB_head == 1024)
     fake_ROB_head = 0;
   }

}

void execute()
{
   for(int i=0; i<(20*N); i++)
   {
      if(execute_queue[i] != -1)
      {
         if(fake_ROB[execute_queue[i]].instruction_state == "EX")
         {
            if(((fake_ROB[execute_queue[i]].operation_type == 0) && (fake_ROB[execute_queue[i]].timer == 1))||
               ((fake_ROB[execute_queue[i]].operation_type == 1) && (fake_ROB[execute_queue[i]].timer == 2))||
               ((fake_ROB[execute_queue[i]].operation_type == 2) && (fake_ROB[execute_queue[i]].timer == 5)))
            {
               fake_ROB[execute_queue[i]].instruction_state = "WB";
               fake_ROB[execute_queue[i]].begin_cycle_WB = cyclecount;

               if(fake_ROB[execute_queue[i]].destination_reg != -1)
               {
                  if(RF[fake_ROB[execute_queue[i]].destination_reg].newname == fake_ROB[execute_queue[i]].destination_tag)
                  RF[fake_ROB[execute_queue[i]].destination_reg].readybit = 1;
               }

               for(int j=0; j<1024; j++)
               {
                  if(fake_ROB[j].source1_tag == fake_ROB[execute_queue[i]].destination_tag)
                  fake_ROB[j].source1_readybit = 1;
                       
                  if(fake_ROB[j].source2_tag == fake_ROB[execute_queue[i]].destination_tag)
                  fake_ROB[j].source2_readybit = 1;
               }

               fake_ROB[execute_queue[i]].timer = 0;
               execute_queue[i] = -1;
                               
            }
            else
            fake_ROB[execute_queue[i]].timer++;   
         }
      }
   }  
}


void issue()
{
   sort(issue_queue, issue_queue+(S), arraycomparison);

   for(int i=0,k=0; i<S && k<N; i++)
   {
      if(issue_queue[i] != -1)
      {
         if(fake_ROB[issue_queue[i]].instruction_state == "IS")
         {
            if(fake_ROB[issue_queue[i]].source1_readybit && fake_ROB[issue_queue[i]].source2_readybit)
            {
               for(int j=0; j<(20*N); j++)
               {
                  if(execute_queue[j] == -1)
                  {
                     execute_queue[j] = issue_queue[i];
                     issue_queue[i] = -1;
                     issue_invalid_count++;
                     k++;

                     fake_ROB[execute_queue[j]].instruction_state = "EX";
                     fake_ROB[execute_queue[j]].begin_cycle_EX = cyclecount;
                     fake_ROB[execute_queue[j]].timer = 1;

                     break;
                  }
               }
            }
             
         }
      }
   }

}

void dispatch()
{  
   sort(dispatch_queue, dispatch_queue+(2*N), arraycomparison);

  for(int i=0; i<(2*N); i++)
  {
     if(dispatch_queue[i] != -1)
     {
        if(fake_ROB[dispatch_queue[i]].instruction_state == "ID")
        {
           if(issue_invalid_count)
           {  
              for(int j=0; j<S; j++)
              {
                 if(issue_queue[j] == -1)
                 {
                    issue_queue[j] = dispatch_queue[i];
                    dispatch_queue[i] = -1;
                    issue_invalid_count--;
                    dispatch_invalid_count++;

                    fake_ROB[issue_queue[j]].instruction_state = "IS";
                    fake_ROB[issue_queue[j]].begin_cycle_IS = cyclecount;

                    if(fake_ROB[issue_queue[j]].source1_reg == -1)
                    fake_ROB[issue_queue[j]].source1_readybit = 1;
                    else
                    {
                       if(RF[fake_ROB[issue_queue[j]].source1_reg].readybit)
                       fake_ROB[issue_queue[j]].source1_readybit = 1;
                       else
                       {
                         fake_ROB[issue_queue[j]].source1_readybit = 0; 
                         fake_ROB[issue_queue[j]].source1_tag = RF[fake_ROB[issue_queue[j]].source1_reg].newname;
                        } 
                     }

                    if(fake_ROB[issue_queue[j]].source2_reg == -1) 
                    fake_ROB[issue_queue[j]].source2_readybit = 1;
                    else
                    {
                       if(RF[fake_ROB[issue_queue[j]].source2_reg].readybit)
                       fake_ROB[issue_queue[j]].source2_readybit = 1;
                       else
                       {
                         fake_ROB[issue_queue[j]].source2_readybit = 0; 
                         fake_ROB[issue_queue[j]].source2_tag = RF[fake_ROB[issue_queue[j]].source2_reg].newname;
                        } 
                     }

                    if(fake_ROB[issue_queue[j]].destination_reg != -1)
                    {
                        RF[fake_ROB[issue_queue[j]].destination_reg].readybit = 0;
                        RF[fake_ROB[issue_queue[j]].destination_reg].newname = issue_queue[j];
                     }
                  break;
                  }
               }
            }
         }
         else
         {
            fake_ROB[dispatch_queue[i]].instruction_state = "ID";
            fake_ROB[dispatch_queue[i]].begin_cycle_ID = cyclecount;
         }
      }
   }

}

void fetch()
{

   //if(!readfile) 
   //if(readfile.eof())
   //cout << "Cannot open file.\n"; 
 
  for(int i=0,j=0; i<(2*N) && j<N; i++)
  { 
     if(!readfile.eof())
     {    
        if(dispatch_invalid_count)                              /*&& getline(readfile,instruction)*/
        {   
            if(dispatch_queue[i] == -1)                         /*instruction!="" && */
           {  
               if(readfile>>fake_ROB[fake_ROB_tail].PC>>fake_ROB[fake_ROB_tail].operation_type>>fake_ROB[fake_ROB_tail].destination_reg
               >>fake_ROB[fake_ROB_tail].source1_reg>>fake_ROB[fake_ROB_tail].source2_reg>>fake_ROB[fake_ROB_tail].memory_address) 
               {
                 fake_ROB[fake_ROB_tail].tag = ins_count;
                 fake_ROB[fake_ROB_tail].destination_tag = fake_ROB_tail;
                 dispatch_queue[i] = fake_ROB[fake_ROB_tail].destination_tag ;

                 fake_ROB[fake_ROB_tail].instruction_state = "IF";
                 fake_ROB[fake_ROB_tail].begin_cycle_IF = cyclecount;

                 dispatch_invalid_count--;
                 j++;
                 ins_count++;

                 fake_ROB_tail++;
                 if(fake_ROB_tail == 1024)
                 fake_ROB_tail = 0; 
               }
            }
         }
      } 
   }
}

bool advance_cycle()
{
   //cout<<"advance_cycle"<<endl;
   if((fake_ROB_head == fake_ROB_tail) && readfile.eof())
   {
    //cout<<"terminated"<<endl;
      return false;
   }
  
   else
   {
      //cout<<fake_ROB_head<<" "<<fake_ROB_tail<<endl;
      cyclecount++;
      return true;
   }

}


