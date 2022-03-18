/****
Code should be compiled and tested in the linux g++ environment
****/

#include <bits/stdc++.h>
using namespace std;
map<string, string> optab;
map<string, int> symtab;
// checks whether an opcode is present or not
map<string, int> present_op;
// checks whether a symbol is present or not
map<string, int> present_sym;
// register mapping
map<string, string> registers;
/*
We must store  information such as block names and block sizes for each control section.
In particular we would want the external references and definition in each block,
along with the names of each section and their sizes. Also store the addresses
for the definitions. 

*/
vector<string> block_names;
vector<int> block_sizes;
vector<vector<string>> block_definitions;
vector<vector<string>> block_references;
vector<vector<pair<int, int>>> instruction_to_address_mul;
vector<vector<int>> instruction_to_address_mulhex;
vector<map<string, int>> present_sym_mul;
vector<map<string, pair<int, int>>> sym_tab_mul;
vector<vector<int>> block_information;



struct modification_record
{
    vector<string> expression;
    string modval;
    int length;
    int disp;
    int type;
    modification_record(int l, int displacement, string &mod)
    {
        length = l;
        disp = displacement;
        modval = mod;
        type = 0;
    }
    modification_record(int l, int displacement, vector<string> mod)
    {
        length = l;
        disp = displacement;
        expression = mod;
        type = 1;
    }
    modification_record()
    {
        type = 0;
    }
};

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
void Set(const string &s1, const string &code)
{
    optab[s1] = code;
    present_op[s1] = 1;
}

// Routine to fill the optable
void setup_optab()
{

    Set("LDA", "00");
    Set("LDX", "04");
    Set("LDL", "08");
    Set("LDCH", "50");
    Set("STA", "0C");
    Set("STX", "10");
    Set("STL", "14");
    Set("STCH", "54");
    Set("SUB", "1C");
    Set("ADD", "18");
    Set("MUL", "20");
    Set("DIV", "24");
    Set("COMP", "28");
    Set("J", "3C");
    Set("JLT", "38");
    Set("JEQ", "30");
    Set("JSUB", "48");
    Set("JGT", "34");
    Set("RSUB", "4C");
    Set("TIX", "2C");
    Set("TD", "E0");
    Set("RD", "D8");
    Set("WD", "DC");
    Set("TIXR", "B8");
    Set("CLEAR", "B4");
    Set("COMPR", "A0");
    Set("LDT", "74");
}
// These vectors are used for resolving references
vector<int> instruction_to_address;
vector<string> instruction_to_address_hex;

/* decomposes a line of input into label , opcode and operand
It returns 0 to indicate that the current line in the assembly
code corresponds to a comment
*/ 


int parse_line(string &s1, vector<string> &v1)
{
    if (s1[0] == '.')
        return 0;
    int length = s1.size();
    for (int i = 0; i < length; ++i)
    {
        int index = i;
        string temp;
        while (index < length && s1[index] != ' ')
        {
            temp.push_back(s1[index]);
            index++;
        }
        if (temp.size() > 0)
            v1.push_back(temp);
        else
        {
            v1.push_back(" ");
        }
        i = max(index, i + 1);
    }
    if (v1.size() < 3)
        v1.push_back(" ");
    return 1;
}

