#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>



typedef struct HuffmanTree {
    int occurrences;
    char* str;
    struct HuffmanTree * left;
    struct HuffmanTree * right;
}HuffmanTree;

typedef struct node
{
    char* data;
    char* code; //ONLY TO BE USED WITH COMPRESS AND DECOMPRESS
    int occurrences;
    struct node * prev;
    struct node * next;
}node;

typedef struct HeapNode{
    char* str;
    int occurrences;
    struct HeapNode * left;
    struct HeapNode * right;
}HeapNode;

typedef struct HuffmanMinHeap{
    int numberOfItems;
    int size;
    HeapNode * items;
    //char** items;
} HuffmanMinHeap;

HuffmanMinHeap * heap;
node * totalItems;
HeapNode * codeTree;
int sizeofstring = 0;
int numOfItems = 0;

void printHuffmanCode(HeapNode* temp, char* str, int fd);
bool isFile(char* path);

int level = 0;

bool isTxt(char* file){
    int length1 = strlen(file);
    int length2 = 4; //The length of the text ".txt"
    if (strcmp(file+(length1 - length2), ".txt") == 0){
        return true;
    } else {
        return false;
    }
}

bool isDirectory(char* path){
    DIR* directory = opendir(path);
    if (directory != NULL){
        return true;
    } else {
        return false;
    }

}

void appendFilePath(char* path, char* nextDirectoryName){
/*
    int length1 = strlen(path);
    int length2 = strlen(nextDirectoryName);
    char* result = malloc(sizeof(char) * (length1 + length2 + 1));
    bzero(result, (length1 + length2 + 1));
    strcat(result, path);
    strcat(result, "/");
    strcat(result, nextDirectoryName);
    return result;
*/

    strcat(path, "/");
    strcat(path, nextDirectoryName);
}

void deleteLatestFilePath(char* path, char* nextDirectoryName){
    int length1 = strlen(path);
    int length2 = strlen(nextDirectoryName);
    bzero(path+(length1 - length2 - 1), length2 + 1);
}

void printMinHeap(){
    HeapNode* temp = heap->items;
    int i;
    for (i = 0; i < heap->numberOfItems; i++){
        printf("%s: Occurrences: %d\n", temp[i].str, temp[i].occurrences);
    }
    printf("\n");
}

void siftUp(){
    int k = heap->numberOfItems - 1;
    while (k > 0){
        int p = (k-1)/2;
        HeapNode* itemt = heap->items + k;
        HeapNode* parentt = heap->items + p;
        int item = itemt->occurrences;
        int parent = parentt->occurrences;
        if (item < parent){
            //swap
            char* temp = malloc(sizeofstring * sizeof(char));
            int temp2 = itemt->occurrences;
            HeapNode* temp3 = itemt->left;
            HeapNode* temp4 = itemt->right;
            strcpy(temp, itemt->str);
            strcpy(itemt->str, parentt->str);
            itemt->occurrences = parentt->occurrences;
            itemt->left = parentt->left;
            itemt->right = parentt->right;
            strcpy(parentt->str, temp);
            parentt->occurrences = temp2;
            parentt->left = temp3;
            parentt->right = temp4;
            free(temp);
            k = p;
        } else {
            break;
        }
    }
}

void copyNode(HeapNode* dest, HeapNode* src){
    strcpy(dest->str, src->str);
    dest->occurrences = src->occurrences;
    dest->left = src->left;
    dest->right = src->right;
}

void insert(HeapNode* ptr){
    int num = heap->numberOfItems;

    if (num == heap->size){
        //grow heap array here...Im too lazy to do this now
        heap->size += 100;
        HeapNode* realloced = realloc(heap->items, sizeof(HeapNode) * heap->size);
        if (realloced != NULL){
            heap->items = realloced;
            HeapNode* temp = heap->items;
            int i;
            for (i = heap->size - 100; i < heap->size; i++){
                temp[i].occurrences = 0;
                temp[i].str = malloc(sizeof(char) * sizeofstring); //Assuming tokens are gonna be 256 characers or less
                bzero(temp[i].str, sizeofstring * sizeof(char));
            }
        } else {
            printf("The heap array could not be grown\n");
            exit(1);
        }
        
    }
    HeapNode* items = heap->items;
    copyNode(items+num, ptr);
    heap->numberOfItems++;
    siftUp();
}

