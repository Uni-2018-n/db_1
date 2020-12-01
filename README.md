Notes:
* I put the code from BF.h inside extern "C", not sure if this is allowed.

From antonis:
  at HT_InsertEntry before return i've added a comment with a function that i need from HP.
  Basically i need a function to insert to a heap block a record.
  as arguments i can give the fd of the heap (that i have stored into hash table(what the for loop initializes as 0 in HT_CreateIndex)) and the record.
  
