#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXSIZE 100
#define MAX_FILES 30
#define DIRECT_BLOCKS 10
#define INDIRECT_BLOCKS 1

// Block Types
#define DATA_BLOCK 0
#define INODE_BLOCK 1
#define INDIRECT_BLOCK 2

struct block
{
    int type; // 0: data, 1: inode, 2: indirect
    int data; // 0: free, 1: used
};

struct inode
{
    char *name;
    int size;                  // Size in blocks
    time_t created;            // Creation time
    int direct[DIRECT_BLOCKS]; // Direct block pointers
    int indirect;              // Single indirect block pointer
    int used;                  // 0: free, 1: used
};

// Global variables
struct block disk[MAXSIZE];
struct inode inodes[MAX_FILES];
int freeSpace = MAXSIZE;

// Function prototypes
void init(void);
int getFreeBlock(void);
int getFreeInode(void);
int searchFile(char *name);
void insertFile(char *name, int blocks);
void deleteFile(char *name);
void displaySize(void);
void displayDisk(void);
void displayFiles(void);

void init()
{
    // Initialize disk blocks
    for (int i = 0; i < MAXSIZE; i++)
    {
        disk[i].type = DATA_BLOCK;
        disk[i].data = 0;
    }

    // Initialize inodes
    for (int i = 0; i < MAX_FILES; i++)
    {
        inodes[i].name = NULL;
        inodes[i].size = 0;
        inodes[i].used = 0;
        inodes[i].indirect = -1;
        for (int j = 0; j < DIRECT_BLOCKS; j++)
        {
            inodes[i].direct[j] = -1;
        }
    }
}

int getFreeBlock()
{
    for (int i = 0; i < MAXSIZE; i++)
    {
        if (disk[i].data == 0)
        {
            return i;
        }
    }
    return -1;
}

int getFreeInode()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (!inodes[i].used)
        {
            return i;
        }
    }
    return -1;
}

