#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define maxsize 100

struct fileEntry
{
    char *name;
    int start;
    int length;
} fileEntry;

struct block
{
    int data; // 0 for free, 1 for used
};

struct block disk[maxsize];
int freeSpace = maxsize;
struct fileEntry files[30];

void init(void);
int getEmptySlot(void);
int searchFile(char *name);
void insertFile(char *name, int blocks);
void deleteFile(char *name);
void displaySize(void);
void displayDisk(void);
void displayFiles(void);

void init()
{
    int i;
    for (i = 0; i < 30; i++)
    {
        files[i].name = NULL;
    }

    for (int i = 0; i < maxsize; i++)
    {
        disk[i].data = 0; // Initialize all blocks as free
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

int searchFile(char *name)
{
    int i;
    for (i = 0; i < 30; i++)
    {
        if (files[i].name != NULL && strcmp(files[i].name, name) == 0)
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
        printf("\nCannot insert the file\n");
        return;
    }
    int i, j, free = 0;
    for (i = 0; i < maxsize; i++)
    {
        if (disk[i].data == 0)
            free++;
        else
            free = 0;
        if (free == blocks)
        {
            break;
        }
    }
    if (free != blocks)
    {
        printf("\nCannot insert the file\n");
        return;
    }
    int temp = i;
    i -= blocks - 1;
    int slot = getEmptySlot();
    files[slot].name = malloc(strlen(name) + 1);
    strcpy(files[slot].name, name);
    files[slot].length = blocks;
    files[slot].start = i;
    freeSpace -= blocks;

    for (; i <= temp; i++)
    {
        disk[i].data = 1; // Mark blocks as used
    }

    printf("\nFile '%s' inserted successfully\n", name);
    printf("Location: Blocks %d to %d\n", files[slot].start, files[slot].start + files[slot].length - 1);
}
void deleteFile(char *name)
{
    int pos;
    if ((pos = searchFile(name)) == -1)
    {
        printf("\nFile not found\n");
        return;
    }

    for (int i = files[pos].start; i < (files[pos].start + files[pos].length); i++)
    {
        disk[i].data = 0; // Mark blocks as free
    }

    freeSpace += files[pos].length;
    free(files[pos].name);
    files[pos].name = NULL;

    printf("\nFile '%s' deleted successfully\n", name);
    printf("Freed %d blocks starting from block %d\n",
           files[pos].length, files[pos].start);
}
void displaySize()
{
    printf("\n================== DISK INFO ==================\n");
    printf("Total size: %d blocks\n", maxsize);
    printf("Free space: %d blocks\n", freeSpace);
    printf("Used space: %d blocks\n", maxsize - freeSpace);
    printf("===============================================\n");
}
void displayDisk()
{
    printf("\n=================== DISK MAP ===================\n");
    printf("     ");
    for (int j = 0; j < 10; j++)
        printf("%4d ", j);
    printf("\n");

    for (int i = 0; i < maxsize; i++)
    {
        if (i % 10 == 0)
        {
            printf("\n%3d  ", i);
        }
        printf("[%2d] ", disk[i].data);
    }
    printf("\n");
    printf("===============================================\n");
}
void displayFiles()
{
    printf("\n================ FILES IN DISK ================\n");
    printf("%-20s %-10s %-10s %-s\n", "File Name", "Start", "Length", "Blocks");
    printf("-----------------------------------------------\n");

    for (int i = 0; i < 30; i++)
    {
        if (files[i].name != NULL)
        {
            printf("%-20s %-10d %-10d [ ",
                   files[i].name,
                   files[i].start,
                   files[i].length);

            // Print the actual blocks used by the file
            for (int j = files[i].start; j < files[i].start + files[i].length; j++)
            {
                printf("%d ", j);
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
    int blocks, start;
    init();
    printf("Sequential File allocation technique\n\n");
    printf("\n1. Insert a File");
    printf("\n2. Delete a File");
    printf("\n3. Display the disk");
    printf("\n4. Display all files");
    printf("\n5. Exit\n");
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