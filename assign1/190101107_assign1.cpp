/**** 
Code should be compiled and tested in the linux g++ environment
****/

#include<bits/stdc++.h> 
using namespace std; 
map<string,string> optab; 
map<string,int> symtab; 
//checks whether an opcode is present or not
map<string,int> present_op;  
//checks whether a symbol is present or not
map<string,int> present_sym;  
// register mapping 
map<string , string> registers; 
/* 
We must store block information for each control section. 
In particular we would want the external references and definition in each block, 
along with the names of each section and their sizes. Also store the addresses
for the definitions.
*/ 
vector<string> block_names; 
vector<int> block_sizes; 
vector<vector<string>> block_definitions; 
vector<vector<string>> block_references;  
vector<vector<int>> instruction_to_address_mul; 
vector<vector<int>> instruction_to_address_mulhex; 
vector<map<string,int>> present_sym_mul; 
vector<map<string,int>> sym_tab_mul; 



// Setting the registers on SIC/XE machine 
void mapping_register() 
{ 
    registers["T"] = "5"; 
    registers["B"] = "3"; 
    registers["S"] = "4"; 
    registers["F"] = "6"; 
    registers["A"] = "0"; 
    registers["X"] = "1"; 
    registers["L"] = "2";
    registers["PC"] = "8"; 
    registers["SW"] = "9";
}
// Subroutine used tby setup_optab optable
void Set(const string &s1,const string &code) 
{ 
    optab[s1] = code;
    present_op[s1] = 1;
}  

// Routine to fill the optable
void setup_optab(){ 

Set("LDA" , "00"); 
Set("LDX" , "04"); 
Set("LDL" , "08"); 
Set("LDCH" , "50"); 
Set("STA" , "0C");
Set("STX" , "10");
Set("STL" , "14"); 
Set("STCH","54"); 
Set("SUB" , "1C");
Set("ADD" , "18"); 
Set("MUL" , "20"); 
Set("DIV" , "24"); 
Set("COMP" , "28"); 
Set("J" ,"3C"); 
Set("JLT" , "38"); 
Set("JEQ" , "30"); 
Set("JSUB" , "48"); 
Set("JGT" , "34"); 
Set("RSUB" , "4C"); 
Set("TIX" , "2C");
Set("TD" , "E0");
Set("RD" , "D8"); 
Set("WD" , "DC"); 
Set("TIXR" , "B8");
Set("CLEAR" , "B4"); 
Set("COMPR" , "A0"); 

}  
// These vectors are used for resolving references
vector<int> instruction_to_address;
vector<string> instruction_to_address_hex; 

/* decomposes a line of input into label , opcode and operand
It returns 0 to indicate that the current line in the assembly 
code corresponds to a comment
*/
int parse_line(string &s1 , vector<string> &v1) 
{ 
if(s1[0] == '.') 
return 0; 
int length = s1.size(); 
for(int i=0;i<length;++i) 
{   
    int index=i ; 
    string temp; 
    while(index< length && s1[index]!=' ') 
    { 
        temp.push_back(s1[index]); 
        index++;
    } 
    if(temp.size() > 0) 
        v1.push_back(temp);  
    else 
    { 
        v1.push_back(" ");
    }
    i= max(index, i+1);

}  
if(v1.size() < 3) 
    v1.push_back(" ");
return 1; 

}

/* Pads the character c to the input so that it's size becomes tlen
if start = 1 c is padded to the start of the input  
if start = 0 c is padded to the end of the input
*/
void padd(string &input , char c , int tlen , int start = 0) 
{ 
    int n=input.size(); 
    if(n == tlen) 
        return ; 
    if(start == 1)
        reverse(input.begin() , input.end()); 
    for(int i=n;i<tlen;++i) 
    { 
        input.push_back(c);
    }  
    if(start == 1)
        reverse(input.begin(),input.end()); 
} 

// Converts a hexadecimal into decimal
int hextodec(string &s1) {
    int ans=0;  
    int n = s1.size();
    for(int i=0;i<n;++i) 
    { 
        ans = ans*16 + ((s1[i]<='9')?(s1[i]-'0'):(s1[i]-'A' + 10)); 
    }  
    return ans;
} 

