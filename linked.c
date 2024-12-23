#include <stdio.h>
#include <stdlib.h> // for malloc, exit
#include <string.h> // for strcpy, strcmp
#include <time.h>	// for clock and nanosleep

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
void displayFileInfo(void); // New function

struct block
{
	int value;
	struct block *next;
};

struct fileEntry
{
	char *name;
	int start;
	int end;
};

struct block disk[maxsize];
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
		disk[i].value = 0; // Disk is empty & blocks do not point to any other block
		disk[i].next = NULL;
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
		printf("\nCannot insert the file\n");
		return;
	}
	int start = -1;
	int i, allocated = 0;
	int prev = -1;
	for (i = 0; i < maxsize; i++)
	{
		if (disk[i].value == 0)
		{
			if (start == -1)
			{
				start = i;
			}
			disk[i].value = 1;
			if (prev != -1)
				disk[prev].next = &disk[i];
			allocated++;
			prev = i;
		}
		if (allocated == blocks)
		{
			disk[i].next = NULL;
			break;
		}
	}

	int slot = getEmptySlot();
	files[slot].name = malloc(strlen(name) + 1);
	strcpy(files[slot].name, name);
	files[slot].start = start;
	files[slot].end = i;
	freeSpace -= blocks;

	printf("\nFile '%s' inserted successfully\n", name);
}

void deleteFile(char *name)
{
	int pos;
	int size = 0;
	if ((pos = searchFile(name)) == -1)
	{
		printf("\nFile not found\n");
		return;
	}
	struct block *temp = &disk[files[pos].start];
	while (temp != NULL)
	{
		temp->value = 0;
		size++;
		temp = temp->next;
	}
	freeSpace += size;
	free(files[pos].name);
	files[pos].name = NULL;
	printf("\nFile '%s' deleted successfully\n", name);
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

void displaySize()
{
	printf("Free space in disk = %d\n", freeSpace);
}

void displayDisk()
{
	int i;
	printf("\nDISK:\n\n\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
	for (i = 0; i < maxsize; i++)
	{
		if (i % 10 == 0)
			printf("\n%d\t", i);
		printf("%d\t", disk[i].value);
	}
	printf("\n");
}

void displayFiles()
{
	int i;
	printf("\nFiles in disk:\n");
	printf("Name\tStart\tEnd\n\n");
	for (i = 0; i < 30; i++)
	{
		if (files[i].name != NULL)
		{
			printf("%s\t%4d\t%3d\n", files[i].name, files[i].start, files[i].end);
			printf("Blocks: %d -> ", files[i].start);
			struct block *temp = disk[files[i].start].next;
			while (temp != NULL)
			{
				printf("%ld -> ", (long int)(temp - disk));
				temp = temp->next;
			}
			printf("NULL\n");
		}
	}
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

	struct block *temp = &disk[files[pos].start];
	int blockCount = 0;
	printf("\nFile: %s\n", files[pos].name);

	// Sequential Access Time
	clock_t start = clock();
	while (temp != NULL)
	{
		temp = temp->next;
		blockCount++;

		// Giả sử mỗi block mất 5ms để truy cập
		struct timespec delay;
		delay.tv_sec = 0;
		delay.tv_nsec = 5 * 1000000L; // 5ms
		nanosleep(&delay, NULL);
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

	temp = &disk[files[pos].start];
	start = clock();
	for (int i = 0; i <= targetIndex; i++)
	{
		temp = temp->next;

		// Giả sử mỗi block mất 5ms để truy cập
		struct timespec delay;
		delay.tv_sec = 0;
		delay.tv_nsec = 5 * 1000000L; // 5ms
		nanosleep(&delay, NULL);
	}
	end = clock();
	double randomAccessTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

	printf("Random Access Time to Block %d: %.2f ms\n", targetIndex, randomAccessTime);
	printf("===============================================\n");
}

int main()
{
	int option;
	char *name = (char *)malloc(20 * sizeof(char));
	int blocks;
	init();
	printf("Linked File Allocation Technique\n\n");
	printf("\n1. Insert a File");
	printf("\n2. Delete a File");
	printf("\n3. Display the disk");
	printf("\n4. Display all files");
	printf("\n5. Display file information (access times and memory usage)");
	printf("\n6. Exit\n");

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
			displayFileInfo();
			break;
		case 6:
			free(name);
			exit(0);
		default:
			printf("Invalid option\n");
		}
	}
}