bool heapHasMoreThanOne(){
    if (heap->numberOfItems > 1){
        return true;
    } else {
        return false;
    }
}

void intializeMinHeap(){
    heap = malloc(sizeof(HuffmanMinHeap));
    heap->numberOfItems = 0;
    heap->size = numOfItems;
    heap->items = malloc(sizeof(HeapNode) * heap->size);
    //heap->items = malloc(sizeof(char*) * 100);
    HeapNode* temp = heap->items;
    //char** temp = heap->items;
    int i;
    for (i = 0; i < heap->size; i++){
        temp[i].occurrences = 0;
        temp[i].str = malloc(sizeof(char) * sizeofstring); //Assuming tokens are gonna be 256 characers or less
        bzero(temp[i].str, sizeof(char) * sizeofstring);
    }
}

void siftDown(){
    int k = 0;
    int l = 2*k+1;
    while(l < heap->numberOfItems){
        int min = l;
        int r = l+1;
        if (r < heap->numberOfItems){ //Means there is a right child
            int left = heap->items[l].occurrences;
            int right = heap->items[r].occurrences;
            if (right < left){
                min++;
            }
        }
        int parent = heap->items[k].occurrences;
        int minChild = heap->items[min].occurrences;
        if (minChild < parent){
            char* temp = malloc(sizeof(char) * sizeofstring);
            strcpy(temp, heap->items[k].str);
            int temp2 = heap->items[k].occurrences;
            HeapNode* temp3 = heap->items[k].left;
            HeapNode* temp4 = heap->items[k].right;
            strcpy(heap->items[k].str, heap->items[min].str);
            heap->items[k].occurrences = heap->items[min].occurrences;
            heap->items[k].left = heap->items[min].left;
            heap->items[k].right = heap->items[min].right;
            strcpy(heap->items[min].str, temp);
            heap->items[min].occurrences = temp2;
            heap->items[min].left = temp3;
            heap->items[min].right = temp4;
            k = min;
            l = 2*k+1;
            free(temp);
        } else {
            break;
        }
    }
}

HeapNode* deleteNode(){
    if (heap->numberOfItems == 0){
        printf("There are no items in the min Heap\n");
        return NULL;
    }
    if (heap->numberOfItems == 1){
        HeapNode* result = malloc(sizeof(HeapNode));
        result->str = malloc(sizeof(char) * sizeofstring);
        result->occurrences = heap->items[0].occurrences;
        strcpy(result->str, heap->items[0].str);
        result->left = heap->items[0].left;
        result->right = heap->items[0].right;
        bzero(heap->items[0].str, sizeof(char) * sizeofstring);
        heap->items[0].occurrences = 0;
        heap->items[0].left = NULL;
        heap->items[0].right = NULL;
        heap->numberOfItems--;
        return result;
    }
    HeapNode* result = malloc(sizeof(HeapNode));
    result->occurrences = heap->items[0].occurrences;
    result->str = malloc(sizeof(char) * sizeofstring);
    strcpy(result->str, heap->items[0].str);
    result->left = heap->items[0].left;
    result->right = heap->items[0].right;
    strcpy(heap->items[0].str, heap->items[heap->numberOfItems - 1].str);
    bzero(heap->items[heap->numberOfItems - 1].str, sizeof(char) * sizeofstring);
    heap->items[0].left = heap->items[heap->numberOfItems - 1].left;
    heap->items[0].right = heap->items[heap->numberOfItems - 1].right;
    heap->items[0].occurrences = heap->items[heap->numberOfItems - 1].occurrences;
    heap->items[heap->numberOfItems - 1].occurrences = 0;
    heap->items[heap->numberOfItems - 1].left = NULL;
    heap->items[heap->numberOfItems - 1].right = NULL;
    heap->numberOfItems--;
    siftDown();
    return result;
}

