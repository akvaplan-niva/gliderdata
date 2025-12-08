# Slocum realtime data management

## Goals

- Near-realtime data on [RTD](https://rtd.akvaplan.no/#tag/slocum)
- Auto-archiving of realtime assets (dinkum binary/ascii) in cloud storage
- Data isolation for project, mission, and glider

## Architecture

After surfacing a Slocum sends a compressed `dinkum binary data` file over
iridium link to the SFMC basestation.

Running on the basestation, a `inotify` _watcher_ detects new files, and starts
a realtime _pipeline_ for each file.

The pipeline

- unpacks the compressed binary file to a regular dinkum binary
- convert the dinkum binary into a dinkum ascii file
- sends a HTTP POST with the ascii as payload to
  https://rtd.akvaplan.no/slocum/xba
- stores the compressed binary and the dinkum ascii in a local project folder

## Installation

Watcher/pipeline:

```bash
sudo su - localuser
git clone https://github.com/akvaplan-niva/gliderdata
cd gliderdata && gliderdata=$(pwd)
sudo ln -s $gliderdata/slocum/realtime/*.bash /usr/local/bin
touch $gliderdata/slocum/realtime/.env
```

## Configuration

Global

```bash
# RTD API
export RTD_API_USER="…"
export RTD_API_KEY_READ="…"
export RTD_API_KEY_CREATE="…"

# Global `.cac` cache:
export SLOCUM_CAC_CACHE="…"
```

Project and mission

Project and mission metadata must be updated prior to each launch, in order to
isolate and archive data locally, and in blob storage.

Example for open access project `jellysafe` with planned first launch on
`2025-12-05`:

```bash
# Create project/mission dir:
mkdir -p /srv/akvaplan-open/jellysafe/2025-12-05
```

```bash
# Configure active project/mission
export RTD_PROJECT="jellysafe"
export RTD_PROJECT_ACCESS="open" # open | restricted
#RTD_PROJECT_DIR
#RTD_MISSION_ISODATE="2025-12-05"
export RTD_MISSION_DIR="/srv/akvaplan-open/jellysafe/2025-12-05"
#RTD_MISSION_BLOB
```

# Blob storage

Everything under /srv/akvaplan-{open|restricted}* is archived in Azure blob
storage account `gliderdata`: https://gliderdata.blob.core.windows.net
