#!/bin/bash
# Export to hardware...

echo "Flashing..."
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../../sdk/CH32LibSDK}"
../../../tools/CH32LibSDK/_e1.sh
bash r.sh