void recursiveBehavior(char* path, void (*fnptr)(char*), bool openNewDirectories){

    DIR* dir;
    struct dirent * handle;

    dir = opendir(path);

    if (dir == NULL){
        printf("Error opening directory with path name: [%s] in recursive Behavior\n", path);
        exit(1);
    }

    handle = readdir(dir);

    while (handle != NULL){
        if (strcmp(".git", handle->d_name) != 0 && strcmp(".", handle->d_name) != 0 && strcmp("..", handle->d_name) != 0){
            if (handle->d_type == DT_DIR && openNewDirectories){
                appendFilePath(path, handle->d_name);
                //DIR* temp = opendir(path);
                recursiveBehavior(path, fnptr, openNewDirectories);
                deleteLatestFilePath(path, handle->d_name);
                
            } else if (handle->d_type == DT_REG && strcmp(handle->d_name, "fileCompressor") != 0){
                appendFilePath(path, handle->d_name);
                printf("Scanning diles [%s]\n", path);
                fnptr(path);
                deleteLatestFilePath(path, handle->d_name);

            }
        }
        handle = readdir(dir);
    }
    closedir(dir);
}

/*
void recursiveBehavior(char* path, void (*fnptr)(char*), bool fullRecursive){
    DIR * directory;
    struct dirent * handle;
    directory = opendir(path);
    if (directory == NULL){
        printf("The directory failed to open, EXITING Program\n");
        exit(1);
    } else {
        //printf("The directory was successfully opened\n");
    }
    
    printFileHierachy(directory, path, fnptr, fullRecursive);
}
*/

void printlist(node * ptr){
    while (ptr != NULL){
        printf("[%s], Code: [%s], Occurrence: %d\n", ptr->data, ptr->code, ptr->occurrences);
        ptr = ptr->next;
    }
}

void freelist(node * sort){
    while (sort != NULL){
        node * temp = sort;
        sort = sort->next;
        temp->prev = NULL;
        temp->next = NULL;
        free(temp->data);
        free(temp->code);
        free(temp);
    }
}

bool listContainsAndUpdates(char* str, node * ptr, int num){
    while (ptr != NULL){
        if (strcmp(str, ptr->data) == 0){
            ptr->occurrences += num;
            bzero(str, sizeof(char) * 1000);
            return true;
        } else {
            ptr = ptr->next;
        }
    }
    return false;
}

