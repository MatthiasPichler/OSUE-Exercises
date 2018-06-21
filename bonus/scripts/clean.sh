#!/bin/bash

dir_name="dev"
module="secvault"
data_device="sv_data"
ctl_device="sv_ctl"

/sbin/rmmod ${module} || true

rm -f /${dir_name}/${data_device}[0-3]
rm -f /${dir_name}/${ctl_device}