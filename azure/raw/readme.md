# Raw glider data access

## Azure blob storage

Akvaplan-niva stores unprocessed glider data in [Azure blob storage][azure-blob], in the `raw` container under the `gliderdata` account.

## Azure configuration

See [env-config] for how to configure your local machine for accessing `gliderdata`.

Contact [Akvaplan-niva's data engineer](mailto:che) to get a a ccess token that provides you with read-only access rights.

## Data organisation

### Naming convention

The raw container is organised in a flat structure with one folder per vehicle deployment.

The deployment folders have this naming convention: `{year}_campaignarea}_{vehicletype}_{vehicleid}`,
eg.`2021_polarfront_seaglider_sg644`.

### Data organisation

Inside the deployment folders, the files/folders are copied as-is from the data provider, but separate data types should be kept in separate folders.

## Azure tools

A number of tools may be used to download data from Azure blob containers.

Users on all computing platforms (Linux, MacOS, Windows) can use the [azcopy] command line tool.

Linux users might want to use [Blobfuse] and simply mount the blob container as a virtual file system (see [guide][blobfuse-azure]).

[env-config]: ../env-config.md
[azure-blob]: https://azure.microsoft.com/en-us/services/storage/blobs
[azcopy]: https://docs.microsoft.com/en-us/azure/storage/common/storage-use-azcopy-v10
[blobfuse]: https://github.com/Azure/azure-storage-fuse
[blobfuse-azure]: https://docs.microsoft.com/en-us/azure/storage/blobs/storage-how-to-mount-container-linux

## azcopy

```sh
# List all TXT files:
azcopy list $env:GLIDERDATA_RAW/?$env:GLIDERDATA_SAS | Select-String ".TXT"

# Download everything from a deployment
azcopy sync $env:GLIDERDATA_RAW/2021_polarfront_sb_adcp?$env:GLIDERDATA*SAS ./2021_polarfront_sb_adcp/

# Download all TXT files across deployments to current folder (.)
azcopy sync --include-pattern="*.TXT" $env:GLIDERDATA_RAW/2021_polarfront_sb_adcp?$env:GLIDERDATA_SAS .

$ azcopy sync $GLIDERDATA/raw/2020_2021_antarctica_sb_echo_1/?$GLIDERDATA_SAS --include-pattern=*20210330* ./2020_2021_antarctica_sb_echo_1/

```
