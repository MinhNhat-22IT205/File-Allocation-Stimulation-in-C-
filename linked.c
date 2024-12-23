#include <stdio.h>
#include <stdlib.h> // for malloc, exit
#include <string.h> // for strcpy, strcmp
#include <time.h>	// for clock and nanosleep

#define MAX_SIZE 100
#define MAX_FILES 30

// Function prototypes
void initializeDisk(void);
int findEmptyFileSlot(void);
int findFileIndex(char *fileName);
void insertFile(char *fileName, int blockCount);
void deleteFile(char *fileName);
void displayFreeSpace(void);
void displayDiskStatus(void);
void displayAllFiles(void);
void displayFileDetails(void);

struct Block
{
	int isOccupied;
	struct Block *next;
};

struct FileEntry
{
	char *fileName;
	int startBlock;
	int endBlock;
};

struct Block disk[MAX_SIZE];
int freeSpace = MAX_SIZE;
struct FileEntry fileTable[MAX_FILES];

void initializeDisk()
{
	for (int fileSlot = 0; fileSlot < MAX_FILES; fileSlot++)
	{
		fileTable[fileSlot].fileName = NULL; // No files initially
	}
	for (int blockIndex = 0; blockIndex < MAX_SIZE; blockIndex++)
	{
		disk[blockIndex].isOccupied = 0; // Disk is empty
		disk[blockIndex].next = NULL;
	}
}

int findEmptyFileSlot()
{
	for (int fileSlot = 0; fileSlot < MAX_FILES; fileSlot++)
	{
		if (fileTable[fileSlot].fileName == NULL)
		{
			return fileSlot;
		}
	}
	return -1;
}

void insertFile(char *fileName, int blockCount)
{
	if (blockCount > freeSpace)
	{
		printf("\nError: Not enough free space to insert the file.\n");
		return;
	}
	if (findFileIndex(fileName) != -1)
	{
		printf("\nError: File with the same name already exists.\n");
		return;
	}

	int startBlock = -1;
	int allocatedBlocks = 0;
	int previousBlock = -1;

	for (int blockIndex = 0; blockIndex < MAX_SIZE; blockIndex++)
	{
		if (disk[blockIndex].isOccupied == 0)
		{
			if (startBlock == -1)
			{
				startBlock = blockIndex;
			}
			disk[blockIndex].isOccupied = 1;

			if (previousBlock != -1)
			{
				disk[previousBlock].next = &disk[blockIndex];
			}
			allocatedBlocks++;
			previousBlock = blockIndex;
		}
		if (allocatedBlocks == blockCount)
		{
			disk[blockIndex].next = NULL;
			break;
		}
	}

	int fileSlot = findEmptyFileSlot();
	fileTable[fileSlot].fileName = malloc(strlen(fileName) + 1);
	strcpy(fileTable[fileSlot].fileName, fileName);
	fileTable[fileSlot].startBlock = startBlock;
	fileTable[fileSlot].endBlock = previousBlock;
	freeSpace -= blockCount;

	printf("\nFile '%s' inserted successfully.\n", fileName);
}

void deleteFile(char *fileName)
{
	int fileIndex = findFileIndex(fileName);
	if (fileIndex == -1)
	{
		printf("\nError: File not found.\n");
		return;
	}

	struct Block *currentBlock = &disk[fileTable[fileIndex].startBlock];
	int releasedBlocks = 0;

	while (currentBlock != NULL)
	{
		currentBlock->isOccupied = 0;
		releasedBlocks++;
		currentBlock = currentBlock->next;
	}

	freeSpace += releasedBlocks;
	free(fileTable[fileIndex].fileName);
	fileTable[fileIndex].fileName = NULL;

	printf("\nFile '%s' deleted successfully.\n", fileName);
}

int findFileIndex(char *fileName)
{
	for (int fileSlot = 0; fileSlot < MAX_FILES; fileSlot++)
	{
		if (fileTable[fileSlot].fileName != NULL && strcmp(fileTable[fileSlot].fileName, fileName) == 0)
		{
			return fileSlot;
		}
	}
	return -1;
}

void displayFreeSpace()
{
	printf("Free space in disk: %d blocks\n", freeSpace);
}

