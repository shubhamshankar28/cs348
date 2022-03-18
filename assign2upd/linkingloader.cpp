#include<bits/stdc++.h> 
using namespace std;   
map<string,int> estab;
/* Pads the character c to the input so that it's size becomes tlen
if start = 1 c is padded to the start of the input
if start = 0 c is padded to the end of the input
*/
void padd(string &input, char c, int tlen, int start = 0)
{
    int n = input.size();
    if (n >= tlen)
        return;
    if (start == 1)
        reverse(input.begin(), input.end());
    for (int i = n; i < tlen; ++i)
    {
        input.push_back(c);
    }
    if (start == 1)
        reverse(input.begin(), input.end());
}
// Converts a hexadecimal into decimal
int hextodec(string &s1)
{
    int ans = 0;
    int n = s1.size();
    for (int i = 0; i < n; ++i)
    {
        ans = ans * 16 + ((s1[i] <= '9') ? (s1[i] - '0') : (s1[i] - 'A' + 10));
    }
    return ans;
}

/* Converts a decimal into hex format
In mode 1 which is the default mode the string is padded to make its length 4. This mode should be used for addresses
Otherwise the string is returned as it is.

*/
string dectohex(int n, int mode = 1)
{
    if (n == 0)
    {
        if (mode == 1)
            return "0000";
        else
            return "0";
    }
    string s1;
    int dec;
    while (n != 0)
    {
        dec = (n % 16);
        if (dec <= 9)
        {
            s1.push_back((char)(dec + '0'));
        }
        else
        {
            s1.push_back((char)(dec - 10 + 'A'));
        }
        n = n / 16;
    }
    reverse(s1.begin(), s1.end());
    if (mode == 1)
        padd(s1, '0', 4, 1);

    return s1;
} 

void parse_definition_record_pass1(string &temp,int csaddr) 
{ 
    int sz=temp.size(); 
    string variable_name,variable_value; 
    for(int i=1;i<sz;++i) 
    { 
        if(temp[i] == '^') 
        { 
            variable_name = temp.substr(i+1,6); 
            if(estab[variable_name] > 0) 
            { 
                cout<<"Fatal Error\n"; 
                _Exit(0);
            } 
            else 
            { 
                variable_value = temp.substr(i+8,6); 
                estab[variable_name]  = csaddr + hextodec(variable_value) + 1;
             //   cout<<"in pass 1 parse definition record :"<<variable_name<<":"<<"\n"; 
             //   cout<<"value is "<<estab[variable_name]<<"\n";
            i = i+13;
            }
        } 
        else 
        { 
           variable_name = temp.substr(i,6); 
            if(estab[variable_name] > 0) 
            { 
                cout<<"Fatal Error\n"; 
                _Exit(0);
            } 
            else 
            { 
                variable_value = temp.substr(i+6,6); 
                estab[variable_name]  = hextodec(variable_value) + 1;
            i = i+11;
            } 
        }
    }
}
void pass1(char *file ,int progaddr = 0) 
{ 

    int csaddr = progaddr; 
    ifstream inpfile; 
    inpfile.open(file); 
    string temp,starting_address;
    while(inpfile) 
    {   getline(inpfile,temp); 
        //cout<<temp<<"\n";
        int size = temp.size();  
       // cout<<size<<"\n"; 
        if(size == 0) 
        {  // cout<<"inserting a new line \n";
            continue;} 
        string control_secname = temp.substr(2,6); 
        if(estab[control_secname] != 0) 
            { 
                cout<<"duplicate control section name\n"; 
                _Exit(0);
            } 
        else 
            { 
                estab[control_secname] = csaddr+1;
            }

        starting_address = temp.substr(size-6,6);
        int cslth =  hextodec(starting_address); 
        while(inpfile) 
        {   
            getline(inpfile,temp); 
            if(temp[0] == 'E') 
                break; 
            if(temp[0] == 'D') 
            { 
                parse_definition_record_pass1(temp,csaddr);
            }
        } 
        csaddr +=cslth; 

    } 
    inpfile.close();
}

