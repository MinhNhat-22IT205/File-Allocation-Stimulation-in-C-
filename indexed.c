#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define maxsize 100
#define MAX_FILES 30
#define MAX_BLOCK_PTRS maxsize

#define DATA_BLOCK_TYPE 0
#define INDEX_BLOCK_TYPE 1

union BlockContent
{
    int data;                                // For DATA_BLOCK
    struct Block *blockPtrs[MAX_BLOCK_PTRS]; // For INDEX_BLOCK
};

struct Block
{
    int type; // DATA_BLOCK (0) or INDEX_BLOCK (1)
    union BlockContent content;
};

struct FileEntry
{
    char *name;     // File name
    int indexBlock; // Index block location
};

struct Block disk[maxsize];
int freeSpace = maxsize;
struct FileEntry files[MAX_FILES];

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
        disk[i].content.data = 0;
    }
}

int getFreeBlock()
{
    for (int i = 0; i < maxsize; i++)
    {
        if (disk[i].content.data == 0)
            return i;
    }
    return -1;
}

int getEmptySlot()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (files[i].name == NULL)
            return i;
    }
    return -1;
}

int searchFile(char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (files[i].name != NULL && strcmp(files[i].name, name) == 0)
            return i;
    }
    return -1;
}

void insertFile(char *name, int blocks)
{
    if (blocks + 1 > freeSpace)
    {
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

    int indexBlock = getFreeBlock();
    if (indexBlock == -1)
    {
        printf("\nNo free blocks available\n");
        return;
    }

    disk[indexBlock].type = INDEX_BLOCK_TYPE;
    disk[indexBlock].content.data = 1;

    for (int i = 0; i < MAX_BLOCK_PTRS; i++)
    {
        disk[indexBlock].content.blockPtrs[i] = NULL;
    }

    int allocated = 0;
    for (int i = 0; i < maxsize && allocated < blocks; i++)
    {
        if (disk[i].content.data == 0 && i != indexBlock)
        {
            disk[i].type = DATA_BLOCK_TYPE;
            disk[i].content.data = 1;
            disk[indexBlock].content.blockPtrs[allocated] = &disk[i];
            allocated++;
        }
    }

    if (allocated < blocks)
    {
        printf("\nNot enough free blocks\n");
        disk[indexBlock].content.data = 0;
        for (int i = 0; i < allocated; i++)
        {
            struct Block *blockPtr = disk[indexBlock].content.blockPtrs[i];
            if (blockPtr != NULL)
            {
                blockPtr->content.data = 0;
            }
        }
        return;
    }

    files[slot].name = strdup(name);
    files[slot].indexBlock = indexBlock;
    freeSpace -= (blocks + 1);

    printf("File inserted successfully\n");
}

void deleteFile(char *name)
{
    int pos = searchFile(name);
    if (pos == -1)
    {
        printf("\nFile not found\n");
        return;
    }

    int indexBlock = files[pos].indexBlock;
    struct Block *indexPtr = &disk[indexBlock];

    for (int i = 0; i < MAX_BLOCK_PTRS; i++)
    {
        if (indexPtr->content.blockPtrs[i] != NULL)
        {
            indexPtr->content.blockPtrs[i]->content.data = 0;
            indexPtr->content.blockPtrs[i] = NULL;
            freeSpace++;
        }
    }

    disk[indexBlock].content.data = 0;
    free(files[pos].name);
    files[pos].name = NULL;
    files[pos].indexBlock = -1;
    freeSpace++;

    printf("\nFile deleted successfully\n");
}

void displayFileInfo()
{
    char name[20];
    printf("\nEnter file name: ");
    getchar();
    fgets(name, 20, stdin);
    name[strcspn(name, "\n")] = '\0';

    int pos = searchFile(name);
    if (pos == -1)
    {
        printf("File not found!\n");
        return;
    }

    int indexBlock = files[pos].indexBlock;
    struct Block *indexPtr = &disk[indexBlock];

    int blockCount = 0;
    printf("\nFile: %s\n", files[pos].name);

    // Sequential Access Time
    clock_t start = clock();
    for (int i = 0; i < MAX_BLOCK_PTRS; i++)
    {
        if (indexPtr->content.blockPtrs[i] != NULL)
        {
            struct timespec delay;
            delay.tv_sec = 0;
            delay.tv_nsec = 5 * 1000000L; // 5ms
            nanosleep(&delay, NULL);
            blockCount++;
        }
    }
    clock_t end = clock();
    double sequentialAccessTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Sequential Access Time: %.2f ms\n", sequentialAccessTime);

    // Random Access Time
    printf("Enter target block index (0 to %d): ", blockCount - 1);
    int targetIndex;
    scanf("%d", &targetIndex);

    if (targetIndex < 0 || targetIndex >= blockCount)
    {
        printf("Invalid block index!\n");
        return;
    }

    start = clock();
    struct Block *targetBlock = indexPtr->content.blockPtrs[targetIndex];

    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 5 * 1000000L; // 5ms
    nanosleep(&delay, NULL);

    end = clock();
    double randomAccessTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    printf("Random Access Time to Block %d: %.2f ms\n", targetIndex, randomAccessTime);
    printf("===============================================\n");
}

void displayDisk()
{
    printf("\nDISK:\n");
    printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");

    for (int i = 0; i < maxsize; i++)
    {
        if (i % 10 == 0)
        {
            printf("\n%d\t", i);
        }
        if (disk[i].content.data == 0)
        {
            printf("0\t");
        }
        else
        {
            if (disk[i].type == DATA_BLOCK_TYPE)
            {
                printf("DB\t");
            }
            else if (disk[i].type == INDEX_BLOCK_TYPE)
            {
                printf("IB\t");
            }
        }
    }
    printf("\n");
}

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
            printf("[ ");
            for (int j = 0; j < MAX_BLOCK_PTRS; j++)
            {
                struct Block *blockPtr = disk[files[i].indexBlock].content.blockPtrs[j];
                if (blockPtr != NULL)
                {
                    printf("%ld ", blockPtr - disk); // Print block index by subtracting the base address of disk
                }
            }
            printf("]\n");
        }
    }
    printf("==========================================================\n");
}

int main()
{
    int option;
    char *name = malloc(20 * sizeof(char));
    int blocks;

    init();
    printf("Indexed File Allocation Technique Simulation\n\n");
    printf("1. Insert a File\n");
    printf("2. Delete a File\n");
    printf("3. Display Disk\n");
    printf("4. Display File Info (Access Times)\n");
    printf("5. Exit\n");

    while (1)
    {
        printf("\nEnter option: ");
        scanf("%d", &option);

        switch (option)
        {
        case 1:
            printf("Enter file name: ");
            getchar();
            fgets(name, 20, stdin);
            name[strcspn(name, "\n")] = '\0';
            printf("Enter number of blocks: ");
            scanf("%d", &blocks);
            insertFile(name, blocks);
            break;

        case 2:
            printf("Enter file name: ");
            getchar();
            fgets(name, 20, stdin);
            name[strcspn(name, "\n")] = '\0';
            deleteFile(name);
            break;

        case 3:
            displayDisk();
            break;
        case 4:
            displayFiles();
            break;

        case 5:
            displayFileInfo();
            break;

        case 6:
            free(name);
            exit(0);

        default:
            printf("Invalid option. Try again.\n");
        }
    }
}
