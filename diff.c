#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Read one line from file descriptor
int readline(int fd, char *buf, int max)
{
    int i = 0;
    char c;

    while (i < max - 1)
    {
        int n = read(fd, &c, 1);
        if (n <= 0)
        {
            if (i == 0)
                return -1;
            break;
        }
        if (c == '\n')
        {
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(2, "usage: diff file1 file2 [-q]\n");
        exit(1);
    }

    char *file1 = argv[1];
    char *file2 = argv[2];
    int quiet = 0;

    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-q") == 0)
        {
            quiet = 1;
            break;
        }
    }

    int fd1 = open(file1, 0);
    if (fd1 < 0)
    {
        fprintf(2, "diff: cannot open %s\n", file1);
        exit(1);
    }

    int fd2 = open(file2, 0);
    if (fd2 < 0)
    {
        fprintf(2, "diff: cannot open %s\n", file2);
        close(fd1);
        exit(1);
    }

    char buf1[512];
    char buf2[512];
    int linenum = 0;
    int differ = 0;
    int len1 = 0;
    int len2 = 0;

    while (1)
    {
        linenum++;

        len1 = readline(fd1, buf1, sizeof(buf1));
        len2 = readline(fd2, buf2, sizeof(buf2));

        if (len1 == -1 && len2 == -1)
        {
            break;
        }

        int line_differ = 0;

        if (len1 == -1 && len2 >= 0)
        {
            line_differ = 1;
        }
        else if (len1 >= 0 && len2 == -1)
        {
            line_differ = 1;
        }
        else if (len1 >= 0 && len2 >= 0)
        {
            if (strcmp(buf1, buf2) != 0)
            {
                line_differ = 1;
            }
        }

        if (line_differ)
        {
            differ = 1;

            if (quiet)
            {
                break;
            }

            if (len1 >= 0)
            {
                printf("f1:%d: < %s\n", linenum, buf1);
            }
            else
            {
                printf("f1:%d: < EOF\n", linenum);
            }

            if (len2 >= 0)
            {
                printf("f2:%d: > %s\n", linenum, buf2);
            }
            else
            {
                printf("f2:%d: > EOF\n", linenum);
            }
        }

        if (len1 == -1 || len2 == -1)
        {
            break;
        }
    }

    if (len2 == -1 && len1 >= 0)
    {
        int len1_remaining;
        while ((len1_remaining = readline(fd1, buf1, sizeof(buf1))) >= 0)
        {
            linenum++;
            differ = 1;

            if (quiet)
            {
                break;
            }

            printf("f1:%d: < %s\n", linenum, buf1);
            printf("f2:%d: > EOF\n", linenum);
        }
    }

    if (len1 == -1 && len2 >= 0)
    {
        int len2_remaining;
        while ((len2_remaining = readline(fd2, buf2, sizeof(buf2))) >= 0)
        {
            linenum++;
            differ = 1;

            if (quiet)
            {
                break;
            }

            printf("f1:%d: < EOF\n", linenum);
            printf("f2:%d: > %s\n", linenum, buf2);
        }
    }

    if (quiet && differ)
    {
        printf("diff: files differ\n");
    }

    close(fd1);
    close(fd2);
    exit(0);
}
