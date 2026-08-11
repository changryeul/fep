#ifndef ALARM_H
#define ALARM_H

/*------------------------------------------------------------------------
#   OMS 알림(alarm) 송신 — libfepP 편입판 (2026-08-10)
#
#   alrt_msg(trcode, msg, msg_len): OMS 알림 메시지 큐(OMS_MSG_KEY)로
#     DATAHEAD(50B) + body 를 msgsnd. 성공 0 / 실패 -1.
#   원본 alarm/alarm.c 의 자립 구현(libc/POSIX only) — libfepP 편입 시
#     실패경로에 FEP Log() 관측성만 추가(내부 호출, 외부 의존성 무).
#   ※ 원본 alarm.h 는 alert_msg 로 오타 선언되어 있었음 → alrt_msg 로 정정.
------------------------------------------------------------------------*/

int     alrt_msg(int trcode, char *msg, int msg_len);

#endif  /* ALARM_H */
