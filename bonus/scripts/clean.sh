#/bin/sh

dir_name="dev"
module="secvault"
data_device="sv_data"
ctl_device="sv_ctl"

# invoke insmod with all arguments we got
/sbin/rmmod ./$module.ko || true

rm -f /${dir_name}/${data_device}[0-3]
rm -f /${dir_name}/${ctl_device}