for lib in /fsfxwin/fep/st01/lib/libfepP.a /fsfxwin/fep/etc/libletc.a /fsfxwin/fep/alarm/lib/libalarm.a; do
	echo "=== $lib ==="
	for obj in $(ar t $lib); do
		ar x $lib $obj
		if readelf -h $obj | grep -q DYNl; then
			echo "   PIE OBJECT: $lib($obj)"
		fi
		rm -f $obj
	done
done
