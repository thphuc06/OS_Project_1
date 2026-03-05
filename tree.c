#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/fs.h"


static void name_from_dirent(char *out, struct dirent *de) {
    memmove(out, de->name, DIRSIZ);
    out[DIRSIZ] = '\0';
}


static int join_path(char *out, int outsz, const char *parent, const char *child) {
    int lp = strlen(parent);
    int lc = strlen(child);
    if (lp + 1 + lc + 1 > outsz) return -1;
    memmove(out, parent, lp);
    out[lp] = '/';
    memmove(out + lp + 1, child, lc);
    out[lp + 1 + lc] = '\0';
    return 0;
}


int is_dot_or_dotdot(char *name) {
    return (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}


// Đếm số mục sẽ in để xác định mục cuối cùng
int count_valid_entries(char *path, int only_dir) {
    int fd, count = 0;
    struct dirent de;
    struct stat st;
    char name[DIRSIZ + 1], full[512];

    if((fd = open(path, 0)) < 0) return 0;
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
        if(de.inum == 0) continue;
        name_from_dirent(name, &de);
        if(is_dot_or_dotdot(name)) continue;
        if(only_dir) {
            join_path(full, 512, path, name);
            if(stat(full, &st) < 0 || st.type != T_DIR) continue;
        }
        count++;
    }
    close(fd);
    return count;
}


void print_node(int level, char *display_name, int is_last, int mask) {
    if (level == 0) {
        printf("%s\n", display_name);
        return;
    }

    // Duyệt qua các tầng cha để in tiền tố
    for (int i = 1; i < level; i++) {
        // Kiểm tra bit thứ i trong mask để quyết định in vạch đứng hay khoảng trắng
        if (mask & (1 << i)) {
            printf("    "); // Tầng cha là nhánh cuối thì để trống
        } else {
            printf("│   "); // Tầng cha chưa hết nhánh thì in vạch đứng
        }
    }

    // In ký hiệu nhánh cho tầng hiện tại
    if (is_last) {
        printf("└── %s\n", display_name);
    } else {
        printf("├── %s\n", display_name);
    }
}


void tree(char *path, char *display_name, int level, int max_depth, int only_dir, int is_last, int mask) {
    // Gọi hàm in node
    print_node(level, display_name, is_last, mask);

    // Cập nhật Mask cho các tầng con: 
    // Nếu tầng hiện tại (level) là mục cuối, bật bit tương ứng trong mask lên 1
    if (is_last) {
        mask |= (1 << level);
    } else {
        mask &= ~(1 << level);
    }

    if (level >= max_depth) return;

    int fd;
    struct stat st;
    if ((fd = open(path, O_RDONLY)) < 0) return;
    if (fstat(fd, &st) < 0 || st.type != T_DIR) {
        close(fd);
        return;
    }

    // Duyệt thư mục con
    int total = count_valid_entries(path, only_dir);
    int current = 0;
    struct dirent de;
    char name[DIRSIZ + 1], child_path[512];

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;
        name_from_dirent(name, &de);
        if (is_dot_or_dotdot(name)) continue;

        if (join_path(child_path, 512, path, name) < 0) continue;

        struct stat st_child;
        if (stat(child_path, &st_child) < 0) continue;
        if (only_dir && st_child.type != T_DIR) continue;

        current++;
        int child_is_last = (current == total);

        // đệ quy 
        tree(child_path, name, level + 1, max_depth, only_dir, child_is_last, mask);
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    char *path = ".";
    int max_depth = 999;
    int only_dir = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) only_dir = 1;
        else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) max_depth = atoi(argv[++i]);
        else if (argv[i][0] != '-') path = argv[i];
    }

    tree(path, path, 0, max_depth, only_dir, 1, 0);

    exit(0);
}
