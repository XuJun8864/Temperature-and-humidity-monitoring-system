cmd_/home/ubuntu/big_project/m74hc595/Module.symvers := sed 's/ko$$/o/' /home/ubuntu/big_project/m74hc595/modules.order | scripts/mod/modpost -m  -E  -o /home/ubuntu/big_project/m74hc595/Module.symvers -e -i Module.symvers   -T -