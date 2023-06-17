photos of run https://imgur.com/a/2vfFBRa

Overview:
in this assigment we implemented chardevice chardev from sysprog21 manual supporting IOCTL , sequencing , creating sys files reading variables, using mutex
each time we read or write to the device open operation is occuring then the reading and then the release of the chardevice.
with IOCTL we can change the key in the running time of the module using a executeable we give parameters that safely changing the key.
with sys file we can keep track how many bytes_registered to the buffer the maximum will be the buffer size minus 1 (as we assumed this was the meaning.. since you can keep track with bytes_registered+newbytes).
with sequence we use seq_file iterating each byte from start->next->next->...->end->start

usage:
make 
insmod chardev.ko key=(int value)
gcc -o mioctl mioctl.c (to compile ioctl main for changing key)
cat /dev/chardev (to read)
echo "string" > /dev/chardev (to write)
./mioctl (int value) for changing key
cat /sys/kernel/chardev/bytes_registered (how many bytes registered maximum as BUFFER-1) 0 to BUFFER-1
cat /proc/iter (sequence api)
dmesg (print log)




log from dmesg:

[ 3222.809068] (chardev init)
[ 3222.809071] registered major 509 number
[ 3222.809132] key = 5
[ 3222.809133] device created on /dev/chardev 
[ 3247.860033] mutex is locked
[ 3247.868313] device_write
[ 3247.868316] (m`iijrjwia%qmlv%lv%q`vq%75?6<\x0f)
[ 3247.869065] Tmutex unlock!
[ 3257.760128] mutex is locked
[ 3257.768651] device_write
[ 3257.768653] (av\x0fijrjwia%qmlv%lv%q`vq%75?6<\x0fg|`rjwia%qmlv%lv%q`vq%75?6<.v`fjk)
[ 3257.769740] Tmutex unlock!
[ 3266.612895] mutex is locked
[ 3266.612914] new key 77
[ 3266.612917] old key encryption (av\x0fijrjwia%qmlv%lv%q`vq%75?6<\x0fg|`rjwia%qmlv%lv%q`vq%75?6<.v`fjk)
[ 3266.612920] decrypted (ds
               loworld this is test 20:39
               byeworld this is test 20:39+secon)
[ 3266.612922] newkey encryption ()>G!":"?!)m9%$>m$>m9(>9m\x7f}w~tG/4(:"?!)m9%$>m$>m9(>9m\x7f}w~tf>(."#)
[ 3266.612991] Tmutex unlock!
[ 3279.246042] mutex is locked
[ 3279.246051] )>G!":"?!)m9%$>m$>m9(>9m\x7f}w~tG/4(:"?!)m9%$>m$>m9(>9m\x7f}w~tf>(."#
[ 3279.246052] device_read
[ 3279.246057] )>G!":"?!)m9%$>m$>m9(>9m\x7f}w~tG/4(:"?!)m9%$>m$>m9(>9m\x7f}w~tf>(."#
[ 3279.246057] device_read
[ 3279.246061] Tmutex unlock!
[ 3295.452774] mutex is locked
[ 3295.452786] new key 5
[ 3295.452789] old key encryption ()>G!":"?!)m9%$>m$>m9(>9m\x7f}w~tG/4(:"?!)m9%$>m$>m9(>9m\x7f}w~tf>(."#)
[ 3295.452792] decrypted (ds
               loworld this is test 20:39
               byeworld this is test 20:39+secon)
[ 3295.452794] newkey encryption (av\x0fijrjwia%qmlv%lv%q`vq%75?6<\x0fg|`rjwia%qmlv%lv%q`vq%75?6<.v`fjk)
[ 3295.452862] Tmutex unlock!
[ 3297.880140] mutex is locked
[ 3297.880149] av\x0fijrjwia%qmlv%lv%q`vq%75?6<\x0fg|`rjwia%qmlv%lv%q`vq%75?6<.v`fjk
[ 3297.880150] device_read
[ 3297.880156] av\x0fijrjwia%qmlv%lv%q`vq%75?6<\x0fg|`rjwia%qmlv%lv%q`vq%75?6<.v`fjk
[ 3297.880156] device_read
[ 3297.880160] Tmutex unlock!