void displayDiskStatus()
{
	printf("\nDisk Status:\n\n\t");
	for (int blockIndex = 0; blockIndex < 10; blockIndex++)
	{
		printf("%d\t", blockIndex);
	}
	printf("\n");

	for (int blockIndex = 0; blockIndex < MAX_SIZE; blockIndex++)
	{
		if (blockIndex % 10 == 0)
		{
			printf("\n%d\t", blockIndex);
		}
		printf("%d\t", disk[blockIndex].isOccupied);
	}
	printf("\n");
}

void displayAllFiles()
{
	printf("\nFiles on disk:\n");
	printf("Name\tStart\tEnd\n\n");
	for (int fileSlot = 0; fileSlot < MAX_FILES; fileSlot++)
	{
		if (fileTable[fileSlot].fileName != NULL)
		{
			printf("%s\t%4d\t%3d\n", fileTable[fileSlot].fileName, fileTable[fileSlot].startBlock, fileTable[fileSlot].endBlock);
			printf("Blocks: %d -> ", fileTable[fileSlot].startBlock);

			struct Block *currentBlock = disk[fileTable[fileSlot].startBlock].next;
			while (currentBlock != NULL)
			{
				printf("%ld -> ", (long int)(currentBlock - disk));
				currentBlock = currentBlock->next;
			}
			printf("NULL\n");
		}
	}
}

void displayFileDetails()
{
	char fileName[20];
	printf("\nEnter file name: ");
	getchar();
	fgets(fileName, 20, stdin);
	fileName[strcspn(fileName, "\n")] = '\0';

	int fileIndex = findFileIndex(fileName);
	if (fileIndex == -1)
	{
		printf("Error: File not found.\n");
		return;
	}

	struct Block *currentBlock = &disk[fileTable[fileIndex].startBlock];
	int blockCount = 0;
	printf("\nFile: %s\n", fileTable[fileIndex].fileName);

	// Sequential Access Time
	clock_t startTime = clock();
	while (currentBlock != NULL)
	{
		currentBlock = currentBlock->next;
		blockCount++;

		struct timespec delay = {0, 5 * 1000000L}; // 5ms
		nanosleep(&delay, NULL);
	}
	clock_t endTime = clock();
	double sequentialTime = ((double)(endTime - startTime)) / CLOCKS_PER_SEC * 1000;
	printf("Sequential Access Time: %.2f ms\n", sequentialTime);

	// Random Access Time
	printf("Enter target block index (0 to %d): ", blockCount - 1);
	int targetIndex;
	scanf("%d", &targetIndex);

	if (targetIndex < 0 || targetIndex >= blockCount)
	{
		printf("Invalid block index!\n");
		return;
	}

	currentBlock = &disk[fileTable[fileIndex].startBlock];
	startTime = clock();
	for (int index = 0; index <= targetIndex; index++)
	{
		currentBlock = currentBlock->next;

		struct timespec delay = {0, 5 * 1000000L}; // 5ms
		nanosleep(&delay, NULL);
	}
	endTime = clock();
	double randomTime = ((double)(endTime - startTime)) / CLOCKS_PER_SEC * 1000;

	printf("Random Access Time to Block %d: %.2f ms\n", targetIndex, randomTime);
	printf("===============================================\n");
}

int main()
{
	int choice;
	char *fileName = malloc(20 * sizeof(char));
	int blockCount;
	initializeDisk();

	printf("Linked File Allocation Technique\n");
	printf("\n1. Insert a File");
	printf("\n2. Delete a File");
	printf("\n3. Display Disk Status");
	printf("\n4. Display All Files");
	printf("\n5. Display File Details");
	printf("\n6. Exit\n");

	while (1)
	{
		displayFreeSpace();
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
			fgets(fileName, 20, stdin);
			fileName[strcspn(fileName, "\n")] = '\0';
			deleteFile(fileName);
			break;
		case 3:
			displayDiskStatus();
			break;
		case 4:
			displayAllFiles();
			break;
		case 5:
			displayFileDetails();
			break;
		case 6:
			free(fileName);
			exit(0);
		default:
			printf("Invalid choice. Please try again.\n");
		}
	}
}
