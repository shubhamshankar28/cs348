%{ 
#include<bits/stdc++.h> 
#include "hashing.cpp"
using namespace std; 



int lineno=1; 
int linestatus=1;
int yyerror(char *); 
void newline(void); 
void OutputToken(int, string);
ofstream output; 
HashTable identifier_table("identifier.txt"); 
HashTable num_constant_table("num_constant.txt");
void install_id(string num); 
void install_num(string id);


%}
identifiers   [A-Z][A-Z0-9]* 
numbers       [0-9]+
%%
[ \t]           {}
"\n"            {newline();}
"PROGRAM"       {OutputToken(1," ");} 
"VAR"           {OutputToken(2," ");} 
"BEGIN"         {OutputToken(3," ");} 
"END"           {OutputToken(4," ");} 
"END."          {OutputToken(5," ");} 
"INTEGER"       {OutputToken(6," ");} 
"FOR"           {OutputToken(7," ");} 
"READ"          {OutputToken(8," ");} 
"WRITE"         {OutputToken(9," ");} 
"TO"            {OutputToken(10," ");} 
"DO"            {OutputToken(11," ");} 
";"             {OutputToken(12," ");} 
":"             {OutputToken(13," ");} 
","             {OutputToken(14," ");} 
":="            {OutputToken(15," ");} 
"+"             {OutputToken(16, " ");} 
"-"             {OutputToken(17," ");} 
"*"             {OutputToken(18," ");} 
"DIV"           {OutputToken(19," ");} 
"("             {OutputToken(20," ");} 
")"             {OutputToken(21," ");}
{numbers}       {
                 string num = yytext;
                 OutputToken(23,num); 
                 install_num(num); 
                }
{identifiers}   { 
                string id = yytext;
                OutputToken(22,id); 
                install_id(id); 
                }


 
%%
int main() 
{ 
output.open("output.txt"); 
yylex(); 
output.close(); 
identifier_table.print(); 
num_constant_table.print(); 
cout<<"Output in output.txt\n"; 
cout<<"Identifiers in identifier.txt\n"; 
cout<<"Numericals in num_constant.txt\n";
}  
void newline() 
{ 
linestatus=1;
lineno++;
}
void OutputToken(int tokenid,string name) 
{ 
if(linestatus == 1) 
{ 
output<<lineno; 
linestatus=0;
} 
else 
{ 
output<<" ";
}  

output<<"    "<<tokenid<<"    "; 
if(tokenid == 23) 
{
output<<"#";
} 
if(tokenid == 22) 
output<<"^"; 
output<<name<<"\n";
}


void install_num(string num) 
{ 
num_constant_table.insert(num);
} 

void install_id(string id) { 
identifier_table.insert(id);
}  

int yywrap(void){ 
return 1; 
}
