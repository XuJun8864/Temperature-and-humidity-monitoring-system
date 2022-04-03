cmd_/home/ubuntu/big_project/led/modules.order := {   echo /home/ubuntu/big_project/led/leds.ko; :; } | awk '!x[$$0]++' - > /home/ubuntu/big_project/led/modules.order
