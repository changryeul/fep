# Design: atoif-optimize

## References

- Plan: `docs/01-plan/features/atoif-optimize.plan.md`
- Source: FEP_Architecture_Analysis.md §4.7

## Implementation Order

1. `sub/atoif.c` — AtoIf (FR-01)
2. `sub/atolf.c` — AtoLf (FR-02)
3. `sub/atodf.c` — AtoDf (FR-03)

## Detailed Changes

### Step 1: AtoIf — sub/atoif.c (FR-01)

**Before** (lines 21-44):
```c
{
	int		i, j, jj, u;

	j = jj = u = 0;

	for (i = 0; i < p_len; i ++)
	{
		for (j = 0; j < 10; j ++)
		{
			if (*(p_ascii+i) == ('0' + j))
				break;
		}

		if (j < 10)
			jj = jj * 10 + j;

		if (*(p_ascii+i) == '-')
			u = 1;
	}

	if (u)
		jj = jj * -1;

	return (jj);
}
```

**After**:
```c
{
	int		i, jj, u;
	char	c;

	jj = u = 0;

	for (i = 0; i < p_len; i ++)
	{
		c = *(p_ascii+i);

		if (c >= '0' && c <= '9')
			jj = jj * 10 + (c - '0');
		else if (c == '-')
			u = 1;
	}

	if (u)
		jj = jj * -1;

	return (jj);
}
```

**Changes**:
- Remove variable `j` (inner loop counter no longer needed)
- Add `char c` for single character cache (avoids repeated `*(p_ascii+i)`)
- Replace inner `for (j=0; j<10; j++)` loop with direct range check `c >= '0' && c <= '9'`
- Replace `jj * 10 + j` with `jj * 10 + (c - '0')` — mathematically identical
- Merge `-` check into `else if` (was separate `if` — equivalent because `-` is not a digit)

**Behavioral equivalence proof**:
- Old: loops j from 0..9, breaks when `c == '0'+j`, so `j = c - '0'`. If no match, j==10 (skip).
- New: `c - '0'` gives same j value directly. Range check `c >= '0' && c <= '9'` is equivalent to `j < 10`.

### Step 2: AtoLf — sub/atolf.c (FR-02)

**Before** (lines 21-45):
```c
{
	int		i, j, u;
	long	jj;

	j = jj = u = 0;

	for (i = 0; i < p_len; i ++)
	{
		for (j = 0; j < 10; j ++)
		{
			if (*(p_ascii+i) == ('0' + j))
				break;
		}

		if (j < 10)
			jj = jj * 10 + j;

		if (*(p_ascii+i) == '-')
			u = 1;
	}

	if (u)
		jj = jj * -1;

	return (jj);
}
```

**After**:
```c
{
	int		i, u;
	long	jj;
	char	c;

	jj = u = 0;

	for (i = 0; i < p_len; i ++)
	{
		c = *(p_ascii+i);

		if (c >= '0' && c <= '9')
			jj = jj * 10 + (c - '0');
		else if (c == '-')
			u = 1;
	}

	if (u)
		jj = jj * -1;

	return (jj);
}
```

**Changes**: Identical to AtoIf pattern. Remove `j`, add `char c`, direct arithmetic.

### Step 3: AtoDf — sub/atodf.c (FR-03)

**Before** (lines 21-70):
```c
{
	int		i, j, k, d, u;
	double	jj, js, ss;

    k = u = 0;
	js = ss = jj = 0.;

    for (i = 0; i < p_len; i ++)
	{
        for (j = 0; j < 10; j ++)
		{
			if (*(p_ascii+i) == ('0' + j))
				break;
		}

		if (j >= 10)
		{
			if (*(p_ascii+i) == ('.'))
				k = 1;

			if (*(p_ascii+i) == '-')
				u = 1;

			continue;
		}
		else
		{
			if (k == 0)
				js = js * 10. + j;
			else
			{
				ss = ss * 10. + j;
				k ++;
			}
		}
    }

	if (k > 0)
		k --;

	d = 1;
	for (i = 0; i < k; i++)
		d = d * 10;

	jj = js + ss/d;

	if (u)
		jj = jj * -1;

    return (jj);
}
```

**After**:
```c
{
	int		i, k, d, u;
	double	jj, js, ss;
	char	c;

	k = u = 0;
	js = ss = jj = 0.;

	for (i = 0; i < p_len; i ++)
	{
		c = *(p_ascii+i);

		if (c >= '0' && c <= '9')
		{
			if (k == 0)
				js = js * 10. + (c - '0');
			else
			{
				ss = ss * 10. + (c - '0');
				k ++;
			}
		}
		else
		{
			if (c == '.')
				k = 1;

			if (c == '-')
				u = 1;
		}
	}

	if (k > 0)
		k --;

	d = 1;
	for (i = 0; i < k; i++)
		d = d * 10;

	jj = js + ss/d;

	if (u)
		jj = jj * -1;

	return (jj);
}
```

**Changes**:
- Remove variable `j`
- Add `char c` for character cache
- Replace inner `for (j=0; j<10; j++)` loop with range check
- Replace `j` in accumulation with `(c - '0')` — mathematically identical
- Invert control flow: old was `if (j >= 10) { non-digit; continue; } else { digit; }` → new is `if (digit) { ... } else { non-digit; }`
- Fix inconsistent indentation: old file mixed spaces and tabs (lines 25, 28, 30, 56, 70 used spaces). Normalize to tab indentation.
- Remove `continue` (no longer needed — `else` branch handles non-digit path naturally)

**Behavioral equivalence**: `j` was always `c - '0'` when `j < 10`. The `(c - '0')` expression in `js * 10. + (c - '0')` produces a double via implicit int-to-double promotion, same as old `js * 10. + j`.

## Verification Checklist

| ID | Requirement | Verification Method |
|----|-------------|-------------------|
| V-01 | AtoIf inner loop removed | Grep: no `for (j = 0` in atoif.c |
| V-02 | AtoIf direct arithmetic | Grep: `c - '0'` in atoif.c |
| V-03 | AtoIf variable `j` removed | Read: no `j` in variable declaration |
| V-04 | AtoIf char c added | Read: `char c;` in declaration |
| V-05 | AtoLf inner loop removed | Grep: no `for (j = 0` in atolf.c |
| V-06 | AtoLf direct arithmetic | Grep: `c - '0'` in atolf.c |
| V-07 | AtoLf variable `j` removed | Read: no `j` in variable declaration |
| V-08 | AtoDf inner loop removed | Grep: no `for (j = 0` in atodf.c |
| V-09 | AtoDf direct arithmetic | Grep: `c - '0'` in atodf.c (2 occurrences) |
| V-10 | AtoDf variable `j` removed | Read: no `j` in variable declaration |
| V-11 | AtoDf decimal logic preserved | Read: `k` increment and `ss/d` calculation unchanged |
| V-12 | All 3 functions: negative sign handling | Grep: `== '-'` in all 3 files |
| V-13 | All 3 functions: return type unchanged | Grep: function signatures match extern in fep_sub.h |
| V-14 | No caller changes | Grep: no changes in src/ files |
| V-15 | Build test | `mk.sh sub` compiles without error (NOT TESTED — macOS cross-compile) |
