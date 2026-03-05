#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void producer(int fd_src, int p_write) {
  char buf[512];
  int n;
  while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
    if (write(p_write, buf, n) != n) {
      fprintf(2, "cp: pipe write error\n");
      exit(1);
    }
  }
  close(fd_src);
  close(p_write);
  exit(0);
}

void consumer(int p_read, int fd_dst) {
  char buf[512];
  int n;
  while ((n = read(p_read, buf, sizeof(buf))) > 0) {
    if (write(fd_dst, buf, n) != n) {
      fprintf(2, "cp: disk write error\n");
      exit(1);
    }
  }
  close(p_read);
  close(fd_dst);
  exit(0);
}

int main(int argc, char *argv[]) {
  int fd_src, fd_dst;
  int p[2];

  if (argc != 3) {
    fprintf(2, "usage: cp src dst\n");
    exit(1);
  }

  if ((fd_src = open(argv[1], O_RDONLY)) < 0) {
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  if ((fd_dst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC)) < 0) {
    fprintf(2, "cp: cannot create %s\n", argv[2]);
    close(fd_src);
    exit(1);
  }

  // Tạo pipe
  if (pipe(p) < 0) {
    fprintf(2, "cp: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "cp: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // tiến trình con: Đóng đầu đọc, đóng file đích, chỉ làm nhiệm vụ producer
    close(p[0]);
    close(fd_dst);
    producer(fd_src, p[1]);
  } else {
    // tiến trình cha: Đóng đầu ghi, đóng file nguồn, chỉ làm nhiệm vụ consumer
    close(p[1]);
    close(fd_src);
    consumer(p[0], fd_dst);
    wait(0); // đợi con hoàn thành
  }

  exit(0);
}
