#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // To measure access time

#define MAX_DISK_SIZE 100
#define MAX_FILES 30

struct FileEntry
{
    char *fileName;
    int startBlock;
    int blockLength;
};

struct DiskBlock
{
    int status; // 0 for free, 1 for used
};

struct DiskBlock disk[MAX_DISK_SIZE];
int availableBlocks = MAX_DISK_SIZE;
struct FileEntry fileEntries[MAX_FILES];

void initializeDisk();
int findEmptyFileSlot();
int findFileIndex(const char *fileName);
void insertFile(const char *fileName, int blockCount);
void deleteFile(const char *fileName);
void displayDiskUsage();
void displayDiskMap();
void displayFiles();
void displayFileAccessTime();

void initializeDisk()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        fileEntries[i].fileName = NULL;
    }
    for (int i = 0; i < MAX_DISK_SIZE; i++)
    {
        disk[i].status = 0; // Mark all blocks as free
    }
}

int findEmptyFileSlot()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (fileEntries[i].fileName == NULL)
        {
            return i;
        }
    }
    return -1;
}

int findFileIndex(const char *fileName)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (fileEntries[i].fileName != NULL && strcmp(fileEntries[i].fileName, fileName) == 0)
        {
            return i;
        }
    }
    return -1;
}

void insertFile(const char *fileName, int blockCount)
{
    if (blockCount > availableBlocks)
    {
        printf("\nFile size too large.\n");
        return;
    }

    if (findFileIndex(fileName) != -1)
    {
        printf("\nFile already exists.\n");
        return;
    }

    int contiguousFreeBlocks = 0;
    int startIndex = -1;
    for (int i = 0; i < MAX_DISK_SIZE; i++)
    {
        if (disk[i].status == 0)
        {
            contiguousFreeBlocks++;
        }
        else
        {
            contiguousFreeBlocks = 0;
        }

        if (contiguousFreeBlocks == blockCount)
        {
            startIndex = i - blockCount + 1;
            break;
        }
    }

    if (startIndex == -1)
    {
        printf("\nNot enough contiguous space to insert the file.\n");
        return;
    }

    int fileSlot = findEmptyFileSlot();
    if (fileSlot == -1)
    {
        printf("\nNo available file slot.\n");
        return;
    }

    fileEntries[fileSlot].fileName = malloc(strlen(fileName) + 1); // Allocate memory for the file name
    strcpy(fileEntries[fileSlot].fileName, fileName);
    fileEntries[fileSlot].startBlock = startIndex;
    fileEntries[fileSlot].blockLength = blockCount;
    availableBlocks -= blockCount;

    for (int i = startIndex; i < startIndex + blockCount; i++)
    {
        disk[i].status = 1; // Mark blocks as used
    }

    printf("\nFile '%s' inserted successfully.\n", fileName);
    printf("Location: Blocks %d to %d\n", startIndex, startIndex + blockCount - 1);
}

void deleteFile(const char *fileName)
{
    int fileIndex = findFileIndex(fileName);
    if (fileIndex == -1)
    {
        printf("\nFile not found.\n");
        return;
    }

    int startBlock = fileEntries[fileIndex].startBlock;
    int blockLength = fileEntries[fileIndex].blockLength;

    for (int i = startBlock; i < startBlock + blockLength; i++)
    {
        disk[i].status = 0; // Mark blocks as free
    }

    availableBlocks += blockLength;
    free(fileEntries[fileIndex].fileName);
    fileEntries[fileIndex].fileName = NULL;

    printf("\nFile '%s' deleted successfully.\n", fileName);
    printf("Freed %d blocks starting from block %d.\n", blockLength, startBlock);
}

void displayDiskUsage()
{
    printf("\n================== DISK INFO ==================\n");
    printf("Total size: %d blocks\n", MAX_DISK_SIZE);
    printf("Free space: %d blocks\n", availableBlocks);
    printf("Used space: %d blocks\n", MAX_DISK_SIZE - availableBlocks);
    printf("===============================================\n");
}

