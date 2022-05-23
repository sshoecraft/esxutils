for d in yes no
do
	make DEBUG=$d clean
	make DEBUG=$d 
	make DEBUG=$d install
done
