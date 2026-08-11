#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
fields.csv → 전문별 C 헤더(struct) 생성.

- 필드명: 항목영문명을 snake_case(영문_단어_단어)로 정규화
- 레이아웃: KRX 전문은 고정폭 ASCII → 모든 필드 char[len] (수기 오프셋 계산 불필요)
- 검증: DATA부 길이 합계를 주석에 표기 (interface-list.csv의 '길이(헤더제외)'와 대조 가능)

사용:
  python3 gen_headers.py                 # 전 전문 → headers/ 에 개별 .h
  python3 gen_headers.py TCHODR10001 ... # 지정 TR만 (stdout 미리보기)
"""
import os, csv, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
FIELDS = os.path.join(HERE, "fields.csv")
OUTDIR = os.path.join(HERE, "headers")


def snake(en):
    """항목영문명 → 기존 코드 형식(Title_Case + 언더스코어).
    'MESSAGE_ / SEQUENCE / NUMBER' -> Message_Sequence_Number, 'ME_GRP_NO' -> Me_Grp_No
    (기존 struct 스타일: Transaction_Code, Order_Type, Board_id 등)"""
    s = en.replace(" / ", " ").replace("/", " ")
    parts = [p for p in re.split(r"[^0-9A-Za-z]+", s) if p]
    return "_".join(p.capitalize() for p in parts) or "Field"


def clen(length):
    """'11.0' -> 11 (없으면 0)"""
    try:
        return int(float(length))
    except (ValueError, TypeError):
        return 0


def load():
    with open(FIELDS, encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def block_for(tr, rows):
    """해당 TR이 속한 필드 블록 반환 (변형 공유 블록은 TR_ALL 멤버십으로 매칭)."""
    data = [r for r in rows if r["TR_CODE"] == tr]
    if data:
        return data
    for r in rows:
        if tr in (r.get("TR_ALL", "") or "").split(";"):
            prim = r["TR_CODE"]
            return [x for x in rows if x["TR_CODE"] == prim]
    return []


def gen_struct(tr, rows):
    data = block_for(tr, rows)
    if not data:
        return None, 0, 0
    iface = data[0]["인터페이스명"]
    body = [r for r in data if r["HDR_DATA"] == "DATA"]
    used, total = set(), 0
    lines = []
    for r in body:
        name = snake(r["항목영문명"]) or snake(r["항목명"])
        # 중복 방지
        base, n = name, 1
        while name in used:
            n += 1; name = f"{base}_{n}"
        used.add(name)
        ln = clen(r["길이"])
        total += ln
        note = (r["항목명"] + (" ★" + r["차세대변경"] if r["차세대변경"] else "")).strip()
        lines.append(f"    char {name}[{ln}];".ljust(48) + f"/* {r['순번']:>4} {note} ({r['DataType']} {ln}) */")
    struct_name = tr + "_DATA"
    hdr = []
    hdr.append(f"/* {tr} {iface} - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */")
    if data[0]["TR_CODE"] != tr:
        hdr.append(f"/* (변형: 레이아웃 공유 그룹 {data[0]['TR_CODE']}) */")
    hdr.append(f"/* DATA 길이 합계 = {total} (interface-list.csv '길이(헤더제외)'와 대조) */")
    hdr.append(f"typedef struct {{")
    hdr.extend(lines)
    hdr.append(f"}} {struct_name};")
    return "\n".join(hdr), total, len(body)


def main():
    rows = load()
    trs = sys.argv[1:]
    if trs:
        for tr in trs:
            s, total, n = gen_struct(tr, rows)
            print(s if s else f"/* {tr} not found */")
            print()
        return
    # 전체 생성: 카탈로그(interface-list) 기준 = 전문별 1개, 변형 각각 별도 .h
    os.makedirs(OUTDIR, exist_ok=True)
    cat_trs = []
    with open(os.path.join(HERE, "interface-list.csv"), encoding="utf-8-sig") as f:
        for r in csv.DictReader(f):
            tr = (r["TR_CODE"] or "").strip()
            biz = (r["업무구분"] or "").strip()
            if not tr or tr == "해당업무 TR" or biz.startswith("트랜잭션(예시)"):
                continue     # 예시/placeholder 제외
            cat_trs.append(tr)
    seen = 0
    for tr in dict.fromkeys(cat_trs):
        s, total, n = gen_struct(tr, rows)
        if not s or n == 0:
            continue     # DATA부 없는 세션전문 등은 struct 생략(HEADER 공용)
        with open(os.path.join(OUTDIR, f"{tr.lower()}.h"), "w", encoding="utf-8") as f:
            guard = "_KRX_" + tr.upper() + "_H"
            f.write(f"#ifndef {guard}\n#define {guard}\n\n{s}\n\n#endif  /* {guard} */\n")
        seen += 1
    print(f"{seen} 전문 헤더 생성 (전문별 1개, 변형 개별) -> {OUTDIR}/")


if __name__ == "__main__":
    main()
