/*Initializes the files.*/
int init_files(int num_files);

/*Imports the files.*/
int imports_file(int num_files);

/*Writes the element in the file.*/
int writes_elem(int shardId, char* key, char* value);

/*Removes the element of the file.*/
int removes_elem(int shardId, char* key);

/*Compacts the file.*/
int compacts_file(int shardId);
