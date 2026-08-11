#ifndef STRATEGY_LETTER_H
#define STRATEGY_LETTER_H

/*------------------------------------------------------------------------
 *  전략번호(ApType, 4자리) → 시장 letter 매핑 (시장 재편 A안 §5.2)
 *
 *  po_9000_mp Start_Client가 전략 기동 시 cp할 TYPE 바이너리의 letter를
 *  결정한다. 기존 `pa_` 하드코딩(걸림돌 ①)을 대체 — 단일시장 전략은 그 시장
 *  letter(채권 LP=b), 크로스마켓 전략은 OMS 코어(차익=o).
 *
 *  매핑은 밴드(prefix) 테이블 + longest-match + 기본값('o'):
 *    - 같은 시장에 LP/전략이 추가돼도 그 시장 밴드에 흡수(무변경 수용).
 *    - 신규 시장 밴드만 테이블에 1행 추가. (후일 config-backed 확장 가능)
 *
 *  반환: 시장 letter 문자('a'현물 'b'채권 'c'파생 'o'OMS…). 매칭 없으면 기본 'o'.
 *------------------------------------------------------------------------*/
char    Strategy_Letter(const char *aptype);

#endif  /* STRATEGY_LETTER_H */
