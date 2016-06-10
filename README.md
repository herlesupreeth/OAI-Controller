# OpenAirInterface Controller implementation using C language

Rough implementation of controller to obtain mac stats from Openairinterface ENB agent

#########################################################################################
PREREQUISITES

1. Compatible OAI ENB code with ENB-AGENT implementation
2. Google Protocol Buffer (C++)
3. Protobuf-c

#########################################################################################

INSTRUCTIONS:

1. cd into controller folder and run "source oaienv" command
2. cd cmake_targets
3. ./build_controller
4. Please refer to log generated in controller_dir/cmake_targets/log folder in case of a build fail
5. cd ../targets/bin
6. run the command "./controller"

