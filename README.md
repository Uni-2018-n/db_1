# Important
I can't compile the code. For some reason there is a public variable in BF.h...

I have used functions from HP in HT.
When I want to compile both files together, there is an error.

There is a simple solution, to just copy paste all HP functions that I need in HT. 
And when we want an HT database, we can just remove HP.cpp from the compilation.
(The opposite for an HP database.)

But what happens in they don't do that in the Makefile that they will use? 

Please try to find a way to compile it. 
I have tried every solution that came to mind, but I can't figure it out...

Notes:
* I put the code from BF.h inside extern "C", not sure if this is allowed.
* The HT_HP_* functions need testing.

From Chris:
  * I added some comments in HT_InsertEntry, HT_DeleteEntry and HT_GetAllEntries for you.


From antonis:
  at HT_InsertEntry before return i've added a comment with a function that i need from HP.
  Basically i need a function to insert to a heap block a record.
  as arguments i can give the fd of the heap (that i have stored into hash table(what the for loop initializes as 0 in HT_CreateIndex)) and the record.
  