node * populateTokenList(char* filepath, bool noDuplicatesInList){
    node * sort = malloc(sizeof(node));
    sort->data = malloc(sizeof(char) * 1000);
    sort->code = malloc(sizeof(char) * 1000);
    sort->occurrences = 0;
    bzero(sort->data, sizeof(char) * 1000);
    bzero(sort->code, sizeof(char) * 1000);
    sort->prev = NULL;
    sort->next = NULL;
    node * ptr = sort;
	int fd = open(filepath, O_RDONLY);
	if (fd == -1){
        printf("Error opening file, EXITTING PROGRAM\n");
        freelist(sort);
        return;
	}
	char buffer = '!';
    int readed = 0;
    char* str = malloc(sizeof(char) * 1000);
    str[0] = '\0';
    bool emptyFileCheck = true;
    int i = 0;
    do{
        readed = read(fd, &buffer, 1);
        if (emptyFileCheck){
            if (readed == 0) { //First check of file and the file is empty
                printf("The file given is empty, EXITTING PROGRAM\n");
                freelist(sort);
                free(str);
                return;
            } else {
                emptyFileCheck = false;
            }
        }

        if (readed != 0){
            if (buffer != ' ' && buffer != '\n' && buffer != '\t'){
                char* temp = malloc(sizeof(char) * 2);
                temp[0] = buffer;
                temp[1] = '\0';
                strcat(str, temp);
                free(temp);
            } else if ((buffer == '\n' || buffer == ' ' || buffer == '\t') ){
                printf("Adding [%s] to list\n", str);
                
                if (noDuplicatesInList){
                    if (strlen(str) != 0 && !listContainsAndUpdates(str, sort, 1)){
                        strcpy(ptr->data, str);
                        ptr->occurrences++;
                        node * temp = malloc(sizeof(node));
                        temp->data = malloc(sizeof(char) * 1000);
                        temp->code = malloc(sizeof(char) * 1000);
                        temp->occurrences = 0;
                        bzero(temp->data, sizeof(char) * 1000);
                        bzero(temp->code, sizeof(char) * 1000);
                        temp->prev = ptr;
                        temp->next = NULL;
                        ptr->next = temp;
                        ptr = ptr->next;
                        bzero(str, sizeof(char) * 1000);
                    }

                    char* tempstr = malloc(sizeof(char) * 1000);
                    if (buffer == '\n'){
                        tempstr[0] = '\\';
                        tempstr[1] = 'n';
                        tempstr[2] = '\0';
                    } else if(buffer == '\t'){
                        tempstr[0] = '\\';
                        tempstr[1] = 't';
                        tempstr[2] = '\0';
                    } else {
                        tempstr[0] = buffer;
                        tempstr[1] = '\0';
                    }
                    if (!listContainsAndUpdates(tempstr, sort, 1)){
                        strcpy(ptr->data, tempstr);
                        ptr->occurrences++;
                        node * temp = malloc(sizeof(node));
                        temp->data = malloc(sizeof(char) * 1000);
                        temp->code = malloc(sizeof(char) * 1000);
                        temp->occurrences = 0;
                        bzero(temp->data, sizeof(char) * 1000);
                        bzero(temp->code, sizeof(char) * 1000);
                        temp->prev = ptr;
                        temp->next = NULL;
                        ptr->next = temp;
                        ptr = ptr->next;
                    }
                    free(tempstr);
                } else {
                    if (strlen(str) != 0){
                        strcpy(ptr->data, str);
                        ptr->occurrences++;
                        node * temp = malloc(sizeof(node));
                        temp->data = malloc(sizeof(char) * 1000);
                        temp->code = malloc(sizeof(char) * 1000);
                        temp->occurrences = 0;
                        bzero(temp->data, sizeof(char) * 1000);
                        bzero(temp->code, sizeof(char) * 1000);
                        temp->prev = ptr;
                        temp->next = NULL;
                        ptr->next = temp;
                        ptr = ptr->next;
                        bzero(str, sizeof(char) * 1000);
                    }

                    char* tempstr = malloc(sizeof(char) * 1000);
                    if (buffer == '\n'){
                        tempstr[0] = '\\';
                        tempstr[1] = 'n';
                        tempstr[2] = '\0';
                    } else if(buffer == '\t'){
                        tempstr[0] = '\\';
                        tempstr[1] = 't';
                        tempstr[2] = '\0';
                    } else {
                        tempstr[0] = buffer;
                        tempstr[1] = '\0';
                    }
                    strcpy(ptr->data, tempstr);
                    ptr->occurrences++;
                    node * temp = malloc(sizeof(node));
                    temp->data = malloc(sizeof(char) * 1000);
                    temp->code = malloc(sizeof(char) * 1000);
                    temp->occurrences = 0;
                    bzero(temp->data, sizeof(char) * 1000);
                    bzero(temp->code, sizeof(char) * 1000);
                    temp->prev = ptr;
                    temp->next = NULL;
                    ptr->next = temp;
                    ptr = ptr->next;
                    free(tempstr);
                }
            }
        }
    } while(readed != 0);
    //Delete the last node since it will always be empty
    free(str);
    node * temp = ptr;
    ptr = ptr->prev;
    if (ptr == NULL){
        //That means nothing was put in the list
        printf("The file given is empty or no values were given, EXITTING PROGRAM\n");
        freelist(sort);
        return;

    }
    ptr->next = NULL;
    //Free temp
    temp->prev = NULL;
    free(temp->data);
    free(temp->code);
    free(temp);
    return sort;
}


void addTokenListToHeap(node * ptr){
    while (ptr != NULL){
        HeapNode* temp = malloc(sizeof(HeapNode));
        temp->occurrences = ptr->occurrences;
        temp->str = malloc(sizeof(char) * sizeofstring);
        strcpy(temp->str, ptr->data);
        temp->left = NULL;
        temp->right = NULL;
        insert(temp);
        ptr = ptr->next;
    }
}