/* Converts a decimal into hex format
In mode 1 which is the default mode the string is padded to make its length 4. This mode should be used for addresses
Otherwise the string is returned as it is. 

*/
string dectohex(int n,int mode = 1) 
{ if(n==0) 
    {if(mode == 1)
        return "0000"; 
     else 
        return "0";
    }
    string s1; 
    int dec; 
    while(n!=0) 
    { 
            dec = (n%16); 
            if(dec<=9) 
            { 
                s1.push_back((char)(dec+'0'));
            } 
            else 
            { 
                s1.push_back((char)(dec - 10 +'A'));
            } 
            n=n/16;
    } 
    reverse(s1.begin(),s1.end());   
    if(mode == 1)
    padd(s1,'0',4,1); 


    return s1; 
}

// All the addresses initially in decimal are converted into hexadecimal
void convert() 
{ 
    for(auto &it: instruction_to_address) 
    { 
        instruction_to_address_hex.push_back(dectohex(it)); 
    }
}

// Returns the size of the constant in bytes
int detsize(string &const1) 
{ 
    if(const1[0]=='C') 
        return (const1.size() - 3); 
    else 
        return (const1.size()-3)/2; 

}
/* Converts constant into hexadecimal form 
*/
string convert_const(string &inp) 
{ string result; 
  int n = inp.size();  
    if(inp[0] == 'X') 
        {
            for(int i=2;i<n-1;++i) 
                result.push_back(inp[i]);
        }  
    else 
        {  
            for(int i=2;i<n-1;++i) 
                { 
                    string asciicode = dectohex((int)inp[i],0); 
                    if(asciicode.size() == 1) 
                        padd(asciicode, '0', 2 , 1); 
                    for(auto &it: asciicode) 
                        result.push_back(it);
                }
        } 
    return result;
}

