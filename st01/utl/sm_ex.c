#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHM_SIZE 1024  // Shared memory의 크기

int main() {
    key_t key;
    int shmid;
    char *shm_ptr;

    // 공유 메모리 키 생성
    key = ftok("/tmp", 'R');  // ftok는 공유 메모리를 위한 유니크한 키 생성

    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }

    // 공유 메모리 생성
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);  // 0666은 읽기/쓰기 권한
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // 공유 메모리 연결
    shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // "안녕하세요" 메시지를 공유 메모리에 작성
    strcpy(shm_ptr, "안녕하세요");

    // 출력 (공유 메모리에서 데이터 읽기)
    printf("Shared Memory에 저장된 메시지: %s\n", shm_ptr);

    // 공유 메모리 분리
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt failed");
        exit(1);
    }

    return 0;
}