void buildHuffmanTree(){
    //char* str = malloc(sizeof(char) * 256);
    while (heapHasMoreThanOne()){
        //printMinHeap();
        //bzero(str, 256);
        HeapNode* first = deleteNode();
        //printHuffmanCode(first, str);
        //printf("Post first node\n");
        HeapNode* second = deleteNode();
        //bzero(str, 256);
        //printHuffmanCode(second, str);
        //printf("Post second node\n");
        HeapNode* third = malloc(sizeof(HeapNode));
        third->str = malloc(sizeof(char) * sizeofstring);
        bzero(third->str, sizeof(char) * sizeofstring);
        /*
        if (strlen(first->str) + strlen(second->str) >= 255){
            printf("IM MAXXED OUT\n");
            //exit(1);
        }
        */
        strcat(third->str, first->str);
        strcat(third->str, second->str);
        third->occurrences = first->occurrences + second->occurrences;
        third->left = first;
        third->right = second;
        //bzero(str, 256);
        //printHuffmanCode(third, str);
        //printf("Post third node\n");
        insert(third);
    }
}

void printHuffmanCode(HeapNode* temp, char* str, int fd){
    if (temp == NULL){
        return;
    }

    if (temp->left == NULL && temp->right == NULL){
        char* huffmanString = malloc(sizeof(char) * sizeofstring);
        strcpy(huffmanString, str);
        strcat(huffmanString, "\t");
        strcat(huffmanString, temp->str);
        strcat(huffmanString, "\n");
        int wrote = 0;
        int read = 0;
        do {
            wrote = write(fd, huffmanString+read, strlen(huffmanString) - read);
            read += wrote;
        } while (wrote > 0);
        free(huffmanString);
        //printf("The code for [%s] is [%s]\n", temp->str, str);
        return;
    }

    strcat(str, "0");
    printHuffmanCode(temp->left, str, fd);
    str[strlen(str) - 1] = '1';
    printHuffmanCode(temp->right, str, fd);
    str[strlen(str) - 1] = '\0';
}

void addToTotolList(node * ptr2){
    if (totalItems == NULL){
        totalItems = malloc(sizeof(node));
        totalItems->data = malloc(sizeof(char) * 1000);
        totalItems->code = malloc(sizeof(char) * 1000);
        totalItems->next = NULL;
        totalItems->prev = NULL;
        bzero(totalItems->data, 1000);
        bzero(totalItems->code, sizeof(char) * 1000);
    }
    node * ptr = totalItems;
    while (ptr->next != NULL){
        ptr = ptr->next;
    }
    while (ptr2 != NULL){
        if (!listContainsAndUpdates(ptr2->data, totalItems, ptr2->occurrences)){ //Note that listContainsAndUpdates bzeros ptr2->data if a match is found
            node * temp = malloc(sizeof(node));
            temp->next = NULL;
            temp->data = malloc(sizeof(char) * 1000);
            temp->code = malloc(sizeof(char) * 1000);
            bzero(temp->data, sizeof(char) * 1000);
            bzero(temp->code, sizeof(char) * 1000);
            strcpy(temp->data, ptr2->data);
            strcpy(temp->code, ptr2->code);
            temp->occurrences = ptr2->occurrences;
            temp->prev = ptr;
            ptr->next = temp;
            ptr = ptr->next;
        }
        ptr2 = ptr2->next;
    }
}

void recursiveCodeBook(char* filepath){
    if (!isDirectory(filepath)){
        node * tokenList;
        tokenList = populateTokenList(filepath, true);
        addToTotolList(tokenList);
        freelist(tokenList);
    }

}

void writeHuffmanCode(){
        int fd = open("./HuffmanCodebook", O_WRONLY | O_CREAT, 00600);
        char* str = malloc(sizeof(char) * 1000);
        strcpy(str, "\\\n");
        int wrote = 0;
        int read = 0;
        do {
            wrote = write(fd, str+read, strlen(str) - read);
            read += wrote;
        } while (wrote > 0);
        bzero(str, 1000);
        printHuffmanCode(heap->items, str, fd);
        bzero(str, 1000);
        strcpy(str, "\n");
        write(fd, str, strlen(str));
        free(str);
        close(fd);
}

