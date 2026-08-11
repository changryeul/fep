# Report Generator Agent Memory

## PDCA Report Pattern: Active/Standby HA Features

When generating completion reports for HA (High Availability) features, follow this pattern:

### Key Sections
1. **Overview**: Problem statement (why HA needed), architecture diagram, role definitions
2. **Design vs Implementation Validation**: FR-by-FR checklist showing code line references
3. **Gap Analysis**: For deferred FRs, explain root cause, impact assessment, reproduction scenario, permanent fix options
4. **Code Implementation Summary**: Global variables, constants, functions added/modified with line ranges
5. **Deployment Checklist**: Environment variables, firewall config, operational procedures
6. **Known Issues**: Use FR-XX labeling for requirement traceability

### Common HA Patterns in FEP
- **Environment variable role determination**: `_FEP_DIV` for REAL1/REAL2/TEST routing
- **UDP heartbeat for simplicity**: Lightweight monitoring, no TCP reliability overhead
- **select() socket integration**: Monitor heartbeat socket alongside primary data socket with 1-sec timeout
- **Non-blocking drain loop**: Process all buffered heartbeat packets before timeout check (prevents starvation)
- **Graceful degradation**: HA_Init failure → STANDALONE mode (feature continues without HA)
- **per-loop state machine**: Failover/failback checks in main loop (not callback-based)

### Quality Gate
- Aim for 90%+ match rate on initial implementation (11/12 = 91% is acceptable)
- Defer low-impact oscillations/edge cases (FR-08 type issues) to maintenance phase
- Always provide code references (line numbers) for each FR validation
- Include "Permanent Fix" options for deferred gaps

### Report Structure Template
```
1. Overview (feature description, problem, architecture)
2. PDCA Cycle Summary (Plan/Design/Do/Check/Act status)
3. Design vs Implementation Validation (FR table with code refs)
4. Code Implementation Summary (globals, constants, functions with line ranges)
5. Quality Metrics (coverage, robustness, edge cases)
6. Deployment Checklist (env vars, firewall, procedures)
7. Lessons Learned (what went well, improvements, future patterns)
8. Testing Recommendations (unit tests, integration scenarios)
9. Known Issues and Deferrals (FR-XX labeling, impact, workarounds, permanent fix)
10. Sign-Off (deliverables, match rate justification, overall assessment)
```

## PB-HA Specific Findings

### Implementation Quality: 91% Match Rate
- 11/12 FRs PASS on initial implementation (exceptional for HA)
- FR-08 deferred: Device_Close() sleep(3) causes failback oscillation (minor, low-frequency)
- Root cause: sleep was designed for TCP retry, blocks heartbeat during state transition
- Acceptable defer: affects rare scenario, no data loss, self-resolves after Primary stability

### Key Code Patterns to Reference
- **Recv_Data() modification**: Lines 558-644 (select timeout 1s, non-blocking hb_sockfd drain)
- **PB_7100_UR() main loop**: Lines 275-399 (per-loop heartbeat + ha_active gating)
- **HA_Init()**: Lines 795-920 (role determination via _FEP_DIV, socket setup)
- **HA_Check_Failover()**: Lines 966-998 (elapsed time comparison for failover/failback)

### Deployment Keys
- Ensure _FEP_DIV, _HA_PEER_IP, _HA_HB_PORT configured in pkg_env.sh
- Firewall: open UDP port 50000 bi-directional between REAL1 ↔ REAL2
- Test failover: kill Primary → verify Secondary activates after 3 sec
- Accept occasional oscillation during failback (FR-08 known issue)

## Changelog Integration
Always add completed features to `docs/04-report/changelog.md` with:
- Date of completion
- Feature summary and scope
- Deliverables (files, lines of code, metrics)
- Architecture highlights
- Known issues and next steps
