typedef struct {
   char *word;
   int occurrences;
} HashNode;
HashNode *createTable(int *tabSize, int *occupiedCells);
int nextPrime(int n);
int isPrime(int n);
void insert(HashNode *table, char *str, int *tabSize, int *occupiedCells);
unsigned int hash(char *str);
void rehash(HashNode *table, int *tabSize, int *occupiedCells);
unsigned int findPosition(HashNode *n, char *str, int *tabSize);