void sizeofLargestToken(){
    node* ptr = totalItems;
    while (ptr != NULL){
        numOfItems++;
        if (strlen(ptr->data) > sizeofstring){
            sizeofstring = strlen(ptr->data);
        }
        ptr = ptr->next;
    }
    sizeofstring *= numOfItems;
}

void buildCodeBook(char* filepath, bool recursive){
    if (recursive){
        recursiveBehavior(filepath, recursiveCodeBook, true);
        totalItems = totalItems->next; //Because the first node in totalItems is always going to be empty
    } else if (isDirectory(filepath)){
        recursiveBehavior(filepath, recursiveCodeBook, false);
        totalItems = totalItems->next;
    } else {
        totalItems = populateTokenList(filepath, true);
    }
    sizeofLargestToken();
    intializeMinHeap();
    addTokenListToHeap(totalItems);
    buildHuffmanTree();
    writeHuffmanCode();
    freelist(totalItems);
    //FREE minheap
}

void convertCodebook(char* filepath){
    totalItems = malloc(sizeof(node));
    totalItems->code = malloc(sizeof(char) * 1000);
    totalItems->data = malloc(sizeof(char) * 1000);
    totalItems->occurrences = 0;
    totalItems->next = NULL;
    totalItems->prev = NULL;
    node * ptr = totalItems;
    int fd = open(filepath, O_RDONLY);
	if (fd == -1){
        printf("Error opening codebook, EXITTING PROGRAM\n");
        freelist(totalItems);
        exit(1);
	}
	char buffer = '\0';
    int readed = 0;
    char* str = malloc(sizeof(char) * 1000);
    str[0] = '\0';
    char* code = malloc(sizeof(char) * 1000);
    code[0] = '\0';
    bool codeChecked = false;
    bool emptyFileCheck = true;
    //Throw away escape characters
    read(fd, &buffer, 1); //Throws away "\\"
    read(fd, &buffer, 1); //Throws away "\n"
    do {
        readed = read(fd, &buffer, 1);
        if (emptyFileCheck){
            if (readed == 0) { //First check of file and the file is empty
                printf("The file given is empty, EXITTING PROGRAM\n");
                freelist(totalItems);
                free(str);
                return;
            } else {
                emptyFileCheck = false;
            }
        }

        if (!codeChecked && buffer != '\t' && buffer != '\n'){
            char temp[2] = {buffer, '\0'};
            strcat(code, temp);
        } else if (buffer == '\t'){
            codeChecked = true;
        } else if (buffer != '\n'){
            char temp[2] = {buffer, '\0'};
            strcat(str, temp);
        } else if (buffer == '\n'&& strlen(code) != 0 && strlen(str) != 0){
            strcpy(ptr->code, code);
            strcpy(ptr->data, str);
            node * temp = malloc(sizeof(node));
            temp->occurrences = 0;
            temp->code = malloc(sizeof(char) * 1000);
            temp->data = malloc(sizeof(char) * 1000);
            temp->next = NULL;
            temp->prev = ptr;
            ptr->next = temp;
            ptr = ptr->next;
            bzero(code, sizeof(char) * 1000);
            bzero(str, sizeof(char) * 1000);
            codeChecked = false;
        }
    } while (readed != 0);
    node * temp = ptr;
    ptr = ptr->prev;
    ptr->next = NULL;
    temp->prev = NULL;
    free(temp->code);
    free(temp->data);
    free(temp);
    free(str);
    free(code);
    close(fd);
}

char* getCode(char* str){
    node * ptr = totalItems;
    while (ptr != NULL){
        if (strcmp(ptr->data, str) == 0){
            return ptr->code;
        }
        ptr = ptr->next;
    }
    return NULL;
}

void applyCodebookToCompress(node* list){
    while (list != NULL){
        char* code = getCode(list->data);
        if (code == NULL){
            printf("There is no code for the string [%s]\n", list->data);
            exit(1);
        }
        strcpy(list->code, code);
        list = list->next;
    }
}

