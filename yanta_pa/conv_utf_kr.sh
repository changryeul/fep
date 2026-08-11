#!/usr/bin/bash


find ./ -type f \( -name "*.c" -o -name "*.h" \) -print0| while IFS= read -r -d '' f; do
	enc=$(file -bi "$f" | sed -n 's/.*charset=\(.*\)$/\1/p' |tr -d '[:space:]'| tr '[:upper:]' '[:lower:]')
	if [ "$enc" = "utf-8" ]; then
		echo "Converting $f ..."
		iconv -f "$enc" -t euc-kr "$f" -o "$f.tmp" && mv "$f.tmp" "$f"
	else
		echo "skip $f"
	fi
done

echo "인코딩 완료"

