#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxsize 100
#define MAX_FILES 30
#define DATA_BLOCK_TYPE 0
#define INDEX_BLOCK_TYPE 1
#define MAX_BLOCK_PTRS maxsize

// Function prototypes
void init(void);
int getEmptySlot(void);
int searchFile(char *name);
void insertFile(char *name, int blocks);
void deleteFile(char *name);
void displaySize(void);
void displayDisk(void);
void displayFiles(void);

union blockContent
{
    int data;                      // For DATA_BLOCK
    int blockPtrs[MAX_BLOCK_PTRS]; // For INDEX_BLOCK, stores pointers to data blocks
};
// Block structure with union for different block types
struct block
{
    int type; // DATA_BLOCK (0) or INDEX_BLOCK (1)
    int used; // 0 for free, 1 for allocated
    union blockContent content;
};

// File entry structure
struct fileEntry
{
    char *name;     // File name
    int indexBlock; // Index block location
};

// Disk and file system setup
struct block disk[maxsize];
int freeSpace = maxsize;
struct fileEntry files[MAX_FILES];

// Initialize file system
void init()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        files[i].name = NULL;
        files[i].indexBlock = -1;
    }

    for (int i = 0; i < maxsize; i++)
    {
        disk[i].type = DATA_BLOCK_TYPE;
        disk[i].used = 0;
        disk[i].content.data = 0;
    }
}

// Get first free block in disk
int getFreeBlock()
{
    for (int i = 0; i < maxsize; i++)
    {
        if (!disk[i].used)
            return i;
    }
    return -1;
}

// Get an empty slot for a new file
int getEmptySlot()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (files[i].name == NULL)
            return i;
    }
    return -1;
}

// Insert a new file
void insertFile(char *name, int blocks)
{

    if (blocks + 1 > freeSpace)
    { // +1 for index block
        printf("\nFile size too big (need %d blocks, only %d available)\n", blocks + 1, freeSpace);
        return;
    }

    if (searchFile(name) != -1)
    {
        printf("\nFile already exists\n");
        return;
    }

    int slot = getEmptySlot();
    if (slot == -1)
    {
        printf("\nNo free file slots\n");
        return;
    }

    // Allocate index block first
    int indexBlock = getFreeBlock();
    if (indexBlock == -1)
    {
        printf("\nNo free blocks available\n");
        return;
    }

    // Setup index block
    disk[indexBlock].type = INDEX_BLOCK_TYPE;
    disk[indexBlock].used = 1;

    // Initialize block pointers to -1
    for (int i = 0; i < MAX_BLOCK_PTRS; i++)
    {
        disk[indexBlock].content.blockPtrs[i] = -1;
    }

    // Allocate data blocks
    int allocated = 0;
    for (int i = 0; i < maxsize && allocated < blocks; i++)
    {
        if (!disk[i].used && i != indexBlock)
        {
            disk[i].type = DATA_BLOCK_TYPE;
            disk[i].used = 1;
            disk[i].content.data = 1;
            disk[indexBlock].content.blockPtrs[allocated] = i;
            allocated++;
        }
    }

    if (allocated < blocks)
    {
        printf("\nNot enough free blocks\n");
        // Cleanup allocated blocks
        disk[indexBlock].used = 0;
        for (int i = 0; i < allocated; i++)
        {
            int blockNum = disk[indexBlock].content.blockPtrs[i];
            if (blockNum != -1)
            {
                disk[blockNum].used = 0;
            }
        }
        return;
    }

    // Setup file entry
    files[slot].name = strdup(name);
    files[slot].indexBlock = indexBlock;
    freeSpace -= (blocks + 1); // Count data blocks + index block

    printf("File inserted successfully\n");
    printf("Index block: %d\n", indexBlock);
    printf("Data blocks: ");
    for (int i = 0; i < blocks; i++)
    {
        printf("%d ", disk[indexBlock].content.blockPtrs[i]);
    }
    printf("\n");
}

