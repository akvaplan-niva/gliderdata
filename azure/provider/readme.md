# Upload glider data

Generic documentation for data providers

## Azure blob storage

Akvaplan-niva will store incoming raw data from your company in a separate container in [Azure blob storage][azure-blob].

### Naming convention

The container must contain one folder per vehicle deployment, with the following naming convention: `{year}_campaignarea}_{vehicletype}_{vehicleid}`, eg.`2021_polarfront_seaglider_sg644`.

### Data organisation

Always put each data type in separate folders inside the deployment folder.

### Tools

A number of tools may be used to upload data to your Azure container.

Users on all computing platforms (Linux, MacOS, Windows) can use the [azcopy] command line tool.

Linux users might want to use [Blobfuse] and simply mount the blob container as a virtual file system (see [guide][blobfuse-azure]).

[azure-blob]: https://azure.microsoft.com/en-us/services/storage/blobs
[azcopy]: https://docs.microsoft.com/en-us/azure/storage/common/storage-use-azcopy-v10
[blobfuse]: https://github.com/Azure/azure-storage-fuse
[blobfuse-azure]: https://docs.microsoft.com/en-us/azure/storage/blobs/storage-how-to-mount-container-linux

### Upload script using azcopy sync

A generic upload script for providers running linux is provided here: [gliderdata_sync_up](https://github.com/akvaplan-niva/gliderdata/blob/main/azure/provider/gliderdata_sync_up).

Copy the script to your local root folder and edit `.env` as explained below.

## Configuration

Azure configuration information should be stored in environmental variables.

### Linux/bash

Add the following to your (eg.) `.bashrc` or to a `.env` file in your local sync root:

```pwsh
export GLIDERDATA="https://gliderdata.blob.core.windows.net"
export GLIDERDATA_CONTAINER=""
export GLIDERDATA_SAS=""
```

### PowerShell

Add to `$PROFILE.CurrentUserAllHosts`:

```pwsh
$env:GLIDERDATA="https://gliderdata.blob.core.windows.net"
$env:GLIDERDATA_CONTAINER=""
$env:GLIDERDATA_SAS=""
```
