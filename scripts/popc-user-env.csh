#!/bin/csh
if ( ! $?POPC_LOCATION ) then
  setenv POPC_LOCATION /usr/local/popc
endif

setenv POPC_PLUGIN_LOCATION $POPC_LOCATION/lib/plugins

if ( ! $?PATH ) then
    setenv PATH ''
endif

setenv PATH $POPC_LOCATION/bin:$PATH
setenv PATH $POPC_LOCATION/sbin:$PATH

if ( ! $?LD_LIBRARY_PATH ) then
    setenv LD_LIBRARY_PATH ''
endif

setenv LD_LIBRARY_PATH $POPC_LOCATION/lib:$POPC_PLUGIN_LOCATION:$LD_LIBRARY_PATH

