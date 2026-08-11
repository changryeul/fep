#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 256

int main(int argc, char *argv[]) {
    FILE *fp;
    char line[LINE_SIZE];
    char *filename = "data.txt";

    if (argc < 2) {
        printf("사용법: %s 검색어\n", argv[0]);
        return 1;
    }

    char *keyword = argv[1];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    while (fgets(line, sizeof(line), fp)) {
        // 개행 문자 제거
        line[strcspn(line, "\r\n")] = 0;

        // 검색어가 포함되어 있는지 확인
        if (strstr(line, keyword)) {
            printf("%s\n", line);
        }
    }

    fclose(fp);
    return 0;
}
