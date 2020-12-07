#include <iostream>

#include "HT.h"
#include <cstring>
#include "BF.h"


#define PL 1000

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
	for(int i=0;i<PL;i++){
		if(HT_InsertEntry(*t, items[i])<0){
			cout << "error" << endl;
		}
		// cout << "added: " << i << endl;
	}
	int f= 3;

	cout << "returned by get all entries" << endl << HT_GetAllEntries(*t, &f) << endl;

	cout << "returned by delete entry" << endl << HT_DeleteEntry(*t, &f) << endl;

	cout << "returned by get all entries" << endl << HT_GetAllEntries(*t, &f) << endl;
	cout << "close index: " << HT_CloseIndex(t) << endl << endl << endl;


	if(HashStatistics(temp)<0){
		cout << "Hash returned error" << endl;
		return -1;
	}
	return 0;
}