void parse_text_record(string &temp,int csaddr,int &curbase,vector<int> &addr,vector<string> &values) 
{  // cout<<"parsing text record\n"; 
  // cout<<csaddr<<" "<<temp<<"\n"; 
    int sz=temp.size(); 
    int j;  
    string beginning_address = temp.substr(2,6);
    csaddr += (hextodec(beginning_address));
    int len;
    int length =addr.size();
    // cout<<"starting "<<csaddr<<"\n";
    for(int i=12;i<sz;++i) 
    { 
        j=i;  
         string objectcode;
        while((j<sz) && (temp[j] != '^')) 
            {objectcode.push_back(temp[j]); 
            j++;} 
     len = j-i; 
    // cout<<" writing record at "<<objectcode<<" "<<curbase<<"  "<<csaddr<<" "<<(csaddr/16)*16<<"\n";
     for(int k=0;k<len;k+=2) 
     { 
         if((csaddr/16)*16 != (curbase)) 
         {   
             string value(32,'.'); 
             values.push_back(value); 
             curbase = (csaddr/16)*16; 
            // cout<<"pushing  "<<curbase<<" in \n"; 
             addr.push_back(curbase); 
             length++;
         } 
         values[length-1][2*(csaddr%16)] = objectcode[k]; 
         values[length -1][2*(csaddr%16) + 1] = objectcode[k+1];
      csaddr++;
      }  
    //  cout<<"currently written record at " << length <<" is "<<values[length-1]<<"\n \n";
      i=j;
    }
//cout<<values[0]<<"\n";
}

void parse_modify_record(string &temp,int csaddr,int &curbase,vector<int> &addr,vector<string> &values) 
 { // cout<<"parsing modification record\n"; 
// cout<<"\n"; 
// cout<<"\n";
    int sz=temp.size(); 
    int j;    
    int specaddr;
    string beginning_address = temp.substr(2,6);
    specaddr = (hextodec(beginning_address));
    specaddr+=csaddr;
    int base = (specaddr/16)*16;  
    int initposition;
    int position = (lower_bound(addr.begin(),addr.end(),base) - addr.begin());
    //int stringstart = (specaddr%16)*2; 
    initposition = position; 
    int lengthofmodval = stoi(temp.substr(9,2));  
    int flag = (lengthofmodval%2);
    lengthofmodval = (lengthofmodval+1)/2;
    
    string addend; 
    int ep;
    for(int i=0;i<lengthofmodval;++i) 
    { 
        if(((specaddr+i)/16)*16 != base) 
        {   base =((specaddr+i)/16)*16;
            position++;
        }  
        ep  = ((specaddr+i)%16)*2; 
        addend.push_back(values[position][ep]); 
        addend.push_back(values[position][ep+1]);
    }

    if(flag == 1) 
    { 
        reverse(addend.begin(),addend.end());
        addend.pop_back(); 
        reverse(addend.begin(),addend.end());
    } 
    int finsz = addend.size();


    int hexnum = hextodec(addend);  
    int sign = 1; 
    if(temp[12] == '-') 
        sign=-1;  
    string hexver;
    hexver = temp.substr(13,sz-13);  
    padd(hexver,' ',6);
    //cout<<":"<<hexver<<": :"<<estab[hexver]<<"\n";
    
    if(estab[hexver] != 0)
    hexnum += (sign*(estab[hexver]-1));
    else 
    {   
        //cout<<"symbol not present in estab\n"; 
        _Exit(0);
    }  
    if(hexnum < 0) 
    { 
        // cout<<"alert hexnum is now less than zero "<<hexnum<<"\n"; 
        // cout<<hexnum<<"\n";
        hexnum += (1 << (4*(finsz)));    
    } 
    string replacedstring = dectohex(hexnum,0); 
    padd(replacedstring,'0',finsz+flag,1);  
    int sizes = replacedstring.size();  
    string original = replacedstring;
    replacedstring = replacedstring.substr(sizes-finsz - flag,finsz+flag); 

    //cout<<"the actual value is "<<hexnum<<" "<<"string is "<<replacedstring<<" "<<original<<"\n";
    base = (specaddr/16)*16; 
    position = initposition;
    for(int i=0;i<lengthofmodval;i++) 
    { 
        if(((specaddr+i)/16)*16 != base) 
        {   base =((specaddr+i)/16)*16;
            position++;
        }  
        ep  = ((specaddr+i)%16)*2;  
        if(!(flag==1 &&(i==0)))
            values[position][ep] = replacedstring[2*i]; 
        values[position][ep+1] = replacedstring[2*i+1];
    }

// cout<<"\n"; 
// cout<<"\n";
}