void displayDiskMap()
{
    printf("\n=================== DISK MAP ===================\n");
    printf("     ");
    for (int j = 0; j < 10; j++)
    {
        printf("%4d ", j);
    }
    printf("\n");

    for (int i = 0; i < MAX_DISK_SIZE; i++)
    {
        if (i % 10 == 0)
        {
            printf("\n%3d  ", i);
        }
        printf("[%2d] ", disk[i].status);
    }
    printf("\n===============================================\n");
}

void displayFiles()
{
    printf("\n================ FILES IN DISK ================\n");
    printf("%-20s %-10s %-10s %-s\n", "File Name", "Start", "Length", "Blocks");
    printf("-----------------------------------------------\n");

    for (int i = 0; i < MAX_FILES; i++)
    {
        if (fileEntries[i].fileName != NULL)
        {
            printf("%-20s %-10d %-10d [ ",
                   fileEntries[i].fileName,
                   fileEntries[i].startBlock,
                   fileEntries[i].blockLength);

            for (int j = fileEntries[i].startBlock; j < fileEntries[i].startBlock + fileEntries[i].blockLength; j++)
            {
                printf("%d ", j);
            }
            printf("]\n");
        }
    }
    printf("===============================================\n\n");
}

void displayFileAccessTime()
{
    char fileName[20];
    printf("Enter file name: ");
    getchar();
    fgets(fileName, 20, stdin);
    fileName[strcspn(fileName, "\n")] = '\0';

    int fileIndex = findFileIndex(fileName);
    if (fileIndex == -1)
    {
        printf("File not found!\n");
        return;
    }

    int startBlock = fileEntries[fileIndex].startBlock;
    int length = fileEntries[fileIndex].blockLength;
    printf("\nFile: %s\n", fileEntries[fileIndex].fileName);
    printf("Start Block: %d\n", startBlock);
    printf("Length: %d blocks\n", length);

    // Measure sequential access time
    clock_t start = clock();
    for (int i = startBlock; i < startBlock + length; i++)
    {
        struct timespec delay = {0, 5 * 1000000L}; // 5ms
        nanosleep(&delay, NULL);
    }
    clock_t end = clock();
    double sequentialAccessTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    printf("Sequential Access Time: %.2f ms\n", sequentialAccessTime);

    // Measure random access time
    int targetBlock;
    printf("Enter target block index (relative to file start, 0 to %d): ", length - 1);
    scanf("%d", &targetBlock);

    if (targetBlock < 0 || targetBlock >= length)
    {
        printf("Invalid block index!\n");
        return;
    }

    int targetAbsoluteBlock = startBlock + targetBlock;
    start = clock();
    for (int i = startBlock; i <= targetAbsoluteBlock; i++)
    {
        struct timespec delay = {0, 5 * 1000000L}; // 5ms
        nanosleep(&delay, NULL);
    }
    end = clock();
    double randomAccessTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    printf("Random Access Time to Block %d (absolute index: %d): %.2f ms\n",
           targetBlock, targetAbsoluteBlock, randomAccessTime);
}

int main()
{
    int choice;
    char fileName[20];
    int blockCount;

    initializeDisk();
    printf("Sequential File Allocation Technique\n\n");
    printf("\n1. Insert a File");
    printf("\n2. Delete a File");
    printf("\n3. Display the Disk");
    printf("\n4. Display All Files");
    printf("\n5. Display File Access Time");
    printf("\n6. Exit\n");

    while (1)
    {
        displayDiskUsage();
        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("Enter file name: ");
            getchar();
            fgets(fileName, 20, stdin);
            fileName[strcspn(fileName, "\n")] = '\0';
            printf("Enter number of blocks: ");
            scanf("%d", &blockCount);
            insertFile(fileName, blockCount);
            break;
        case 2:
            printf("Enter file name to delete: ");
            getchar();
            fgets(fileName, 20, stdin);               // Read file name from user 20 characters
            fileName[strcspn(fileName, "\n")] = '\0'; // Remove newline character in the input
            deleteFile(fileName);
            break;
        case 3:
            displayDiskMap();
            break;
        case 4:
            displayFiles();
            break;
        case 5:
            displayFileAccessTime();
            break;
        case 6:
            exit(0);
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}