// Used to parse the header line
void parse_header(ifstream &inpfile , ofstream &outfile, ofstream &loutput, int start , int length,int start_present) 
{ 
if(start_present == 1) 
    {
    string header = "H";  
    header.push_back('^');
    string temp;  
    string listingstring = dectohex(start); 
    listingstring.push_back(' '); 
    getline(inpfile, temp); 
    for(auto &it:temp)
        listingstring.push_back(it); 
    vector<string> array;  
    parse_line(temp,array);   
    padd(array[0] , ' ' , 6 , 0);   
    for(auto &it : array[0]) 
        header.push_back(it); 
    header.push_back('^');  
    padd(instruction_to_address_hex[0] , '0' , 6 ,1); 
    for(auto &it:instruction_to_address_hex[0]) 
        header.push_back(it);   
    header.push_back('^');
    string lengthhex = dectohex(length,0); 
    padd(lengthhex,'0',6,1); 
    for(auto &it: lengthhex) 
        header.push_back(it);  
    outfile << header<< "\n";    
    loutput << listingstring<<"\n"; }
else 
    {  // There was no START command found
        string header = "H^"; 
        for(int i=0;i<6;++i) 
            header.push_back(' '); 
        header.push_back('^'); 
        string starting = dectohex(start); 
        string sizestring = dectohex(length,0); 
        padd(starting,'0',6,1); 
        padd(sizestring , '0' , 6 , 1); 
        for(auto &it:starting) 
            header.push_back(it); 
        header.push_back('^'); 
        for(auto &it: sizestring) 
            header.push_back(it); 
        outfile<<header<<"\n";
    }
}

 
/* 
This function computes the machine code for an assembly instruction and writes the listing record 
It returns -1,0,1,2 depending on the assembly instruction 
0 indicates that text record is full and the current code cannot be fit in the current text record 
1 indicate that machine code was computed and can be placed in the current text record
-1 indicates that END assembler directive was encountered , hence the end record should be inititalized
2 indicates that a RESW or RESB opcode was present in the input so a new text record must be initialized
*/
int parse_command(string &addr,ofstream &loutput , string &input , int& col ,string & out, int write_listing = 1) 
{   int flag=1; 
    int size_str = addr.size()-1; 
    string listingstring;   
    /* 
    Incase the string is already padded take last 4 characters only
    If the address size is less than 3 we need to pad characters at the start
    */ 
    if(size_str < 3) 
        { 
            for(int i=0;i<(3-size_str);i++) 
                listingstring.push_back('0'); 
        
        }
    for(int i=max(size_str - 3,0);i<=size_str;++i) 
        listingstring.push_back(addr[i]); 
    listingstring.push_back(' ');
    for(auto &it:input) 
        listingstring.push_back(it); 
    
    
    vector<string> array; 
    parse_line(input,array);  
    int length = 0; 
    
    if(array[1] == "END") 
        {   listingstring = "  "; 
            for(auto &it: input) 
                listingstring.push_back(it);
            loutput  << listingstring <<"\n";
            return -1;}
    if(present_op[array[1]] == 1) 
    {
        string code = optab[array[1]]; 
        length += (code.size()/2);
        for(auto &it: code) 
            out.push_back(it); 
      
        if(array[2] == " ") 
            {  
                for(int i=0;i<4;++i) 
                    out.push_back('0');
                length += 2;
            } 
        else 
            {    
                int flag2 = 0; 
                string to_convert = array[2]; 
                int t_size = array[2].size();  
                // The instructions that use indexed addressing terminate with ,X and need to be handled specially
                if(t_size > 2) 
                { if((to_convert[t_size-2] == ',') && (to_convert[t_size-1] == 'X') ) 
                    {flag2=1; 
                    to_convert= array[2].substr(0,t_size-2);  
                   
                    }
                } 
                if(present_sym[to_convert] == 0) 
                { 
                    cout<<"symbol "<<to_convert<<" not found\n"; 
                    _Exit(0);
                }
                for(auto &it:dectohex(symtab[to_convert])) 
                    out.push_back(it);   
                
                if(flag2 == 1) 
                { 
                    out[out.size() - 4] = '9';
                }
                length+=2;
            }
        
    } 
    else if ( (array[1] == "WORD") || (array[1] == "BYTE"))
    { string result;
       if(array[1] == "BYTE") 
            {
            length = detsize(array[2]); 
            result = convert_const(array[2]); 
            } 
        else 
            {  length = 3;
                result = dectohex(stoi(array[2]),0); 
                padd(result,'0',6,1);

            }      
        for(auto &it : result) 
            out.push_back(it);
    } 
    else  
    { if((array[1]=="RESB") || (array[1] == "RESW")) 
      {   
        flag=0;
        length += 0;
      } 
      else 
      {   cout<<"opcode "<<array[1]<<"not found\n";  
          _Exit(0);
      }  
    }  
    if(length > 0) 
        { 
            listingstring.push_back(' '); 
            for(auto &it : out) 
                listingstring.push_back(it); 
        }  
    if(write_listing == 1)
        loutput << listingstring <<"\n"; 

if(col + length*2 - 1 <= 69) 
    {   if(flag == 0) 
            return 2;
        col = col + length*2;
        
        return  1;
    } 
else 
    {   
        return 0;
    }
}
/* 
This is the function that is used for the second pass of the assembler
First the initialize the HEADER record
Then loop through the input file until the "END" directive is found, create text record when needed
Lastly the end record is initialized


*/
int pass2(ifstream & inpfile , ofstream &outfile,ofstream &loutput, int start , int length,int start_present) 
{  
parse_header(inpfile,outfile,loutput,start,length,start_present);  
string temp;
string tempout;  
/* 
There can be a scenario where we read an instruction but cannot fit in the current text record
In such a case when we initialize the new text record we must prevent reading of a new instruction
and use the instruction which could not be fit in the earlier text record.
*/
int spare= 0; 
string sparestring; 
string listingstring;
int ic = 0;
while(true) 
{ 
int flag=1;
string textrecord = "T";   
textrecord.push_back('^');
padd(instruction_to_address_hex[ic] , '0' ,6, 1); 
for(auto &it : instruction_to_address_hex[ic]) 
{ 
    textrecord.push_back(it);
}      
textrecord.push_back('^'); 
//addr_check is initialized to the address of the first instruction which is not "RESW" \ "RESB" type 
string addr_check;
// We dont know the length yet so just putting 00
for(int i=0;i<2;++i)
    textrecord.push_back('0'); 
textrecord.push_back('^');  
int properly_parsed = 0;
int col = 10; 
while(true) 
{   int spec= 1;
    if(spare == 0)
        getline(inpfile,temp); 
    else 
        {temp = sparestring; 
        spec = 0;
        spare = 0; 
        } 
    int status = parse_command(instruction_to_address_hex[ic] , loutput, temp,col,tempout,spec); 
    if(status == 2) 
        {   tempout.clear();
            ic++; 
            if(properly_parsed == 0) 
                continue; 
            else 
                break; 
        }
    if(status == -1) 
    {   tempout.clear();
        flag= 0; 
        break;
    } 
    if(status == 0) 
    { spare= 1; 
        sparestring = temp; 
        tempout.clear();
        break;
    } 
    if(properly_parsed == 0) 
        addr_check = instruction_to_address_hex[ic];  
    properly_parsed = 1;
    for(auto &it:tempout) 
    { 
        textrecord.push_back(it); 
    }  
    textrecord.push_back('^');
    ic++;
    tempout.clear();    
}   
padd(addr_check,'0',6,1); 
for(int i=2;i<=7;++i) 
    textrecord[i] = addr_check[i-2];
if(col == 10) 
    break; 
string objectlength = dectohex((col-10)/2,0);  

if(objectlength.size() == 1) 
    padd(objectlength,'0' ,2,1); 
// Setting length of text record.
textrecord[9] = objectlength[0]; 
textrecord[10] = objectlength[1]; 
textrecord.pop_back();
outfile << textrecord << "\n"; 
if(flag == 0) 
    break;
} 
string endrecord = "E";  
endrecord.push_back('^');
string hexver = dectohex(start); 
padd(hexver,'0',6,1); 
for(auto &it: hexver) 
    endrecord.push_back(it); 
outfile << endrecord <<"\n"; 
loutput.close();
inpfile.close(); 
outfile.close();
return 0;
}