int searchFile(char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (inodes[i].used && inodes[i].name != NULL &&
            strcmp(inodes[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void insertFile(char *name, int blocks)
{
    if (blocks > freeSpace)
    {
        printf("\nError: Not enough free space (need %d blocks)\n", blocks);
        return;
    }

    if (searchFile(name) != -1)
    {
        printf("\nError: File already exists\n");
        return;
    }

    int inodeNum = getFreeInode();
    if (inodeNum == -1)
    {
        printf("\nError: No free inodes available\n");
        return;
    }

    // Calculate needed blocks
    int directNeeded = (blocks <= DIRECT_BLOCKS) ? blocks : DIRECT_BLOCKS;
    int indirectNeeded = (blocks > DIRECT_BLOCKS) ? blocks - DIRECT_BLOCKS : 0;
    int totalNeeded = blocks + (indirectNeeded > 0 ? 1 : 0); // +1 for indirect block

    if (totalNeeded > freeSpace)
    {
        printf("\nError: Not enough free space\n");
        return;
    }

    // Allocate direct blocks
    int allocated = 0;
    for (int i = 0; i < directNeeded; i++)
    {
        int block = getFreeBlock();
        if (block == -1)
        {
            printf("\nError: Failed to allocate blocks\n");
            // Cleanup
            for (int j = 0; j < allocated; j++)
            {
                disk[inodes[inodeNum].direct[j]].data = 0;
            }
            return;
        }
        disk[block].data = 1;
        disk[block].type = DATA_BLOCK;
        inodes[inodeNum].direct[i] = block;
        allocated++;
    }

    // Allocate indirect blocks if needed
    if (indirectNeeded > 0)
    {
        // Allocate indirect block
        int indirectBlock = getFreeBlock();
        if (indirectBlock == -1)
        {
            printf("\nError: Failed to allocate indirect block\n");
            // Cleanup
            for (int i = 0; i < allocated; i++)
            {
                disk[inodes[inodeNum].direct[i]].data = 0;
            }
            return;
        }
        disk[indirectBlock].data = 1;
        disk[indirectBlock].type = INDIRECT_BLOCK;
        inodes[inodeNum].indirect = indirectBlock;

        // Allocate data blocks pointed to by indirect block
        for (int i = 0; i < indirectNeeded; i++)
        {
            int block = getFreeBlock();
            if (block == -1)
            {
                printf("\nError: Failed to allocate blocks\n");
                // Cleanup
                for (int j = 0; j < allocated; j++)
                {
                    disk[inodes[inodeNum].direct[j]].data = 0;
                }
                disk[indirectBlock].data = 0;
                return;
            }
            disk[block].data = 1;
            disk[block].type = DATA_BLOCK;
            // Store the block number in the indirect block's data
            disk[indirectBlock].data = block;
            allocated++;
        }
    }

    // Setup inode
    inodes[inodeNum].name = strdup(name);
    inodes[inodeNum].size = blocks;
    inodes[inodeNum].created = time(NULL);
    inodes[inodeNum].used = 1;
    freeSpace -= totalNeeded;

    printf("\nFile '%s' inserted successfully\n", name);
    printf("Inode: %d\n", inodeNum);
}

void deleteFile(char *name)
{
    int inodeNum = searchFile(name);
    if (inodeNum == -1)
    {
        printf("\nError: File not found\n");
        return;
    }

    int blocksFreed = 0;

    // Free direct blocks
    for (int i = 0; i < DIRECT_BLOCKS && inodes[inodeNum].direct[i] != -1; i++)
    {
        disk[inodes[inodeNum].direct[i]].data = 0;
        disk[inodes[inodeNum].direct[i]].type = DATA_BLOCK;
        inodes[inodeNum].direct[i] = -1;
        blocksFreed++;
    }

    // Free indirect blocks
    if (inodes[inodeNum].indirect != -1)
    {
        // Free data blocks pointed to by indirect block
        int indirectBlock = inodes[inodeNum].indirect;
        for (int i = DIRECT_BLOCKS; i < inodes[inodeNum].size; i++)
        {
            int dataBlock = disk[indirectBlock].data;
            if (dataBlock != -1)
            {
                disk[dataBlock].data = 0;
                disk[dataBlock].type = DATA_BLOCK;
                blocksFreed++;
            }
        }
        // Free the indirect block itself
        disk[indirectBlock].data = 0;
        disk[indirectBlock].type = DATA_BLOCK;
        inodes[inodeNum].indirect = -1;
        blocksFreed++;
    }

    // Clear inode
    free(inodes[inodeNum].name);
    inodes[inodeNum].name = NULL;
    inodes[inodeNum].size = 0;
    inodes[inodeNum].used = 0;
    freeSpace += blocksFreed;

    printf("\nFile deleted successfully\n");
    printf("Freed %d blocks\n", blocksFreed);
}

void displaySize()
{
    printf("\n================== DISK INFO ==================\n");
    printf("Total size: %d blocks\n", MAXSIZE);
    printf("Free space: %d blocks\n", freeSpace);
    printf("Used space: %d blocks\n", MAXSIZE - freeSpace);
    printf("===============================================\n");
}

void displayDisk()
{
    printf("\n=================== DISK MAP ===================\n");
    printf("     ");
    for (int j = 0; j < 10; j++)
        printf("%4d ", j);
    printf("\n");

    for (int i = 0; i < MAXSIZE; i++)
    {
        if (i % 10 == 0)
        {
            printf("\n%3d  ", i);
        }
        if (disk[i].data == 0)
        {
            printf("[--] ");
        }
        else
        {
            printf("[%2d] ", disk[i].type);
        }
    }
    printf("\n");
    printf("===============================================\n");
}

void displayFiles()
{
    printf("\n================ FILES IN DISK ================\n");
    printf("%-20s %-8s %-20s %-s\n", "File Name", "Size", "Created", "Blocks");
    printf("-----------------------------------------------\n");

    for (int i = 0; i < MAX_FILES; i++)
    {
        if (inodes[i].used && inodes[i].name != NULL)
        {
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
                     localtime(&inodes[i].created));

            printf("%-20s %-8d %-20s [ ",
                   inodes[i].name,
                   inodes[i].size,
                   timeStr);

            // Print direct blocks
            for (int j = 0; j < DIRECT_BLOCKS && inodes[i].direct[j] != -1; j++)
            {
                printf("%d ", inodes[i].direct[j]);
            }

            // Print indirect blocks
            if (inodes[i].indirect != -1)
            {
                printf("| %d: ", inodes[i].indirect);
                for (int j = DIRECT_BLOCKS; j < inodes[i].size; j++)
                {
                    printf("%d ", disk[inodes[i].indirect].data);
                }
            }
            printf("]\n");
        }
    }
    printf("===============================================\n\n");
}

int main()
{
    int option;
    char *name = (char *)malloc(20 * sizeof(char));
    int blocks;

    init();
    printf("Inode-based File Allocation Technique\n\n");
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
            name[strcspn(name, "\n")] = '\0';
            printf("Enter number of blocks: ");
            scanf("%d", &blocks);
            insertFile(name, blocks);
            break;

        case 2:
            printf("Enter file name to delete: ");
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
            free(name);
            exit(0);

        default:
            printf("Invalid option\n");
        }
    }
}
