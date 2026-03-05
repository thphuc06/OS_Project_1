#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

static int opt_a = 0;

static void name_from_dirent(char *out, struct dirent *de)
{
    memmove(out, de->name, DIRSIZ);
    out[DIRSIZ] = '\0';
}

static int join_path(char *out, int outsz, const char *parent, const char *child)
{
    int lp = strlen(parent);
    int lc = strlen(child);
    if (lp + 1 + lc + 1 > outsz)
        return -1;
    memmove(out, parent, lp);
    out[lp] = '/';
    memmove(out + lp + 1, child, lc);
    out[lp + 1 + lc] = '\0';
    return 0;
}

// Phần 1: du(path) - CHỈ TÍNH tổng bytes, không in gì
static long du(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "du: cannot open %s\n", path);
        return 0;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "du: cannot stat %s\n", path);
        close(fd);
        return 0;
    }

    if (st.type != T_DIR)
    {
        close(fd);
        return st.size;
    }

    long total = 0;
    struct dirent de;
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;

        char name[DIRSIZ + 1];
        name_from_dirent(name, &de);
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char child[512];
        if (join_path(child, sizeof(child), path, name) < 0)
            continue;

        total += du(child);
    }

    close(fd);
    return total;
}

// Phần 2: print_du(path) - Duyệt và IN theo option -a
static void print_du(const char *path, int is_root)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "du: cannot open %s\n", path);
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "du: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR)
    {
        if (opt_a || is_root)
            printf("%ld\t%s\n", st.size, path);
        close(fd);
        return;
    }

    struct dirent de;
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;

        char name[DIRSIZ + 1];
        name_from_dirent(name, &de);
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char child[512];
        if (join_path(child, sizeof(child), path, name) < 0)
            continue;

        print_du(child, 0);
    }

    close(fd);
    printf("%ld\t%s\n", du(path), path);
}

int main(int argc, char *argv[])
{
    const char *path = ".";
    int opt_s = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j]; j++)
            {
                if (argv[i][j] == 'a')
                    opt_a = 1;
                else if (argv[i][j] == 's')
                    opt_s = 1;
                else
                {
                    fprintf(2, "usage: du [path] [-a] [-s]\n");
                    exit(1);
                }
            }
        }
        else
            path = argv[i];
    }

    if (opt_s)
        printf("%ld\t%s\n", du(path), path);
    else
        print_du(path, 1);

    exit(0);
}