// Delete a file
void deleteFile(char *name)
{
    int pos = searchFile(name);
    if (pos == -1)
    {
        printf("\nFile not found\n");
        return;
    }

    int indexBlock = files[pos].indexBlock;
    if (indexBlock < 0 || indexBlock >= maxsize)
    {
        printf("\nInvalid index block\n");
        return;
    }

    int blocksFreed = 1; // Start with 1 for index block

    // Free all data blocks
    for (int i = 0; i < MAX_BLOCK_PTRS; i++)
    {
        int dataBlock = disk[indexBlock].content.blockPtrs[i];
        if (dataBlock >= 0 && dataBlock < maxsize && disk[dataBlock].used)
        {
            disk[dataBlock].used = 0;
            disk[dataBlock].type = DATA_BLOCK_TYPE;
            disk[dataBlock].content.data = 0;
            blocksFreed++;
        }
    }

    // Free index block
    disk[indexBlock].used = 0;
    disk[indexBlock].type = DATA_BLOCK_TYPE;
    disk[indexBlock].content.data = 0;

    // Update free space
    freeSpace += blocksFreed;

    // Clear file entry
    free(files[pos].name);
    files[pos].name = NULL;
    files[pos].indexBlock = -1;

    printf("File deleted successfully\n");
    printf("Freed %d blocks\n", blocksFreed);
}

// Search for a file by name
int searchFile(char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (files[i].name != NULL && strcmp(files[i].name, name) == 0)
            return i;
    }
    return -1;
}

// Display free space
void displaySize()
{
    printf("\nFree space in disk = %d blocks\n", freeSpace);
}

// Display disk layout
void displayDisk()
{
    int i;
    printf("\nDISK:\n\n\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
    for (i = 0; i < maxsize; i++)
    {
        if (i % 10 == 0)
            printf("\n%d\t", i);
        if (disk[i].used)
            printf("%d\t", disk[i].type);
        else
            printf("%d\t", 0);
    }
    printf("\n");
}

// Display files and their blocks
void displayFiles()
{
    printf("\n========== FILES IN DISK ==========\n");
    printf("File Name      Index Block    Data Blocks\n");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < MAX_FILES; i++)
    {
        if (files[i].name != NULL && files[i].indexBlock >= 0 && files[i].indexBlock < maxsize)
        {
            printf("%-15s %-13d ", files[i].name, files[i].indexBlock);

            // Count and display data blocks
            int blockCount = 0;
            printf("[ ");
            for (int j = 0; j < MAX_BLOCK_PTRS; j++)
            {
                int blockNum = disk[files[i].indexBlock].content.blockPtrs[j];
                if (blockNum >= 0 && blockNum < maxsize)
                {
                    printf("%d ", blockNum);
                    blockCount++;
                }
            }
            printf("] (Total: %d blocks)\n", blockCount + 1); // +1 for index block
        }
    }
    printf("==========================================================\n");
}

// Main program
int main()
{
    int option;
    char *name = malloc(20 * sizeof(char));
    int blocks;

    init();
    printf("Indexed File Allocation Technique Simulation\n\n");
    printf("1. Insert a File\n");
    printf("2. Delete a File\n");
    printf("3. Display the Disk\n");
    printf("4. Display All Files\n");
    printf("5. Exit\n");

    while (1)
    {
        displaySize();
        printf("\nEnter option: ");
        scanf("%d", &option);

        switch (option)
        {
        case 1:
            printf("Enter file name: ");
            getchar();
            fgets(name, 20, stdin);
            name[strcspn(name, "\n")] = '\0'; // Remove newline character
            printf("Enter number of blocks: ");
            scanf("%d", &blocks);
            insertFile(name, blocks);
            break;

        case 2:
            printf("Enter file name to delete: ");
            getchar();
            fgets(name, 20, stdin);
            name[strcspn(name, "\n")] = '\0'; // Remove newline character
            deleteFile(name);
            break;

        case 3:
            displayDisk();
            break;

        case 4:
            displayFiles();
            break;

        case 5:
            free(name);
            exit(0);

        default:
            printf("Invalid option. Try again.\n");
        }
    }
}
