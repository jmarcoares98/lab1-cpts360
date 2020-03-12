#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BASE_O 8 //OCTAL
#define BASE_U 10 //UNSIGNED
#define BASE_X 16 //HEX

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

char* ctable = "0123456789ABCDEF";
int* ip; //global pointer for myprintf


struct partition {
	u8 drive;             /* drive number FD=0, HD=0x80, etc. */

	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */

	u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */

	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
};

int rpu(u32 x, int base)
{
	char c;
	if (x)
	{
		c = ctable[x % base];
		1 + rpu(x / base, base);
		putchar(c);
	}
}

/*Prints a string*/
void prints(char* s)
{
	while (*s)
	{
		putchar(*s);
		s++;
	}
}

/*Prints unsigned integers*/
int printu(u32 x)
{
	(x == 0) ? putchar('0') : rpu(x, BASE_U);
	putchar(' ');
}

/*Prints an integer (Can be negative)*/
int printd(int x)
{
	if (x < 0)
	{
		putchar('-');
		x *= -1;
		printu(x);
	}
	else
	{
		printu(x);
	}
}

/*Prints x in HEX (Starts with 0x)*/
int printx(u32 x)
{
	putchar('0');
	putchar('x');
	(x == 0) ? putchar('0') : rpu(x, BASE_X);
}

/*Prints x in OCTAL (starts with 0)*/
int printo(u32 x)
{
	if (x == 0)
	{
		putchar('0');
	}
	(x == 0) ? putchar('0') : rpu(x, BASE_O);
}

//this is my print function
int myprintf(char* fmt, ...)
{
	char* cp = fmt;
	ip = (int*)& fmt + 1;

	while (*cp)
	{
		if (*cp != '%')
		{
			putchar(*cp);
			if (*cp == '\n')
			{
				putchar('\r');
			}
		}
		else
		{
			cp++;
			switch (*cp)
			{
			case 'c': putchar(*ip);
				break;
			case 's': prints(*ip);
				break;
			case 'u': printu(*ip);
				break;
			case 'd': printd(*ip);
				break;
			case 'o': printo(*ip);
				break;
			case 'x': printx(*ip);
				break;
			}
			ip++;
		}
		cp++;
	}
}

int main(int argc, char *argv[], char *env[])
{
	int count = 1;
	int fd;
	char buf[512];
	int exit = 0;

	fd = open(argv[1], O_RDONLY);  // check fd value: -1 means open() failed
	read(fd, buf, 512);            // read sector 0 into buf[ ]

	struct partition* p = (struct partition*) & buf[0x1bE]; // MBR, start of disk

	myprintf("device		start		end		# sectors	system\n");

	//looping to show the 4 partitions 
	for (int i = 0; i < 4; i++)
	{
		//the extended partition
		if (p->sys_type == 5)
		{
			//printing out the partition which has p->sys_type of 5
			myprintf("vdisk%d:	%d		%d		%d		%d", count, p->start_sector, (p->start_sector + p->nr_sectors - 1), p->nr_sectors, p->sys_type);

			myprintf("	Extended\n");

			int p4bs = p->start_sector; //partition 4's begin sector setting it to the localMBR
			int nextbs = p4bs; //needed to get to the nexts partitions MBR

			lseek(fd, (long)(p4bs * 512), SEEK_SET);  // lseek to byte 123*512 OR sector 123
			read(fd, buf, 512);                  // read sector 123 into buf[ ]


			while(exit == 0)
			{	
				struct partition* p2 = (struct partition*) & buf[0x1bE]; //access to local p-table

				count++;
				myprintf("vdisk%d:	%d		%d		%d		%d\n", count, nextbs + p2->start_sector, (nextbs + p2->start_sector + p2->nr_sectors - 1), p2->nr_sectors, p2->sys_type);

				p2++;
				nextbs = p4bs + p2->start_sector;
				
				if (p2->start_sector == 0)
				{
					exit = 1; //exits the loops when the the link lists hits 0
				}

				lseek(fd, (long)(nextbs * 512), SEEK_SET);  // lseek to byte 123*512 OR sector 123
				read(fd, buf, 512);                  // read sector 123 into buf[ ]

			}

		}

		else
		{
			myprintf("vdisk%d:	%d		%d		%d		%d\n", count, p->start_sector,(p->start_sector + p->nr_sectors - 1), p->nr_sectors, p->sys_type);
		}
		p++;
		count++;
	}

	lseek(fd, (long)123 * 512, SEEK_SET);  // lseek to byte 123*512 OR sector 123
	read(fd, buf, 512);                  // read sector 123 into buf[ ]

	myprintf("\nTEST FOR MYPRINTF:\n");
	myprintf("char = %c string = %s      dec=%d hex=%x oct=%o neg=%d\n",
		'A', "this is a test", 100, 100, 100, -100);

	myprintf("\nprint argc and argv\n");
	myprintf("argc = %d\n", argc);

	for (int j = 0; j < argc; j++) {
		myprintf("argv[%d] = %s\n", j, argv[j]);
	}


	return 0;
}

