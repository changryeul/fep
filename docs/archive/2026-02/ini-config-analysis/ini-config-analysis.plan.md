# 계획: ini-config-analysis

> PB FEP INI 설정 분석 및 문서화

## 개요

| 항목 | 세부사항 |
|------|--------|
| 기능 | ini-config-analysis |
| 레벨 | Dynamic |
| 단계 | 계획 |
| 날짜 | 2026-02-18 |
| 관련 | config-db-migration (archived) |

## 배경

FEP (Front-End Processor) PB(채권) 데몬은 8개의 상호 연결된 INI 설정 파일로 작동합니다. 이러한 파일은 전체 런타임 토폴로지를 정의합니다: 프로세스, TCP/UDP 네트워킹, 파일 기반 IPC, 공유 메모리 및 시장 데이터 TR 정의. 이러한 관계를 이해하는 것은 다음에 매우 중요합니다:

- 설정 변경 및 문제 해결
- 새로운 프로세스 추가 또는 포트 할당
- 마이그레이션 검증 (config-db-migration 왕복)
- 새로운 개발자/운영자 온보딩

## 범위

### 포함 범위

1. **8개 INI 파일 분석**: daemon.ini, proc.ini, tcp1.ini, tcp2.ini, udpip.ini, file.ini, dshm.ini, sisetr.ini
2. **2개 IP 화이트리스트 파일**: client.ini, pc.ini
3. **파일 간 관계 매핑**: 프로세스-포트-파일-TR 연결
4. **데이터 흐름 시각화**: KRX <-> FEP <-> 내부 시스템
5. **일관성 검증**: 카운트 필드 대 실제 항목

### 제외 범위

- 소스 코드 분석 (config-db-migration에서 다룸)
- PA/PX/PZ 데몬 설정 (PB만 해당)
- 런타임 SHM 구조 (shm_memory.h 심층 분석 필요)
- REAL1/REAL2 환경 차이 (TEST만 해당)

## 요구사항

### FR-01: 프로세스 관계 지도

proc.ini에서 18개 PB 프로세스 분석:
- 프로세스 ID, 유형, 상태, TCP/UDP 포트 할당
- OFN/IFN 파일 연결 (생산자-소비자 쌍)
- KRX 로그인 ID 할당 (6개 계정)
- 운영 시간 범위

### FR-02: 네트워크 토폴로지

모든 TCP/UDP 연결 매핑:
- tcp2.ini: 16개 직접 TCP 포트 (10개 내부 + 6개 KRX)
- tcp1.ini: 2개 마스터 데몬 포트 그룹 (P/B)
- udpip.ini: 2개 UDP 멀티캐스트 채널
- KRX 게이트웨이 IP 분배 (.119/.120)
- 포트 범위 규칙 (3xxxx, 4xxxx, 5xxxx, 6xxxx)

### FR-03: 파일/FIFO IPC 지도

file.ini에서 8개 파일 기반 IPC 채널 문서화:
- 레코드 크기 및 데이터 구조 (헤더 62B + 데이터 헤더 22B + 데이터 + LF)
- 생산자-소비자 프로세스 쌍
- OFN/IFN과 proc.ini의 일관성 검증

### FR-04: 시장 데이터 TR 카탈로그

sisetr.ini에서 29개 시세 TR 카탈로그:
- TR 코드 명명 규칙
- 6개 시장 범주 (큐 코드)
- TR 유형당 길이 정의
- UDP 수신자 프로세스 매핑

### FR-05: 데몬 설정

daemon.ini에서 Daemon_B (PB) 설정 문서화:
- IPC 리소스 할당 (Proc/File/Tcp2/Udpip 개수)
- 운영 일정 (Date_Flag=2, D~D+1)
- 컴팩트 정책 (9일)
- A~Z의 슬롯 사용

### FR-06: 파일 간 일관성 보고

모든 파일 간 참조 검증:
- daemon.ini 개수 대 실제 INI 항목
- proc.ini Tcp2_Primary 포트 대 tcp2.ini 정의
- proc.ini Udp_Port 대 udpip.ini 정의
- proc.ini OFN/IFN 대 file.ini 이름
- 고아/일치하지 않는 항목 식별

## 결과물

| # | 결과물 | 형식 | 위치 |
|---|-------------|--------|----------|
| 1 | PB 설정 분석 문서 | Markdown | docs/02-design/features/ini-config-analysis.design.md |
| 2 | 프로세스 흐름도 (ASCII) | 문서 내 | 포함 |
| 3 | 네트워크 토폴로지도 (ASCII) | 문서 내 | 포함 |
| 4 | 파일 간 일관성 매트릭스 | 표 | 문서 내 |

## 데이터 소스 (분석 완료)

| INI 파일 | 항목 | 주요 발견 |
|----------|---------|-------------|
| daemon.ini | 26개 슬롯 (A~Z) | Daemon_B 활성, Date_Flag=2 |
| proc.ini | 18개 프로세스 | 6개 KRX 로그인, 8개 파일 쌍 |
| tcp2.ini | 16개 포트 | 10개 내부(0.0.0.0) + 6개 KRX(.119/.120) |
| tcp1.ini | 2개 마스터 그룹 | PB 미사용 (모두 TCP2) |
| udpip.ini | 2개 채널 | 233.38.231.91/153 멀티캐스트 |
| file.ini | 8개 파일 | 크기 200~2048B, 모두 Fifo=1 |
| dshm.ini | 0 | PB는 파일 IPC 사용, 데이터 SHM 아님 |
| sisetr.ini | 29개 TR | 6개 시장, 50~1300B |

## 일관성 확인 결과 (사전 검증)

| 확인 | daemon.ini 할당 | 실제 | 상태 |
|-------|-----------------|--------|--------|
| Proc_Count | 20 | 18 | OK (여유 2) |
| File_Count | 8 | 8 | OK (전체) |
| Dshm_Count | 0 | 0 | OK |
| Tcp2_Count | 20 | 16 | OK (여유 4) |
| Udpip_Count | 5 | 2 | OK (여유 3) |
| proc Tcp2 포트 대 tcp2.ini | - | 14/16 일치 | 42001,42002 프로세스 없음 |
| proc Udp 포트 대 udpip.ini | - | 2/2 일치 | OK |
| proc OFN/IFN 대 file.ini | - | 8/8 일치 | OK |

## 위험 및 메모

| 위험 | 영향 | 완화 |
|------|--------|------------|
| tcp2 포트 42001/42002 프로세스 매핑 없음 | 낮음 | UDP 수신자가 TCP 배포용으로 사용할 가능성 |
| tcp1.ini는 PB에서 미사용 | 정보 | PA 모듈이 사용할 수 있음; 참조로 문서화 |
| File_Count가 용량에 도달 (8/8) | 중간 | 새로운 파일 IPC는 daemon.ini 업데이트 필요 |
| LogManager 제외한 모든 프로세스 Status=S | 정보 | TEST 환경에서는 정상 |

## 성공 기준

- [ ] 모든 8+2 INI 파일 완전히 문서화됨
- [ ] 프로세스 흐름도가 모든 6개 데이터 흐름 다룸
- [ ] 네트워크 토폴로지가 모든 포트 할당을 표시
- [ ] 파일 간 일관성 100% 검증됨
- [ ] 문서가 운영 참조로 사용 가능함
