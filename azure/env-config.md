# Azure env configuration

Azure configuration should be stored in environmental variables.

## Linux/bash

Add the following to your (eg.) `.bashrc` or to a `.env` file in your local sync root:

```bash
export GLIDERDATA="https://gliderdata.blob.core.windows.net"
export GLIDERDATA_CONTAINER=""
export GLIDERDATA_SAS=""
```

Check your bash env configuration by running:

```bash
  env | grep GLIDERDATA
  azcopy list "$GLIDERDATA/$GLIDERDATA_CONTAINER/?$GLIDERDATA_SAS"
```

## PowerShell

Add to `$PROFILE.CurrentUserAllHosts`:

```pwsh
$env:GLIDERDATA="https://gliderdata.blob.core.windows.net"
$env:GLIDERDATA_CONTAINER=""
$env:GLIDERDATA_SAS=""
```

Check your PS \$env configuration with:

```pwsh
dir Env:/GLIDERDATA*
azcopy list "$env:GLIDERDATA/$env:GLIDERDATA_CONTAINER/?$env:GLIDERDATA_SAS"
```