/* Pads the character c to the input so that it's size becomes tlen
if start = 1 c is padded to the start of the input
if start = 0 c is padded to the end of the input
*/
void padd(string &input, char c, int tlen, int start = 0)
{
    int n = input.size();
    if (n == tlen)
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

void generate_listing(ofstream &listing ,string &inp , string &assembly, int addr,int flag1 = 1) 
{   string address= dectohex(addr);
    padd(address,'0',4,1); 
    string final; 
    for(auto it:address) 
        final.push_back(it); 
    final.push_back(' '); 
    for(auto it:inp) 
    final.push_back(it); 
    final.push_back(' '); 
    for(auto it:assembly) 
    final.push_back(it);    
    if(flag1 == 0) 
    { 
        for (int i=0;i<4;++i) 
            final[i] = ' ';
    }
    listing << final <<"\n";


}

// get the address of a label given the block number
int getaddress(string &label, int blocknum, int st = -1)
{
    if (st == -1)
        return (block_information[blocknum][sym_tab_mul[blocknum][label].second] + sym_tab_mul[blocknum][label].first);
    else
        return (block_information[blocknum][instruction_to_address_mul[blocknum][st].second] + instruction_to_address_mul[blocknum][st].first);
}
// All the addresses initially in decimal are converted into hexadecimal
void convert()
{
    for (auto &it : instruction_to_address)
    {
        instruction_to_address_hex.push_back(dectohex(it));
    }
}

// Returns the size of the constant in bytes
int detsize(string &const1)
{
    if (const1[0] == 'C')
        return (const1.size() - 3);
    else
        return (const1.size() - 3) / 2;
}
/* Converts constant into hexadecimal form
 */
string convert_const(string &inp)
{
    string result;
    int n = inp.size();
    if (inp[0] == 'X')
    {
        for (int i = 2; i < n - 1; ++i)
            result.push_back(inp[i]);
    }
    else
    {
        for (int i = 2; i < n - 1; ++i)
        {
            string asciicode = dectohex((int)inp[i], 0);
            if (asciicode.size() == 1)
                padd(asciicode, '0', 2, 1);
            for (auto &it : asciicode)
                result.push_back(it);
        }
    }
    return result;
}
//checks whether the given string is an exprssion or not
int isexpression(string &s1)
{
    for (auto &it : s1)
    {
        if ((it == '-') || (it == '+'))
            return 1;
    }
    return 0;
}
// Seperates the operators and operands in an expression
vector<string> expression_parsing(string &s1)
{ // cout<<"in there\n";
    vector<string> answer;
    answer.push_back("+");
    int len = s1.size();
    for (int i = 0; i < len; ++i)
    {
        string temporary;
        int j = i;
        while ((j < len) && (s1[j] != '+') && (s1[j] != '-'))
        {
            temporary.push_back(s1[j]);
            j++;
        }
        answer.push_back(temporary);
        if (j < len)
        {
            string s2;
            s2.push_back(s1[j]);
            answer.push_back(s2);
        }
        i = j;
    }
    // cout<<"out of there\n";
    return answer;
} 
//Parses indexed operands
string indexedoperand(string &s1, int &p)
{
    int n = s1.size();
    string temp;
    int i = 0;
    while (i < n)
    {
        if (s1[i] == ',')
        {
            p = 1;
            break;
        }
        temp.push_back(s1[i]);
        i++;
    }
    return temp;
} 
// Parses the an sicxe instruction
string sicxe_instruction_parse(ofstream &lstream,string &inp ,vector<string> &array, map<string, pair<int,int>> &sym_tab, map<string, int> &p_tab, int & pcval,int blocknum, int *modif, modification_record &m1, int *flag,int genlist = 0, int st =0)
{string blank = " ";
    if ((array[1] == "CLEAR") || (array[1] == "COMPR") || (array[1] == "TIXR"))
    {
        string inst = optab[array[1]];
        if (array[1] == "COMPR")
        {
            // string s2;
            string s2;
            string s3;
            s2.push_back(array[2][0]);
            s3.push_back(array[2][2]);
            inst.push_back(registers[s2][0]);
            inst.push_back(registers[s3][0]); 
            if(genlist == 1)
                generate_listing(lstream,inp,inst,instruction_to_address_mul[blocknum][st].first);
            return inst;
        }
        else
        {
            string s2;
            s2.push_back(array[2][0]);
            inst.push_back(registers[s2][0]);
            inst.push_back('0'); 
            if(genlist == 1)
                {   
                        generate_listing(lstream,inp,inst,instruction_to_address_mul[blocknum][st].first); 
                     
                }
            return inst;
        }
    }  
    string decx = " ?"; 
   // cout<<array[0]<<" "<<array[1]<<" "<<array[2]<<"\n";
    if ((array[1] == "WORD") || (array[1] == "BYTE") || (array[1] == "RESW") || (array[1] == "RESB"))
    {
        if ((array[1] == "RESW") || (array[1] == "RESB"))
        {   
            *flag = 1; 
            if(genlist == 1) 
                        { 
                            generate_listing(lstream,inp,blank,instruction_to_address_mul[blocknum][st].first);
                        }
            return " ";
        }
        else
        {
            if (array[1] == "WORD")
            {    //cout<<"entered word\n";
                *modif = 1;
                if (isexpression(array[2]))
                {
                    m1 = modification_record(6, 0, expression_parsing(array[2]));
                    //cout << "important   " << m1.type;
                }
                else
                {
                    m1 = modification_record(6, 0, array[2]);
                } 
                string fininst ; 
                if(isexpression(array[2])) 
                { 
                    fininst = "000000";
                }  
                else 
                { 
                    fininst= convert_const(array[2]);
                }
                padd(fininst,'0' , 6,1); 
                if(genlist == 1) 
                        { 
                            generate_listing(lstream,inp,fininst,instruction_to_address_mul[blocknum][st].first);
                        } 
                //cout<<fininst<<"\n";
                return fininst;
            }
            else
            {   //cout<<"entered word else\n";  

                string fininst = convert_const(array[2]); 
                 if(genlist == 1) 
                        { 
                            generate_listing(lstream,inp,fininst,instruction_to_address_mul[blocknum][st].first);
                        }  
                //cout<<fininst<<"\n";
                return fininst;
            }
        }
    }
    if (array[1][0] == '=')
    {
        string temp = array[1].substr(1, array[1].size() - 1);
        string value = convert_const(temp);
        if(genlist == 1) 
            { 
                generate_listing(lstream,inp,value,instruction_to_address_mul[blocknum][st].first);
            }
        return value; 

    }
    int highmode = 0;
    if (array[1][0] == '+')
    {
        array[1] = array[1].substr(1, array[1].size() - 1);
        highmode = 1;
    }
    vector<int> inst_bits;
    if (highmode == 0)
        inst_bits = vector<int>(24, 0);
    else
        inst_bits = vector<int>(32, 0);
    string code = optab[array[1]];
    int dec;
    for (int i = 0; i < 2; ++i)
    {
        dec = ((((code[i] - '0') >= 0) && ((code[i] - '0') <= 9)) ? (code[i] - '0') : (code[i] - 'A' + 10));
        for (int j = 3; j > -1; --j)
        {
            if ((dec % 2) == 1)
            {
                inst_bits[4 * i + j] = 1;
            }
            dec = dec / 2;
        }
    }
    if (array[2][0] == '@')
    {
        inst_bits[6] = 1;
        inst_bits[7] = 0;
        array[2] = array[2].substr(1, array[2].size() - 1);
        int value = 0, indexing = 0;
        array[2] = indexedoperand(array[2], indexing);
        if (p_tab[array[2]] == 1)
        {   
            if (highmode == 0)
                value = getaddress(array[2],blocknum) - (pcval);
            else
                value = getaddress(array[2],blocknum);
        }
        else
        {
            *modif = 1;

            value = 0;
            if (isexpression(array[2]))
                m1 = modification_record((highmode == 0) ? 3 : 5, 3, array[2]);
            else
                m1 = modification_record((highmode == 0) ? 3 : 5, 3, array[2]);
        }
        if (highmode == 0)
        {
            if (value >= -2048 && value < 2048)
            {
                if (value < 0)
                    value += (1 << 12);
            }
        }
        inst_bits[8] = indexing;
        for (int j = ((highmode == 0) ? 23 : 31); j >= 12; --j)
        {
            inst_bits[j] = value % 2;
            value = value / 2;
        }
        if (highmode == 0)
            inst_bits[10] = 1;
        else
            inst_bits[11] = 1;
    }
    else if (array[2][0] == '#')
    {
        inst_bits[6] = 0;
        inst_bits[7] = 1;
        array[2] = array[2].substr(1, array[2].size() - 1);

        int value = stoi(array[2]);
        for (int j = ((highmode == 0) ? 23 : 32); j >= 12; --j)
        {
            inst_bits[j] = value % 2;
            value = value / 2;
        }
    }
    else
    {
        inst_bits[6] = 1;
        inst_bits[7] = 1;
        int indexing = 0;
        if (array[2] != " ")
        {
            array[2] = indexedoperand(array[2], indexing);
            inst_bits[8] = indexing;
            int value;
            if (p_tab[array[2]] == 1)
            {  // cout<<"the value is "<<array[2]<<" "<<getaddress(array[2],blocknum)<<" "<<pcval<<"\n";
                value = getaddress(array[2],blocknum) - (pcval);
            }
            else
            {
                *modif = 1;

                value = 0;
                if (isexpression(array[2]))
                    m1 = modification_record((highmode == 0) ? 3 : 5, 3, array[2]);
                else
                    m1 = modification_record((highmode == 0) ? 3 : 5, 3, array[2]);
            }
            //  cout<<array[1]<<" "<<pcval<<" "<<sym_tab[array[2]]<<" "<<value<<"\n";
            if (value >= -2048 && value < 2048)
            {
                if (value < 0)
                    value += (1 << 12);
            }
            for (int j = ((highmode == 0) ? 23 : 31); j >= 12; --j)
            {
                inst_bits[j] = value % 2;
                value = value / 2;
            }
            if (highmode == 0)
                inst_bits[10] = 1;
            else
                inst_bits[11] = 1;
        }
        else
        {
            if (highmode == 1)
                inst_bits[11] = 1;
        }
    }
    string fininstruction;
    for (int i = 0; i < ((highmode == 0) ? 24 : 32); i += 4)
    {
        int val = 1, s = 0;
        for (int j = i + 3; j >= i; --j)
        {
            s += inst_bits[j] * val;
            val = val * 2;
        }
        fininstruction.push_back((s < 10) ? (s + '0') : (s - 10 + 'A'));
    }
    if(genlist == 1) 
        { 
            generate_listing(lstream,inp,fininstruction,instruction_to_address_mul[blocknum][st].first);
        }
    return fininstruction;
}

string create_modif(int start_address, int length, string externsym, int flag = 1)
{
    string answer = "M^";
    string addrs = dectohex(start_address);
    padd(addrs, '0', 6, 1);
    for (auto &it : addrs)
        answer.push_back(it);
    answer.push_back('^');
    string length1 = dectohex(length);
    padd(length1, '0', 2, 1);
    int sz = length1.size();
    answer.push_back(length1[sz - 2]);
    answer.push_back(length1[sz - 1]);
    answer.push_back('^');
    answer.push_back((flag == 1) ? '+' : '-');
    for (auto &chr : externsym)
        answer.push_back(chr);
    return answer;
}



void sicxepass2(ifstream &inpfile, ofstream &outfile, ofstream &listing, int blocknum)
{
    
    string headerinstruction = "H^";
    vector<string> modificationrecords;

    //cout << "in sicxepass2\n";
    string temp;
    vector<string> array;
    getline(inpfile, temp);
    int st = 0;
    parse_line(temp, array);
    padd(array[0], ' ', 6, 0);
    for (auto &it : array[0])
    {
        headerinstruction.push_back(it);
    }
    headerinstruction.push_back('^');
    for (int i = 0; i < 6; ++i)
    {
        headerinstruction.push_back('0');
    }
    headerinstruction.push_back('^');
    string temp4 = dectohex(block_sizes[blocknum]);
    padd(temp4, '0', 6, 1);
    for (auto &it : temp4)
        headerinstruction.push_back(it);
    string blank = " ";
    outfile << headerinstruction << "\n";
    generate_listing(listing,temp,blank,instruction_to_address_mul[blocknum][st].first);
    array.clear();

    //cout << temp << "\n";
    st++;
    if (block_definitions[blocknum].size() != 0)
    {
        string drecords = "D^";
      //  cout << "definitions\n";
        int rc = 0;
        string temp, copy;
        for (auto &it : block_definitions[blocknum])
        {
            if (rc == 5)
            {
                drecords.pop_back();
                outfile << drecords << "\n";
                rc = 0;
                drecords = "D^";
            }
            copy = it;
            padd(copy, ' ', 6, 0);
            for (auto &chr : copy)
                drecords.push_back(chr);
            drecords.push_back('^');
            temp = dectohex(sym_tab_mul[blocknum][it].first + block_information[blocknum][sym_tab_mul[blocknum][it].second]);
            padd(temp, '0', 6, 1);
            for (auto &chr : temp)
                drecords.push_back(chr);
            drecords.push_back('^');
            rc++;
        }
        if (rc > 0)
        {
            drecords.pop_back();
            outfile << drecords << "\n";
        }
        getline(inpfile, temp);
        generate_listing(listing,temp,blank,instruction_to_address_mul[blocknum][st].first,0);
        st++;
    }

    if (block_references[blocknum].size() != 0)
    {
        string drecords = "R^";
        //std::cout << "references\n";
        int rc = 0;
        string temp, copy;
        for (auto &it : block_references[blocknum])
        {
            if (rc == 11)
            {
                drecords.pop_back();
                outfile << drecords << "\n";
                rc = 0;
                drecords = "R^";
            }
            copy = it;
            padd(copy, ' ', 6, 0);
            for (auto &chr : copy)
                drecords.push_back(chr);
            drecords.push_back('^');
            rc++;
        }
        if (rc > 0)
        {
            drecords.pop_back();
            outfile << drecords << "\n";
        }
        getline(inpfile, temp);
        generate_listing(listing,temp,blank,instruction_to_address_mul[blocknum][st].first,0);
        st++;
    }

    int flag;
    string instruction;

    //cout << "start: " << blocknum << " " << st << "\n";
    int sz = instruction_to_address_mul[blocknum].size();

    int parsed_instruction_available = 0;
    string parsed_line;
    int length = 0, brokenonres = 0;
    string textrecord;
    string addr;
    int first_parsed_st = -1;
    int curblock = 0;
    string des = " "; 
    int genlist=  0; 
    //cout<<sz<<"\n";
    while (inpfile && (st+1 < sz))
    {
        if (length == 0)
        {
            // time to write a new text record !!!!
            textrecord = "T^";
            string addr = dectohex(getaddress(des, blocknum, st)); 
            //  cout<<"in block : "<<blocknum<<"\n"; 
            //  cout<<"st is : "<<st<<"\n"; 
            // cout<<"ra and ba are "<<instruction_to_address_mul[blocknum][st].first<<" "<<instruction_to_address_mul[blocknum][st].second<<"\n"; 
            // cout<< "starting is "<<block_information[blocknum][instruction_to_address_mul[blocknum][st].second]<<"\n";
            padd(addr, '0', 6, 1);
            for (auto &it : addr)
                textrecord.push_back(it);
            textrecord.push_back('^');
            textrecord.push_back('0');
            textrecord.push_back('0');
            textrecord.push_back('^');
            length = 9;
        }
        if (parsed_instruction_available == 0)
            {getline(inpfile, temp); 
            genlist = 1;}
        else
        {   genlist = 0;
            temp = parsed_line;
            parsed_instruction_available = 0;
        } 
        

        flag = parse_line(temp, array);
       // cout << "blocknum: " << blocknum << " " << temp << " " << st << " " << dectohex(getaddress(des, blocknum, st)) << "\n";
        if ((array[1] != "LTORG") && (array[1] != "EQU") && (array[1] != "END") && (array[1] != "USE"))
        {
            int modif = 0, lenmodif = 0, disp = 0, flag = 0;
            string modifstr;
            modification_record m2;
            assert((st + 1) < instruction_to_address_mul[blocknum].size()); 
            int pcvalue =  getaddress(des,blocknum,st+1);
            instruction = sicxe_instruction_parse(listing,temp,array, sym_tab_mul[blocknum], present_sym_mul[blocknum], pcvalue , blocknum,&modif, m2, &flag,genlist,st);

            // cout<<array[1]<<" "<<instruction<<"  modif: "<<modif<<" lemodif: "<<lenmodif<<" disp: "<<disp<<" flag: "<<flag<<"\n";
            if ((length + instruction.size() > 68) || ((flag == 1) && (brokenonres == 0)))
            {
                array.clear();
                if ((flag == 1) && (brokenonres == 0))
                {

                    brokenonres = 1;
                    st++;
                }
                else
                {
                    parsed_instruction_available = 1;
                    parsed_line = temp;
                }
                textrecord.pop_back();

                if (first_parsed_st != -1)
                {
                    addr = dectohex(getaddress(des, blocknum, first_parsed_st));
               //     cout << first_parsed_st << " " << addr << " " << length << "\n";
                    padd(addr, '0', 6, 1);
                    for (int i = 2; i <= 7; ++i)
                        textrecord[i] = addr[i - 2];
                }
                length = (length - 9) / 2;
                addr = dectohex(length);
            //    cout << addr << "\n";
                padd(addr, '0', 2, 1);
                for (int i = 9; i <= 10; ++i)
                {
                    textrecord[i] = addr[addr.size() - 1 - (10 - i)];
                }

                outfile << textrecord << "\n";

             //   cout << "breaking at " << instruction << " " << length << " " << st << "\n";
                length = 0;
                first_parsed_st = -1;

                continue;
            }
            if (flag == 1)
            {
                array.clear();
                //  cout<<st<<"\n";
                st++;
                continue;
            }
            if (first_parsed_st == -1)
            {
                first_parsed_st = st;
            }
            if (modif == 1)
            {
                // cout << array[2] << " " << m2.type << " "
                //      << "\n";
                if (m2.type == 0)
                    modificationrecords.push_back(create_modif(getaddress(des, blocknum, st) + m2.disp / 2, m2.length, m2.modval));
                else
                {
                    //cout << "in critical section\n";
                    int size = m2.expression.size();
                    for (int i = 0; i < size; i += 2)
                        modificationrecords.push_back(create_modif(getaddress(des, blocknum, st) + m2.disp / 2, m2.length, m2.expression[i + 1], ((m2.expression[i] == "-") ? 0 : 1)));
                }
            }
            brokenonres = 0;
            for (auto &it : instruction)
            {
                textrecord.push_back(it);
            }
            length += instruction.size();
            textrecord.push_back('^');
        } 
        else if (array[1] == "USE") 
        { 
            if(first_parsed_st != -1) {
                textrecord.pop_back();
                addr = dectohex(getaddress(des, blocknum, first_parsed_st));
                  //  cout << first_parsed_st << " " << addr << " " << length << "\n";
                    padd(addr, '0', 6, 1);
                    for (int i = 2; i <= 7; ++i)
                        textrecord[i] = addr[i - 2];
                
                length = (length - 9) / 2;
                addr = dectohex(length);
                //cout << addr << "\n";
                padd(addr, '0', 2, 1);
                for (int i = 9; i <= 10; ++i)
                {
                    textrecord[i] = addr[addr.size() - 1 - (10 - i)];
                }

                outfile << textrecord << "\n";

                //cout << "breaking at " << instruction << " " << length << " " << st << "\n";
                length = 0;
                first_parsed_st = -1; 
            }  
            generate_listing(listing,temp,blank,instruction_to_address_mul[blocknum][st].first,0);
            st++;
            array.clear();
                continue;
        }
        else
        {   if(array[1] != "EQU")
            generate_listing(listing,temp,blank,instruction_to_address_mul[blocknum][st].first,0);
            else 
            generate_listing(listing,temp,blank,sym_tab_mul[blocknum][array[0]].first);
            // cout << array[1] << "\n";  
            // cout <<"tem\n";
        }
        st++;
        array.clear();
    }
    if ((length - 9) > 0)
    {
        textrecord.pop_back();
        if (first_parsed_st != -1)
        {
            addr = dectohex(getaddress(des, blocknum, first_parsed_st));
                       
            padd(addr, '0', 6, 1);
            for (int i = 2; i <= 7; ++i)
                textrecord[i] = addr[i - 2];
        }
        length = (length - 9) / 2;
        addr = dectohex(length);
      //  cout << addr << "\n";
        padd(addr, '0', 2, 1);
        for (int i = 9; i <= 10; ++i)
        {
            textrecord[i] = addr[addr.size() - 1 - (10 - i)];
        }

        outfile << textrecord << "\n";
    }
    for (auto &it : modificationrecords)
        outfile << it << "\n";
    string endrecord;

    if (blocknum == 0)
        endrecord = "E^000000";
    else
        endrecord = "E";

    outfile << endrecord << "\n";
    for (int i = 0; i < 2; ++i)
        outfile << "\n";
    //cout << "end: " << blocknum << "\n";
}

void write_literals(vector<string> &literal_pool, ofstream &interim, int &locctr, int &cur_program_block, vector<pair<int,int>> &address, map<string, int> &p_sym, map<string, pair<int,int>> &sym_temp)
{
    //cout << "entered write literals\n";
    int sizet = literal_pool.size(); 

    for (int i = 0; i < sizet; ++i)
    {
        address.push_back({locctr,cur_program_block});
        string op1 = "* ";
        for (auto &it : literal_pool[i])
        {
            op1.push_back(it);
        }
        op1.push_back(' ');
        op1.push_back(' ');
        string t2 = literal_pool[i].substr(1, literal_pool[i].size() - 1);
        // string temp = convert_const(t2);
        // for(auto &it: temp)
        // {
        //     op1.push_back(it);
        // }
        p_sym[literal_pool[i]] = 1;
        sym_temp[literal_pool[i]] = {locctr,cur_program_block};
        interim << op1 << "\n";
      //  cout << "writing literal " << op1 << "\n";
        locctr += (detsize(t2));
    }
    literal_pool.clear();
}

int equexphandler(string &arr, int locctr, map<string, pair<int,int>> &sym_temp)
{
    int sign = 1;
    int sum = 0;
    int sz = arr.size();
    for (int i = 0; i < sz; ++i)
    {
        int j = i;
        string temp;
        while ((j < sz) && (arr[j] != '+') && (arr[j] != '-'))
        {
            temp.push_back(arr[j]);
            j++;
        }
        if (temp == "*")
        {
            sum += (sign * locctr);
        }
        else
        {
            sum += (sign * (sym_temp[temp].first));
        }

        if (arr[j] == '+')
            sign = 1;
        else
            sign = -1;
        i = j;
    }
    return sum;
}
int pass1(ifstream &inpfile, ofstream &interim, int *sa = 0, int *size = 0, int *start_present = 0)
{
    string temporary;
    vector<string> array;
    int locctr = 0;
    int flag;
    getline(inpfile, temporary);
    parse_line(temporary, array);
    int block_num = 0;
    int specsig = 0;
    while (inpfile)
    {
       // cout << "this is " << block_num << "\n";
        map<string, int> p_sym; 
        map<string,pair<int,int>> sym_temp;
        vector<string> literal_pool;
        map<string, int> literal2int;
        vector<int> program_block_locctr = {0};
        int cur_program_block = 0;
        int sz_blocks = 1;
        map<string, int> name_to_programblock_mapping;

        block_names.push_back(array[0]);
        interim << temporary << "\n";
        array.clear();
        int fref = 0, fdef = 0;
        vector<pair<int, int>> addresses = {{0, 0}};
        while (inpfile)
        {

            getline(inpfile, temporary);
            // cout<<temporary.size()<<"\n";
            if (temporary.size() == 0)
            {

                continue;
            }
            flag = parse_line(temporary, array);
            // cout<< temporary<<"\n";
         //   cout << array[0] << ":: ::" << array[1] << ":: ::" << array[2] << "\n";
            //ntry_present  = 0;

            
                //cout << "This is LTORG\n"
            if (flag == 0)
            {
                array.clear();
                continue;
            }
            if (array[1] == "CSECT")
            {
                break;
            }
            addresses.push_back({program_block_locctr[cur_program_block], cur_program_block});
            if (array[1] == "END")
            {
                specsig = 1;
                interim << temporary << "\n";
                break;
            }
            if (array[1] == "EQU")
            { 
                if (p_sym[array[0]] == 1)
                {
                    cout << "symbol " << array[0] << " already exists\n";
                    _Exit(0);
                }
                else
                {
                    p_sym[array[0]] = 1;
                    sym_temp[array[0]] = {equexphandler(array[2], program_block_locctr[cur_program_block], sym_temp),-1};
                   // cout << "in equ   " << locctr << " " << dectohex(sym_temp[array[0]].first) << "\n";
                }
                array.clear();
                interim << temporary << "\n";
                continue;
            }
            if (array[1] == "USE")
            {
                if (array[2] == " ")
                    {cur_program_block = 0; 
                    //cout<<"changing blocks\n"; 
                    }
                else
                {
                    if (name_to_programblock_mapping[array[2]] == 0)
                    {
                        sz_blocks += 1;
                        program_block_locctr.push_back(0);
                        cur_program_block = sz_blocks - 1;
                        name_to_programblock_mapping[array[2]] = sz_blocks - 1;
                    }
                    else
                    {
                        cur_program_block = name_to_programblock_mapping[array[2]];
                    }
                }  
                interim << temporary<<"\n";
                array.clear();
                continue;
            }
            if (array[2][0] == '=')
            {
                //cout << "entered key sec\n";
                if (literal2int[array[2]] == 0)
                {
                    literal_pool.push_back(array[2]);
                    literal2int[array[2]] = 1;
                }
            }
            if (array[0] == " ")
                ;
            else
            {
                if (p_sym[array[0]] == 1)
                {
                  //  cout << "symbol " << array[0] << " already exists\n";
                    _Exit(0);
                }
                else
                {
                    p_sym[array[0]] = 1;
                    sym_temp[array[0]] = {program_block_locctr[cur_program_block],cur_program_block};
                }
            }
            int inlen = 3;
            if (array[1][0] == '+')
            {
                array[1] = array[1].substr(1, array[1].size() - 1);
                inlen = 4;
            }
            if ((array[1] == "TIXR") || (array[1] == "CLEAR") || (array[1] == "COMPR"))
            {
                inlen = 2;
            }
            if ((array[1] == "EXTREF") || (array[1] == "EXTDEF"))
            {
                int sz_temp = array[2].size();
                int i = 0, j;
                vector<string> ref_def;
                //cout << array[2] << "\n";
                while (i < sz_temp)
                {
                    j = i;
                    string temp_def_ref;
                    while ((j < sz_temp) && (array[2][j] != ','))
                    {
                        temp_def_ref.push_back(array[2][j]);
                        j++;
                    }
                    ref_def.push_back(temp_def_ref);
                    i = j + 1;
                }
                if (array[1] == "EXTREF")
                {
                    fref = 1;
                    block_references.push_back(ref_def);
                }
                else
                {
                    fdef = 1;
                    block_definitions.push_back(ref_def);
                }
                inlen = 0;
                array.clear();
                interim << temporary << "\n";
                continue;
            }
            if (array[1] == "LTORG")
            {

                array.clear();
                interim << temporary << "\n";
                //cout << "in LTORG proceeding to write literals\n";
                write_literals(literal_pool, interim, program_block_locctr[cur_program_block],cur_program_block, addresses, p_sym, sym_temp);
                continue;
            }
            int locctr = program_block_locctr[cur_program_block];
            if (present_op[array[1]] == 1)
            {
                locctr += inlen;
            }
            else if (array[1] == "WORD")
            {
                locctr += 3;
            }
            else if (array[1] == "RESW")
            {
                locctr += 3 * stoi(array[2]);
            }
            else if (array[1] == "RESB")
            {
                locctr += stoi(array[2]);
            }
            else if (array[1] == "BYTE")
            {
                locctr += detsize(array[2]);
            }
            else
            {
                cout << "error opcode not found!!! " << array[1] << "\n";
                _Exit(0);
            }
            program_block_locctr[cur_program_block] = locctr;
            array.clear();
            //count0++;

            interim << temporary << "\n";
        }
        vector<string> empty;
        if (fref == 0)
        {
            block_references.push_back(empty);
        }
        if (fdef == 0)
        {
            block_definitions.push_back(empty);
        }
        int locctr = program_block_locctr[cur_program_block];
        write_literals(literal_pool, interim, locctr,cur_program_block, addresses, p_sym, sym_temp);
       
        addresses.push_back({locctr,cur_program_block});
        program_block_locctr[cur_program_block] = locctr;
        block_sizes.push_back(accumulate(program_block_locctr.begin(), program_block_locctr.end(), 0));
        
        present_sym_mul.push_back(p_sym);
        sym_tab_mul.push_back(sym_temp);
        instruction_to_address_mul.push_back(addresses);
        
        //cout<<"this is pcb value "<<program_block_locctr[0]<<"\n";
        // cout<<"ending\n"; 
        // for(int i=0;i<sz_blocks;++i) 
        // cout<<program_block_locctr[i]<<" ";
        vector<int> temporary(sz_blocks,0); 
        for(int i=1;i<sz_blocks;++i) 
        { 
            temporary[i] = temporary[i-1] + program_block_locctr[i-1];
            
        }
        block_information.push_back(temporary);
        if (specsig == 1)
            break;
        block_num++;
    }

    inpfile.close();
    interim.close();

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("input file name not specified\n");
        _Exit(0);
    }

    ifstream inpfile;
    inpfile.open(argv[1]);
    ofstream interim;
    interim.open("intermediate.txt");
    int sa = 0, size = 0, start_present = 0;
    mapping_register();
    setup_optab();
    pass1(inpfile, interim, &sa, &size, &start_present);
    int sz = block_names.size();
    // for(int i=0;i<sz;++i)
    // {
    //     cout<<block_names[i]<<" "<<block_sizes[i]<<"\n";
    //     for(auto &it:instruction_to_address_mul[i])
    //     {
    //         cout<<dectohex(it.first)<<" "<<it.second<<"\n";
    //     }
    // }
    // cout<<"\n"<<"\n"<<"\n";   
    // cout<<"finished \n";
    ofstream listingfile; 
    listingfile.open("listing.txt");
    inpfile.open("intermediate.txt");
    ofstream out;
    out.open("output.txt");
    for (int i = 0; i < sz; ++i)
    {
        sicxepass2(inpfile, out,listingfile, i);
    }
    inpfile.close();
    out.close();
}
