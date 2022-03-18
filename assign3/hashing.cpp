#include<bits/stdc++.h> 
using namespace std; 


class HashNode{ 
public: 
HashNode *next; 
string value; 
HashNode(string val):value(val){next=NULL;} 

}; 


class HashTable { 
public: 

HashNode *hashtable[10]; 
string outputfile;

HashTable(string val){ 
outputfile = val;	
for(int i =0 ;i<10;++i) hashtable[i]=NULL;
}  

void insert(string &val){ 
int length = val.size();  
int index = length%10;
HashNode *starting = hashtable[index]; 
HashNode *previous = NULL;


while(starting != NULL) 
{ 
if(starting->value ==val) 
break; 
previous = starting; 
starting=starting->next;
}


if(starting != NULL) 
return;


HashNode * temp =new HashNode(val);
if(previous == NULL) 
{ 
hashtable[index] = temp;
} 
else 
{ 
previous->next= temp;	
}

}


void print(){ 
ofstream fout(outputfile); 
for(int i=0;i<10;++i) 
{  HashNode *temp = hashtable[i]; 
 while(temp!=NULL) 
 {
fout << temp->value<<"\n"; 
temp = temp->next;
 }	 
} 
fout.close();
}

};


