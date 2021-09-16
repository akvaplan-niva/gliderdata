# Uploading glider data

Generic documentation for data providers

## Azure blob storage

Akvaplan-niva will store incoming raw data from your company in a separate container in [Azure blob storage][azure-blob].

## Data organisation

### Naming convention

The container must contain one folder per vehicle deployment, with the following naming convention: `{year}_campaignarea}_{vehicletype}_{vehicleid}`, eg.`2021_polarfront_seaglider_sg644`.

### Data organisation

Always put each data type in separate folders inside the deployment folder.

## Azure tools

A number of tools may be used to upload data to your Azure blob container.

Users on all computing platforms (Linux, MacOS, Windows) can use the [azcopy] command line tool.

Linux users might want to use [Blobfuse] and simply mount the blob container as a virtual file system (see [guide][blobfuse-azure]).

[azure-blob]: https://azure.microsoft.com/en-us/services/storage/blobs
[azcopy]: https://docs.microsoft.com/en-us/azure/storage/common/storage-use-azcopy-v10
[blobfuse]: https://github.com/Azure/azure-storage-fuse
[blobfuse-azure]: https://docs.microsoft.com/en-us/azure/storage/blobs/storage-how-to-mount-container-linux

## azcopy

### gliderdata sync up

[gliderdata_sync_up](https://github.com/akvaplan-niva/gliderdata/blob/main/azure/provider/gliderdata_sync_up) is a generic gliderdata upload script for linux.

The script wraps `azcopy sync` and uploads all directories under a local root directory.
Before uploading, SHA256 checksums are calculated for all files and stored in `shasum.txt` inside each sub-directory.

Copy the script to your local root folder and configure by editing `.env` as explained below.

Uploading your local changes by running:

```sh
./gliderdata_sync_up
```

## Configuration

Azure configuration information should be stored in environmental variables.

### Linux/bash

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

### PowerShell

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
