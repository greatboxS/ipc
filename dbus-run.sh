#!/bin/bash

# Start dbus-daemon
eval $(dbus-launch --sh-syntax)

# Export environment variables
export DBUS_SESSION_BUS_ADDRESS
export DBUS_SESSION_BUS_PID

# Print the exported variables
echo "DBUS_SESSION_BUS_ADDRESS: $DBUS_SESSION_BUS_ADDRESS"
echo "DBUS_SESSION_BUS_PID: $DBUS_SESSION_BUS_PID"