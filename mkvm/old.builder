#
vc=usodpwpvc001.uson.usoncology.int
#vc=usogpwpvc001.uson.usoncology.int
tools=/usr/local/lib/tools
tmp=/tmp/mkvm.dat

#hn=usodtlvwb002.ulab.usoncology.unx
hn=usodplvrx002.uson.usoncology.int
sn=`echo $hn | awk -F. '{ print $1 }'`
#os=RedHat; ver=6; arch=x86_64; id=rhel6_64Guest; nic=vmxnet3
os=RedHat; ver=5; arch=x86_64; rhel5_64Guest; nic=e1000
way=ddc-coe.uson.usoncology.int
profile=default
ip=10.5.203.24

doit() {
        n="$1"
        c="$2"
        g="$3"
        h="$4"
        a="$5"
        n=`echo $fn | awk -F. '{ print $1 }'`
        m=`echo "$g 1024 * p" | dc`
        t=`echo $h | sed "s:GB$::"`
        s=`echo "$t 1024 * p" | dc`
        cat tmpl.xml | sed -e "s|+NAME+|$n|" -e "s:+VCPU+:$c:" -e "s:+MEM+:$m:" -e "s:+SIZE+:$s:" \
		-e "s:+NIC+:$e:" -e "s:+ID+:$i:" -e "s:+ANN+:$a:" > $tmp
        ./create_vms.pl --server $vc --username 'USON\svcvsphere' --password 'svcvsphere*!' --file $tmp
#       echo "$a -> $fn"
}
doit usodplvrx002 4 8 16 e1000 rhel5_64Guest; "RX3000"

exit 0
	t=`$tools/getiso $hn $os $ver $arch $way $profile $ip $vlan`
	if test `echo $t | grep -c ERR` -gt 0; then
		echo $t
		exit 1
	fi
	url=`echo $t | grep ^URL= | awk -F= '{ print $2 }'`
	#echo "URL: $url"
	iso=`basename $url`
       fn=`curl -S -s -n "http://tools/namer/namer.php?action=get&site=usod&env=t&os=l&type=v&app=$app&reserve=yes"`
       if test `echo $fn | grep -i -c error` -gt 0; then
               echo $n
               exit 1
       fi
#farminfo -x Misc12 > /tmp/do_remcd.dat
#worker /tmp/do_remcd.dat 16 ./do_remcd
cat test.xml | sed -e "s:+++NAME+++:$sn:" -e "s|+++ISO+++|$iso|" > $tmp
cat test.xml | sed -e "s:+++NAME+++:$sn:" -e "s|+++ISO+++|$iso|" > $tmp
./create_vms.pl --server $vc --username 'USON\svcvsphere' --password 'svcvsphere*!' --file $tmp
