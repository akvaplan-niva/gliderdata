# Upload glider data

## Generic provider documentation

### Azure blob storage

Akvaplan-niva will store incoming raw data from your company in a separate container in [Azure blob storage][azure-blob]

### Tools

A number of tools may be used to upload data to your Azure container.

Users on all computing platforms (Linux, MacOS, Windows) can use the [azcopy] command line tool.

Linux users might want to use [Blobfuse] and simply mount the [blob container][blobfuse-azure] as a (virtual) file system.

[azure-blob]: https://azure.microsoft.com/en-us/services/storage/blobs
[azcopy]: https://docs.microsoft.com/en-us/azure/storage/common/storage-use-azcopy-v10
[blobfuse]: https://github.com/Azure/azure-storage-fuse
[blobfuse-azure]: https://docs.microsoft.com/en-us/azure/storage/blobs/storage-how-to-mount-container-linux