int pass1(ifstream &inpfile,ofstream &interim, int* sa = 0 , int *size = 0,int *start_present = 0) 
{  
string temporary; 
vector<string> array; 
int locctr = 0; 
 
int flag;
/// SPECIAL PARSING REQUIRED FOR FIRST LINE 

getline(inpfile , temporary); 
parse_line(temporary,array); 
/*int entry_present = 0;
if(array[1] == "START") 
    {   *start_present = 1;
        *sa = hextodec(array[2]); 
        interim << temporary<<"\n"; 
        locctr = *sa;
    }
else 
    {entry_present = 1;
        locctr = 0; 
        *sa = 0;
    }
array.clear(); 
 
int count0 = 0; 
while(inpfile) 
{  
if(entry_present == 0) 
    getline(inpfile,temporary);
flag= parse_line(temporary,array); 

entry_present  = 0;
if(flag == 0) 
    {array.clear();
    continue;}    
instruction_to_address.push_back(locctr); 
if(array[1] == "END") 
    {   interim << temporary<<"\n"; 
        break;}
if(array[0] == " ");
else 
    {
        if(present_sym[array[0]] == 1) 
            {cout<<"symbol "<<array[0]<<" already exists\n"; 
            _Exit(0);} 
        else 
            {present_sym[array[0]] = 1; 
            symtab[array[0]] = locctr;}
    }   
if (present_op[array[1]] == 1) 
    { 
        locctr+=3;
    } 
else if (array[1] == "WORD") 
    { 
        locctr+=3;
    } 
else if (array[1] == "RESW") 
    { 
        locctr+= 3*stoi(array[2]);
    } 
else if(array[1] == "RESB") 
    {
        locctr += stoi(array[2]);
    } 
else if(array[1] == "BYTE") 
    {  
        locctr += detsize(array[2]);
    } 
else 
    { 
        cout<<"error opcode not found!!! "<<array[1]<<"\n";
        _Exit(0);
    } 
array.clear(); 
count0++; 
interim << temporary <<"\n";
}  
interim.close(); 
inpfile.close(); 
*size = (locctr - *sa); 
*/ 
int block_num = 0;
int specsig=0;
while(inpfile) 
{   map<string,int> p_sym,sym_temp; 
   
    block_names.push_back(array[0]);  
    interim << temporary <<"\n"; 
    array.clear();  
    int fref=0,fdef=0;
    while(inpfile) 
        {  

            getline(inpfile,temporary);
         flag=   parse_line(temporary,array); 

            //ntry_present  = 0;
        if(flag == 0) 
            {   
                array.clear();
            continue;}     
        if(array[0] == "CSECT") 
        { 
            break;
        }
        instruction_to_address_mul[block_num].push_back(locctr); 
        if(array[1] == "END") 
            {   specsig = 1;
                interim << temporary<<"\n"; 
                break;} 
        
        if(array[0] == " ");
        else 
            {
                if(p_sym[array[0]] == 1) 
                    {cout<<"symbol "<<array[0]<<" already exists\n"; 
                    _Exit(0);} 
                else 
                    {p_sym[array[0]] = 1; 
                    sym_temp[array[0]] = locctr;}
            } 
        if(array[1][0] == '=') 
        { 
            p_sym[array[1]] = 1; 
            sym_temp[array[1]] = locctr;  
            array.clear();  
            locctr +=3;
            interim << temporary<<"\n";
            continue;
        }   
        int inlen = 3;
        if(array[1][0] == '+') 
        { 
            array[1] = array[1].substr(1,array[1].size() - 1); 
            inlen=4;
        } 
        if((array[1] == "TIXR") || (array[1] == "CLEAR")) 
        { 
            inlen=2;
        } 
        if((array[1] == "EXTREF") || (array[1] == "EXTDEF")) 
        { int sz_temp = array[2].size();
          int i=0,j; 
          vector<string> ref_def; 
          while(i<sz_temp) 
          { 
              j=i; 
              string temp_def_ref; 
              while((j<sz_temp) &&(array[2][j] != ',')) 
              { temp_def_ref.push_back(array[2][j]); 
                j++;

              } 
              ref_def.push_back(temp_def_ref); 
              i=j+1; 
          }  
            if(array[1] == "EXTREF") 
            {   fref = 1;
                block_references.push_back(ref_def);
            } 
            else 
            {   fdef=1;
                block_definitions.push_back(ref_def);
            } 
            inlen=0; 
            array.clear(); 
            interim << temporary<<"\n";
        }
        if(array[1] == "LTORG") 
        { 
            array.clear(); 
            interim<< temporary << "\n"; 
            continue;
        }

        if (present_op[array[1]] == 1) 
            { 
                locctr+=inlen;
            } 
        else if (array[1] == "WORD") 
            { 
                locctr+=3;
            } 
        else if (array[1] == "RESW") 
            { 
                locctr+= 3*stoi(array[2]);
            } 
        else if(array[1] == "RESB") 
            {
                locctr += stoi(array[2]);
            } 
        else if(array[1] == "BYTE") 
            {  
                locctr += detsize(array[2]);
            } 
        else 
            { 
                cout<<"error opcode not found!!! "<<array[1]<<"\n";
                _Exit(0);
            } 
        array.clear(); 
        //count0++; 
        
        interim << temporary <<"\n";
        }     
    vector<string> empty;
    if(fref == 0) 
    { 
        block_references.push_back(empty);
    } 
    if(fdef == 0) 
    { 
        block_definitions.push_back(empty);
    }
    block_sizes.push_back(locctr); 
    present_sym_mul.push_back(p_sym); 
    sym_tab_mul.push_back(sym_temp);
    if(specsig == 1) 
        break;
    block_num++;
}

//return (locctr - *sa); 
return 1;
}



int main(int argc , char *argv[]) { 
if(argc < 2) 
{ 
    printf("input file name not specified\n"); 
    _Exit(0);
}

ifstream inpfile; 
inpfile.open(argv[1]); 
ofstream interim; 
interim.open("intermediate.txt"); 
int sa = 0, size =0,start_present = 0 ; 
mapping_register(); 
setup_optab();
pass1(inpfile,interim,&sa,&size,&start_present);  
int sz = block_names.size(); 
for(int i=0;i<sz;++i)  
{ 
    cout<<block_names[i]<<" "<<block_sizes[i]<<"\n"; 
    for(auto &it:instruction_to_address_mul[i]) 
    { 
        cout<<it<<"\n";
    }
}
/*convert();
ifstream intread("intermediate.txt");  
ofstream aoutput("output.txt");  
ofstream loutput("listing.txt");
pass2(intread,aoutput,loutput,sa,size,start_present);
*/
}

