#!/bin/bash

#go to script location in order to use relative paths
scriptdir=$(readlink -f -- "$0")
scriptdir=${scriptdir%/*}
cd ${scriptdir}

dir_name="dev"
module="secvault"
data_device="sv_data"
ctl_device="sv_ctl"
mode="666"

bash ./clean.sh

# invoke insmod with all arguments we got
/sbin/insmod ../${module}.ko $* || exit 1

major=231

mknod /${dir_name}/sv_ctl c $major 0

for i in {0..3};
do
    mknod /${dir_name}/${data_device}$i c $major $((i+1))
done

# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /${dir_name}/${data_device}[0-3]
chmod $mode /${dir_name}/${data_device}[0-3]

chgrp $group /${dir_name}/${ctl_device}
chmod $mode /${dir_name}/${ctl_device}