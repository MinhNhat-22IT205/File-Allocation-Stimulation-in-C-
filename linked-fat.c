#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define maxsize 100

// Function prototypes
void init(void);
int getEmptySlot(void);
int searchFile(char *name);
void insertFile(char *name, int blocks);
void deleteFile(char *name);
void displaySize(void);
void displayDisk(void);
void displayFiles(void);
void displayFAT(void);

// Block structure for visualization
struct block
{
    int data; // 0=free, 1=occupied
};

// File entry structure
struct fileEntry
{
    char *name;
    int start;  // Starting block number
    int blocks; // Number of blocks
};

// Global variables
struct block disk[maxsize]; // Disk blocks
int FAT[maxsize];           // File Allocation Table (-1=EOF, -2=free, otherwise points to next block)
int freeSpace = maxsize;
struct fileEntry files[30];

void init()
{
    int i;
    for (i = 0; i < 30; i++)
    {
        files[i].name = NULL; // No files present
    }
    for (i = 0; i < maxsize; i++)
    {
        disk[i].data = 0; // Disk is Empty
        FAT[i] = -2;      // All blocks are free
    }
}

int getEmptySlot()
{
    int i;
    for (i = 0; i < 30; i++)
    {
        if (files[i].name == NULL)
            return i;
    }
    return -1;
}

void insertFile(char *name, int blocks)
{
    if (blocks > freeSpace)
    {
        printf("\nFile size too big\n");
        return;
    }
    if (searchFile(name) != -1)
    {
        printf("\nFile already exists\n");
        return;
    }

    int start = -1;
    int prev = -1;
    int allocated = 0;

    // Find and link free blocks
    for (int i = 0; i < maxsize && allocated < blocks; i++)
    {
        if (FAT[i] == -2)
        { // If block is free
            if (start == -1)
            {
                start = i;
            }
            disk[i].data = 1; // Mark block as occupied

            if (prev != -1)
            {
                FAT[prev] = i; // Link previous block to current
            }

            FAT[i] = -1; // Mark as end of file for now
            prev = i;
            allocated++;
        }
    }

    if (allocated == blocks)
    {
        int slot = getEmptySlot();
        files[slot].name = malloc(strlen(name) + 1);
        strcpy(files[slot].name, name);
        files[slot].start = start;
        files[slot].blocks = blocks;
        freeSpace -= blocks;
        printf("File inserted successfully\n");
    }
}

void deleteFile(char *name)
{
    int pos = searchFile(name);
    if (pos == -1)
    {
        printf("\nFile not found\n");
        return;
    }

    int current = files[pos].start;
    int next;

    // Follow FAT chain and free blocks
    while (current != -1)
    {
        next = FAT[current];
        disk[current].data = 0; // Mark block as free
        FAT[current] = -2;      // Mark block as free in FAT
        current = next;
    }

    freeSpace += files[pos].blocks;
    free(files[pos].name);
    files[pos].name = NULL;
    printf("File deleted successfully\n");
}

int searchFile(char *name)
{
    for (int i = 0; i < 30; i++)
    {
        if (files[i].name != NULL && strcmp(files[i].name, name) == 0)
            return i;
    }
    return -1;
}

void displaySize()
{
    printf("\nFree space in disk = %d blocks\n", freeSpace);
}

void displayDisk()
{
    printf("\nDISK STATUS:\n");
    printf("\n\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
    for (int i = 0; i < maxsize; i++)
    {
        if (i % 10 == 0)
            printf("\n%d\t", i);
        printf("%d\t", disk[i].data);
    }
    printf("\n");
}

void displayFAT()
{
    printf("\nFILE ALLOCATION TABLE:\n");
    printf("\n\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
    for (int i = 0; i < maxsize; i++)
    {
        if (i % 10 == 0)
            printf("\n%d\t", i);
        printf("%d\t", FAT[i]);
    }
    printf("\n");
}

void displayFiles()
{
    printf("\nFILES IN DISK:\n");
    printf("Name\tStart\tBlocks\tBlock Chain\n");
    printf("----------------------------------------\n");

    for (int i = 0; i < 30; i++)
    {
        if (files[i].name != NULL)
        {
            printf("%s\t%d\t%d\t", files[i].name, files[i].start, files[i].blocks);

            // Print block chain
            int current = files[i].start;
            printf("%d", current);
            while (FAT[current] != -1)
            {
                printf(" -> %d", FAT[current]);
                current = FAT[current];
            }
            printf(" -> NULL\n");
        }
    }
    printf("\n");
}

int main()
{
    char *name = (char *)malloc(20 * sizeof(char));
    int blocks, option;

    init();
    printf("Linked File Allocation with FAT\n\n");
    printf("1. Insert a File\n");
    printf("2. Delete a File\n");
    printf("3. Display Disk Status\n");
    printf("4. Display Files\n");
    printf("5. Display FAT\n");
    printf("6. Exit\n");

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
            name[strcspn(name, "\n")] = 0; // Remove trailing newline
            printf("Enter number of blocks: ");
            scanf("%d", &blocks);
            insertFile(name, blocks);
            break;

        case 2:
            printf("Enter file name to delete: ");
            getchar();
            fgets(name, 20, stdin);
            name[strcspn(name, "\n")] = 0; // Remove trailing newline
            deleteFile(name);
            break;

        case 3:
            displayDisk();
            break;

        case 4:
            displayFiles();
            break;

        case 5:
            displayFAT();
            break;

        case 6:
            free(name);
            exit(0);

        default:
            printf("Invalid option\n");
        }
    }
    return 0;
}
