#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
KRX EXTURE 3.0 시장접속 프로토콜 전문(.xlsb) → durable CSV 추출.

원본: KRX+EXTURE+3.0+시장접속+프로토콜_전문(회원사용)_v3.24_20260730_*.xlsb (repo 루트)
사용:  python3 -m venv /tmp/xlsbenv && /tmp/xlsbenv/bin/pip install pyxlsb
       /tmp/xlsbenv/bin/python docs/reference/krx-exture3/extract_exture.py <xlsb경로>
출력:  docs/reference/krx-exture3/interface-list.csv, fields.csv

.xlsb는 Excel 바이너리라 도구 의존적 → 이 스크립트로 뽑은 CSV를 repo에 고정해
이후 구현은 CSV만 참조(도구 불필요).
"""
import sys, os, csv
import pyxlsb

OUT = os.path.dirname(os.path.abspath(__file__))


def cell(c):
    if c is None or c.v is None:
        return ""
    v = c.v
    if isinstance(v, float) and v == int(v):
        v = int(v)
    return str(v).strip()   # 내부 개행(\n)은 보존 (다중 TR 분리에 사용)


def clean(s):
    return s.replace("\r", " ").replace("\n", " / ").strip()


def extract_interface_list(wb):
    with wb.get_sheet("회원사인터페이스목록") as sh:
        rows = [[cell(c) for c in r] for r in sh.rows()]
    # 데이터는 순번(col0)이 숫자인 행. (병합셀 확인된 실제 컬럼 인덱스)
    # 0순번 1업무구분 2TR 3인터페이스명 4전문명세부 5길이 6현/파/채권
    # 7유가 8코스닥 9코넥스 10파생 11파생야간 12일반채권 13소액채권 14KTS 15REPO
    # 16배치온라인 17송수신 18데이터주기 19서비스 23비고 25차세대변경내용
    out = []
    for r in rows:
        r = r + [""] * (29 - len(r))
        seq = r[0]
        if not seq.replace(".", "").isdigit():
            continue
        out.append({
            "순번": seq, "업무구분": r[1], "TR_CODE": r[2],
            "인터페이스명": clean(r[3]), "전문명": clean(r[4]), "길이(헤더제외)": r[5],
            "현파채권": r[6],
            "유가": r[7], "코스닥": r[8], "코넥스": r[9],
            "파생": r[10], "파생야간": r[11],
            "일반채권": r[12], "소액채권": r[13], "KTS": r[14], "REPO": r[15],
            "배치온라인": r[16], "송수신": r[17], "데이터주기": clean(r[18]),
            "서비스": clean(r[19]), "비고": clean(r[23]),
            "차세대변경": clean(r[25]),
        })
    path = os.path.join(OUT, "interface-list.csv")
    with open(path, "w", newline="", encoding="utf-8-sig") as f:
        w = csv.DictWriter(f, fieldnames=list(out[0].keys()))
        w.writeheader()
        w.writerows(out)
    return len(out), path


def extract_fields(wb):
    with wb.get_sheet("회원사인터페이스전문") as sh:
        rows = [[cell(c) for c in r] for r in sh.rows()]
    # 컬럼: 0업무 1TR-CODE 2인터페이스명 3HDR/DATA 4순번 5항목명 6영문명 7타입 8길이 9필수 10정의 11차세대변경 12변경내용 13비고
    out = []
    cur_tr_primary = ""
    cur_tr_all = ""
    cur_iface = ""
    cur_biz = ""
    for r in rows:
        r = r + [""] * (14 - len(r)) if len(r) < 14 else r
        raw1 = r[1] if len(r) > 1 else ""
        col1 = clean(raw1)
        # 그룹 헤더: TR-CODE(col1)에 값이 있으면 새 전문 그룹 (여러 변형 나열 가능)
        if col1:
            trs = [t.strip() for t in raw1.replace("\r", "\n").split("\n") if t.strip()]
            if trs:
                cur_tr_primary = trs[0]
                cur_tr_all = ";".join(trs)
            if r[2]:
                cur_iface = clean(r[2])
            if r[0]:
                cur_biz = r[0]
        else:
            if r[2]:
                cur_iface = clean(r[2])
            if r[0]:
                cur_biz = r[0]
        seq = r[4] if len(r) > 4 else ""
        # 필드 행: 순번(col4) 숫자 + 항목명(col5) 존재
        if not (seq.replace(".", "").isdigit() and len(r) > 5 and r[5]):
            continue
        out.append({
            "업무구분": cur_biz, "TR_CODE": cur_tr_primary, "TR_ALL": cur_tr_all,
            "인터페이스명": cur_iface, "HDR_DATA": r[3],
            "순번": seq, "항목명": r[5], "항목영문명": clean(r[6]),
            "DataType": r[7], "길이": r[8], "필수": r[9],
            "정의": clean(r[10])[:300], "차세대변경": r[11] if len(r) > 11 else "",
        })
    path = os.path.join(OUT, "fields.csv")
    with open(path, "w", newline="", encoding="utf-8-sig") as f:
        w = csv.DictWriter(f, fieldnames=list(out[0].keys()))
        w.writeheader()
        w.writerows(out)
    return len(out), path


# col1 원본이 개행 포함 다중 TR일 수 있어 원본 셀에서 줄단위 분리 필요.
# cell()이 이미 strip만 하므로, 여기선 재분리용 헬퍼(정의상 clean 전 값 사용 불가하여 근사).
def cell_multiline(c):
    if c is None or c.v is None:
        return []
    return str(c.v).replace("\r", "\n").split("\n")


def main():
    xlsb = sys.argv[1] if len(sys.argv) > 1 else None
    if not xlsb or not os.path.exists(xlsb):
        print("usage: extract_exture.py <xlsb path>")
        sys.exit(1)
    wb = pyxlsb.open_workbook(xlsb)
    n1, p1 = extract_interface_list(wb)
    n2, p2 = extract_fields(wb)
    print(f"interface-list.csv: {n1} messages -> {p1}")
    print(f"fields.csv: {n2} field rows -> {p2}")


if __name__ == "__main__":
    main()
