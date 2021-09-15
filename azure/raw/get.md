# Raw glider data access

## Azure

The raw glider data is archived in Azure blob container https://gliderdata.blob.core.windows.net/raw

There is one folder per deployment, prefixed by year, like:

- 2021_polarfront_sb_adcp
- 2021_polarfront_sb_iskant

Data may be accessed using the azcopy command line tool.

## PowerShell

### Env variables

Put the following in your `$PROFILE.CurrentUserAllHosts`:

```pwsh
$env:GLIDERDATA_RAW="https://gliderdata.blob.core.windows.net/raw"
$env:GLIDERDATA_SAS="" #
```

## azcopy

```pwsh
# List all TXT files:
azcopy list $env:GLIDERDATA_RAW/?$env:GLIDERDATA_SAS | Select-String ".TXT"

# Download everything from a deployment
azcopy sync $env:GLIDERDATA_RAW/2021_polarfront_sb_adcp?$env:GLIDERDATA*SAS ./2021_polarfront_sb_adcp/

# Download all TXT files across deployments to current folder (.)
azcopy sync --include-pattern="*.TXT" $env:GLIDERDATA_RAW/2021_polarfront_sb_adcp?$env:GLIDERDATA_SAS .
```
