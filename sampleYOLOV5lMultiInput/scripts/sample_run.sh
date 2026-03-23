#!/bin/bash
ScriptPath="$( cd "$(dirname "$BASH_SOURCE")" ; pwd -P )"

function main()
{
    echo "[INFO] The sample starts to run"
    cd ${ScriptPath}/../out
    ./main ../scripts/test_device0.json > device0.log
    ./main ../scripts/test_device1.json > device0.log
    ./main ../scripts/test_device2.json > device0.log
    ./main ../scripts/test_device3.json > device0.log
}
main