void printCode(node* ptr, char* filepath){
    int fd = open(filepath, O_WRONLY | O_CREAT, 00600);
    if (fd == -1){
        printf("Something went wrong with opening file [%s]\n", filepath);
        exit(1);
    }
    while (ptr != NULL){
        char* temp = ptr->code;
        int read = 0;
        int wrote = 0;
        do {
            wrote = write(fd, temp+read, strlen(temp)-read);
            read += wrote;
        } while (wrote != 0);
        //printf("%s", ptr->code);
        ptr = ptr->next;
    }
    //printf("\n");
    close(fd);
}

void recursiveCompress(char* filepath){
    if (!isDirectory(filepath) && !isFile(filepath)){
        node * tokenList;
        tokenList = populateTokenList(filepath, false);
        applyCodebookToCompress(tokenList);
        char* hczfile = malloc(sizeof(char) * 1000);
        strcpy(hczfile, filepath);
        strcat(hczfile, ".hcz");
        printCode(tokenList, hczfile);
        free(hczfile);
        freelist(tokenList);
    }

}


void compressFile(char* filePath, bool recursive, char* codebookFilePath){
    convertCodebook(codebookFilePath);
    //printlist(totalItems);
    if (recursive){
        recursiveBehavior(filePath, recursiveCompress, true);
    } else if (isDirectory(filePath)){
        recursiveBehavior(filePath, recursiveCompress, false);
    } else {
        recursiveCompress(filePath);
    }
    freelist(totalItems);
}

void printTree(HeapNode* ptr){
    if (ptr == NULL) return;

    printTree(ptr->left);
    printTree(ptr->right);

    if (strlen(ptr->str) != 0){
        printf("[%s]\n", ptr->str);
    }
}

void createDecompressedFile(char* file, char* output){
    int fd = open(file, O_WRONLY | O_CREAT, 00600);
    if (fd == -1){
        printf("Something went wrong with opening file in createDecompressFile[%s]\n", file);
        exit(1);
    }
    int read = 0;
    int wrote = 0;
    do {
        wrote = write(fd, output+read, strlen(output)-read);
        read += wrote;
    } while (wrote != 0);
}

void recursiveDecompress(char* filepath){
    char* result = malloc(sizeof(char) * 50000);
    bzero(result, sizeof(char) * 50000);
    if (isFile(filepath)){
        int fd = open(filepath, O_RDONLY);
        if (fd == -1){
            printf("Error opening filename [%s] in decompress process\n", filepath);
            exit(1);
        }
        char buffer = '!';
        int readed = 0;
        bool emptyFileCheck = true;
        HeapNode* ptr = codeTree;
        do {
            readed = read(fd, &buffer, 1);
            if (emptyFileCheck){
                if (readed == 0) { //First check of file and the file is empty
                    printf("The file given is empty in decompress [%s], EXITTING PROGRAM\n", filepath);
                    exit(1);
                } else {
                    emptyFileCheck = false;
                }
            }

            if (readed != 0){
                if (buffer == '0'){
                    ptr = ptr->left;
                } else if (buffer == '1'){
                    ptr = ptr->right;
                }

                if (ptr == NULL){
                    printf("Something bad happened in decompress, there wasn't a compabilible word for the code\n");
                    exit(1);
                }

                if (strlen(ptr->str) != 0){
                    //Hit the leaf

                    if (ptr->str[0] == '\\' && ptr->str[1] == 'n'){
                        strcat(result, "\n");
                        //printf("\n", ptr->str);
                    } else if (ptr->str[0] == '\\' && ptr->str[1] == 't'){
                        strcat(result, "\t");
                        //printf("\t", ptr->str);
                    } else {
                        strcat(result, ptr->str);
                        //printf("%s", ptr->str);
                    }

                    //printf("%s", ptr->str);

                    ptr = codeTree;
                }
            }
        } while (readed != 0);
        deleteLatestFilePath(filepath, "hcz");
        createDecompressedFile(filepath,result);
        strcat(filepath, ".hcz");
        //printf("\n");
        close(fd);
    }
    free(result);

}