void pass2(char *file,int progaddr = 0) 
{ 

int csaddr = progaddr; 
int execaddr = progaddr;
ifstream inpfile; 
ofstream outfile;  
inpfile.open(file); 
vector<int> addr; 
vector<string> values; 
string starting_address;
int cslth;

string temp;
int curbase =-1;
while(inpfile)
{ getline(inpfile,temp);  

 int size = temp.size(); 
 if(size == 0) 
 { 
     //cout<<"newline spotted\n"; 
     continue;
 }
starting_address = temp.substr(size-6,6);
cslth =  hextodec(starting_address); 
//cout<<"header "<<temp<<"    "<<cslth<<"\n";
while(inpfile) 
{ 
getline(inpfile,temp); 
//cout<<"record is "<<temp<<"\n";  
if(temp[0] == 'E')
break;
if(temp[0] == 'T') 
{  

parse_text_record(temp,csaddr,curbase,addr,values); 

} 
else if ( temp[0] == 'M') 
{ 

parse_modify_record(temp,csaddr,curbase,addr,values);
}

} 
if(temp.size()>1) 
{ 
    size = temp.size(); 
    starting_address = temp.substr(size-6,6);
    execaddr = hextodec(starting_address);
} 
csaddr += cslth;
//cout<<"terminating csaddr "<<csaddr<<"\n";
}  
outfile.open("output_final.txt");
 int tlen = addr.size();  
// cout<<"length is "<<tlen<<"\n"; 
// for(auto it: addr) 
// { 
//     cout<<it <<" ";
// }  
// cout<<"\n";
// for(int i=0;i<tlen;++i) 
// { 
//     cout<<values[i]<<"\n";
// }
for(int i=0;i<tlen;++i) 
{ 
    for(auto &it:dectohex(addr[i])) 
        outfile<<it;
    outfile<<" "; 
    for(int j=0;j<32;++j) 
    { 
        outfile<<values[i][j]; 
        if((j%8) == 7) 
        outfile<<" ";
    } 
    outfile<<"\n"; 
if((i+1) == (tlen)) 
continue; 
for(int k=0;k<(addr[i+1]-addr[i]-1)/16;++k) 
{ 
for(auto &it:dectohex(k*16+16 + addr[i])) 
        outfile<<it;
    outfile<<" "; 
    for(int j=0;j<32;++j) 
    { 
        outfile<<"."; 
        if((j%8) == 7) 
        outfile<<" ";
    } 
    outfile<<"\n"; 
} 

}

outfile.close();
inpfile.close();
}


int main(int argc,char *argv[]) 
{
if(argc < 3) 
{ 
    cout<<"Number of parameters are too less\n";
    _Exit(0);
}   
//cout << "entering pass 1\n";
pass1(argv[1],stoi(argv[2]));  
//cout << "exiting pass 1\n"; 
//cout <<" entering pass 2\n";
pass2(argv[1],stoi(argv[2])); 
//cout<<" exiting pass 2\n";

}