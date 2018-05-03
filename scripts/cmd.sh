for nomecycle in `ls -1c *CYCLE*-__`;
do
	echo "Elaboro il file " $nomecycle;
# 	/home/meteo/bin/xrit2raw -v -f $nomecycle $nomecycle".raw";
	/home/meteo/bin/navig 20 70 -20 40 0.02 $nomecycle".raw" $nomecycle;
done

