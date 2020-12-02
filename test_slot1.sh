APP_HASH=$(newtmgr -c ble1 image list | awk 'NR == 11 {print $2}')
newtmgr -c ble1 image test $APP_HASH
newtmgr -c ble1 reset