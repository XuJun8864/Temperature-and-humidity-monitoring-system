cmd_/home/ubuntu/big_project/test/modules.order := {   echo /home/ubuntu/big_project/test/test.ko; :; } | awk '!x[$$0]++' - > /home/ubuntu/big_project/test/modules.order
