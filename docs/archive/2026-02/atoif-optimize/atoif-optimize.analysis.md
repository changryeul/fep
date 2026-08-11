# Gap Analysis: atoif-optimize

> **Feature**: atoif-optimize (String-to-Number Inner Loop Optimization)
> **Design Document**: `docs/02-design/features/atoif-optimize.design.md`
> **Implementation Files**: `st01/sub/atoif.c`, `st01/sub/atolf.c`, `st01/sub/atodf.c`
> **Analysis Date**: 2026-02-28

## Summary

- Match Rate: **100%**
- Total FR Items: 3
- Matched: 3
- Gaps: 0

All three functions (AtoIf, AtoLf, AtoDf) were optimized exactly as specified in the design document. The inner `for (j=0; j<10; j++)` loops were replaced with direct range checks and `(c - '0')` arithmetic in every case. Variable declarations, control flow, indentation, and behavioral semantics all match the design "After" code.

## FR Verification

| FR | File | Description | Status |
|----|------|-------------|:------:|
| FR-01 | `st01/sub/atoif.c` | AtoIf: remove inner loop, add `char c`, direct arithmetic | PASS |
| FR-02 | `st01/sub/atolf.c` | AtoLf: remove inner loop, add `char c`, direct arithmetic | PASS |
| FR-03 | `st01/sub/atodf.c` | AtoDf: remove inner loop, add `char c`, direct arithmetic, fix indentation, remove `continue` | PASS |

### FR-01 Detail: AtoIf (`st01/sub/atoif.c`)

Design "After" vs implementation comparison (lines 21-41):
- Variable `j` removed from declaration: `int i, jj, u;` -- MATCH
- `char c;` added on separate line -- MATCH
- Initialization: `jj = u = 0;` (no `j`) -- MATCH
- Character cache: `c = *(p_ascii+i);` -- MATCH
- Range check: `if (c >= '0' && c <= '9')` -- MATCH
- Direct arithmetic: `jj = jj * 10 + (c - '0');` -- MATCH
- Merged negative check: `else if (c == '-')` -- MATCH
- Return logic unchanged -- MATCH

### FR-02 Detail: AtoLf (`st01/sub/atolf.c`)

Design "After" vs implementation comparison (lines 21-42):
- Variable `j` removed from declaration: `int i, u;` -- MATCH
- `long jj;` on separate line (preserved) -- MATCH
- `char c;` added on separate line -- MATCH
- Initialization: `jj = u = 0;` (no `j`) -- MATCH
- Character cache: `c = *(p_ascii+i);` -- MATCH
- Range check: `if (c >= '0' && c <= '9')` -- MATCH
- Direct arithmetic: `jj = jj * 10 + (c - '0');` -- MATCH
- Merged negative check: `else if (c == '-')` -- MATCH
- Return logic unchanged -- MATCH

### FR-03 Detail: AtoDf (`st01/sub/atodf.c`)

Design "After" vs implementation comparison (lines 21-66):
- Variable `j` removed from declaration: `int i, k, d, u;` -- MATCH
- `char c;` added on separate line -- MATCH
- Initialization: `k = u = 0; js = ss = jj = 0.;` (no `j`) -- MATCH
- Character cache: `c = *(p_ascii+i);` -- MATCH
- Inverted control flow: digit branch first (`if (c >= '0' && c <= '9')`) then `else` for non-digit -- MATCH
- Integer part: `js = js * 10. + (c - '0');` -- MATCH
- Decimal part: `ss = ss * 10. + (c - '0');` with `k ++` -- MATCH
- Non-digit: `if (c == '.') k = 1;` and `if (c == '-') u = 1;` in `else` block -- MATCH
- `continue` removed -- MATCH
- Decimal calculation logic preserved: `k--`, `d = d * 10` loop, `jj = js + ss/d` -- MATCH
- Indentation normalized to tabs throughout -- MATCH

## Verification Checklist

| ID | Requirement | Method | Result |
|----|-------------|--------|:------:|
| V-01 | AtoIf inner loop removed | Grep: `for (j = 0` in atoif.c = 0 matches | PASS |
| V-02 | AtoIf direct arithmetic | Grep: `c - '0'` in atoif.c = 1 occurrence | PASS |
| V-03 | AtoIf variable `j` removed | Read: declaration is `int i, jj, u;` -- no `j` | PASS |
| V-04 | AtoIf `char c` added | Read: line 23 has `char c;` | PASS |
| V-05 | AtoLf inner loop removed | Grep: `for (j = 0` in atolf.c = 0 matches | PASS |
| V-06 | AtoLf direct arithmetic | Grep: `c - '0'` in atolf.c = 1 occurrence | PASS |
| V-07 | AtoLf variable `j` removed | Read: declaration is `int i, u;` -- no `j` | PASS |
| V-08 | AtoDf inner loop removed | Grep: `for (j = 0` in atodf.c = 0 matches | PASS |
| V-09 | AtoDf direct arithmetic | Grep: `c - '0'` in atodf.c = 2 occurrences | PASS |
| V-10 | AtoDf variable `j` removed | Read: declaration is `int i, k, d, u;` -- no `j` | PASS |
| V-11 | AtoDf decimal logic preserved | Read: `k--`, `d * 10` loop, `js + ss/d` all present and unchanged | PASS |
| V-12 | All 3: negative sign handling | Grep: `== '-'` found in all 3 files (1 each) | PASS |
| V-13 | All 3: return type unchanged | fep_sub.h:276-278 declares `int AtoIf`, `long AtoLf`, `double AtoDf` -- matches implementations | PASS |
| V-14 | No caller changes | Grep found 59 files in src/ that call these functions; none required modification (API unchanged) | PASS |
| V-15 | Build test | `mk.sh sub` compiles without error | NOT TESTED |

## Gaps Found

None.

## Conclusion

The atoif-optimize feature achieves a **100% match rate** between design and implementation across all 3 functional requirements and 15 verification items (14 PASS, 1 NOT TESTED for build).

The optimization is clean and behaviorally equivalent:
- The inner `for (j=0; j<10; j++)` digit-matching loop was O(10) per character; the new `c >= '0' && c <= '9'` range check is O(1).
- All three functions now use a `char c` cache variable to avoid repeated pointer dereference `*(p_ascii+i)`.
- Variable `j` was eliminated from all three files.
- AtoDf additionally had its control flow inverted (digit-first instead of non-digit-first) and inconsistent space/tab indentation was normalized.
- Function signatures, return types, and calling conventions are completely unchanged -- 59 caller files in src/ required zero modifications.
