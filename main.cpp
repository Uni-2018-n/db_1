#include <iostream>

#include "HT.h"
#include <cstring>
#include "BF.h"


#define PL 19

using namespace std;
int main(){
	Record items[PL];
	for(int i=0;i<PL;i++){
		items[i].id = i;
		sprintf(items[i].name, "name_%d", i);
		sprintf(items[i].surname, "surname_%d", i);
		sprintf(items[i].address, "address_%d", i);
	}


	char temp[256] = "temp";
	HT_CreateIndex(temp, 'i', "id", 14, 126+8); //with more than one block into the hash table, it runs until 29 record.

	HT_info* t = HT_OpenIndex(temp);
	printf("t: %d %c %s %d %ld\n", t->fileDesc, t->attrType, t->attrName, t->attrLength, t->numBuckets);

	for(int i=0;i<PL;i++){
		if(HT_InsertEntry(*t, items[i])<0){
			printf("error\n");
		}
		// void* tt;
		// BF_ReadBlock(t->fileDesc, 0, &tt);
		// char* ht = new char[3];
		// memcpy(ht, (char *)tt, 3);
		// cout << "Test: " << ht << endl;
		printf("added: %d\n", i);
	}
	printf("close index: %d\n\n\n", HT_CloseIndex(t));

	HashStatistics(temp);
	return 0;
}