void convertCodebookToTree(char* codebookfilepath){
    convertCodebook(codebookfilepath);
    node* ptr = totalItems;

    codeTree = malloc(sizeof(node));
    codeTree->str = malloc(sizeof(char) * 1000);
    bzero(codeTree->str, sizeof(char) * 1000);
    codeTree->occurrences = 0;
    codeTree->left = NULL;
    codeTree->right = NULL;

    HeapNode* treeptr = codeTree;

    while (ptr != NULL){
        char* str = ptr->data;
        char* code = ptr->code;

        int i;
        for (i = 0; i < strlen(code); i++){
            if (code[i] == '0'){
                if (treeptr->left == NULL){
                    HeapNode* temp = malloc(sizeof(node));
                    temp->str = malloc(sizeof(char) * 1000);
                    bzero(temp->str, sizeof(char) * 1000);
                    temp->occurrences = 0;
                    temp->left = NULL;
                    temp->right = NULL;
                    treeptr->left = temp;
                }
                treeptr = treeptr->left;
            } else if (code[i] == '1'){
                if (treeptr->right == NULL){
                    HeapNode* temp = malloc(sizeof(node));
                    temp->str = malloc(sizeof(char) * 1000);
                    bzero(temp->str, sizeof(char) * 1000);
                    temp->occurrences = 0;
                    temp->left = NULL;
                    temp->right = NULL;
                    treeptr->right = temp;
                }
                treeptr = treeptr->right;
            }
        }
        strcpy(treeptr->str, str);
        treeptr = codeTree;
        ptr = ptr->next;
    }



}

bool isFile(char* path){
    int length1 = strlen(path);
    int length2 = 4; //The length of the text ".txt"
    if (strcmp(path+(length1 - length2), ".hcz") == 0){
        return true;
    } else {
        return false;
    }
}

void decompressFile(char* filepath, bool recursive, char* codebookfilepath){
    convertCodebookToTree(codebookfilepath);
    //printTree(codeTree);
    if (recursive){
        recursiveBehavior(filepath, recursiveDecompress, true);
    } else if (!isFile(filepath)){
        recursiveBehavior(filepath, recursiveDecompress, false);
    } else {
        recursiveDecompress(filepath);
    }
    freelist(totalItems);
}

bool containsHuffmanpPath(char* str){
    int length1 = strlen(str);
    int length2 = strlen("HuffmanCodebook");
    if (strcmp(str+(length1 - length2), "HuffmanCodebook") == 0){
        return true;
    } else {
        return false;
    }
    
}


int main(int argc, char** argv){
    bool recursive = false;
    bool codeBook = false;
    bool compress = false;
    bool decompress = false;
    bool filePathSeen = false;
    char* filePath = malloc(sizeof(char) * 4096);
    char* huffmanFilePath = malloc(sizeof(char) * 4096);
    //File paths may only be relative paths as of now !! IMPORTANT
    int i;
    bool flags = true;
    for (i = 1; i < argc; i++){
        if (argv[i][0] == '-'){
            if (argv[i][1] == 'b'){
                codeBook = true;
            } else if (argv[i][1] == 'd'){
                decompress = true;
            } else if (argv[i][1] == 'c'){
                compress = true;
            } else if (argv[i][1] == 'R'){
                recursive = true;
            } else {
                printf("Invalid flag type Error\n");
                exit(1);
            }
        } else if (argv[i][0] == '.'){
            if (!filePathSeen){
                strcpy(filePath, argv[i]);
                filePathSeen = true;
            } else {
                strcpy(huffmanFilePath, argv[i]);
            }
        }
    }

    if (containsHuffmanpPath(filePath)){
        printf("Huffman Codebook filepath needs to come as last argument: ERROR\n");
        exit(1);
    }

    if (codeBook){
        buildCodeBook(filePath, recursive);
    } else if (compress){
        compressFile(filePath, recursive, huffmanFilePath);
    } else if (decompress){
        decompressFile(filePath, recursive, huffmanFilePath);
    }


    //printFileHierachy(directory, path);
    //intializeMinHeap();

    //closedir(directory);
    printf("Success! End of File\n");


	return 0;
}
