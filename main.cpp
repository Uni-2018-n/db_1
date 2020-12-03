#include <iostream>

#include "HT.h"
#include <cstring>
int main(){
	Record items[1000];
	for(int i=0;i<1000;i++){
		items[i].id = i;
		sprintf(items[i].name, "name_%d", i);
		sprintf(items[i].surname, "surname_%d", i);
		sprintf(items[i].address, "address_%d", i);
	}


	char temp[256] = "temp";
	HT_CreateIndex(temp, 'i', "id", 14, 8);

	HT_info* t = HT_OpenIndex(temp);
	printf("t: %d %c %s %d %ld\n", t->fileDesc, t->attrType, t->attrName, t->attrLength, t->numBuckets);

	for(int i=0;i<1000;i++){
		if(HT_InsertEntry(*t, items[i])<0){
			printf("error\n");
		}
		printf("added: %d\n", i);
	}


	// Record yeet;
	// yeet.id = 3;
	// strcpy(yeet.name, "name");
	// strcpy(yeet.surname, "surname");
	// strcpy(yeet.address, "address");
	//
	// printf("HT_InsertEntry: %d\n\n", HT_InsertEntry(*t, yeet));
	// printf("\nHT_GetAllEntries(gives the number of blocks needed to read): %d\n", HT_GetAllEntries(*t,3/));
	// printf("HT_DeleteEntry: %d\n", HT_DeleteEntry(*t, &(yeet.id)));
	// printf("\nHT_GetAllEntries(gives the number of blocks needed to read): %d\n", HT_GetAllEntries(*t, &(yeet.id)));

	printf("close index: %d\n", HT_CloseIndex(t));
	return 0;
}
