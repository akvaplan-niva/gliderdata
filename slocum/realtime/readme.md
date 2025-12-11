# Slocum realtime data management

## Goals

- Near-realtime data on [RTD](https://rtd.akvaplan.no/#tag/slocum)
- Isolate data for project, mission, glider, and data format
- Archive realtime assets (dinkum binary/ascii) in cloud storage

## Architecture

After surfacing a Slocum sends compressed `dinkum binary data` files over
iridium to the [SFMC] basestation.

Running on the basestation, a _watcher_ detects new files, and starts a realtime
file _pipeline_.

The pipeline

- unpacks the compressed binary file to a regular dinkum binary
- converts the dinkum binary into a dinkum ascii file
- sends a HTTP POST with the ascii as payload to RTD
- stores the compressed binary and the dinkum ascii in a local project folder

## Installation

Watcher, pipeline and ascii-convertor:

```bash
sudo su - localuser
git clone https://github.com/akvaplan-niva/gliderdata
cd gliderdata && gliderdata=$(pwd)
sudo ln -s $gliderdata/slocum/realtime/*.bash /usr/local/bin
```

Add the following as `localuser` (`$ crontab -e`): to reload watcher every 10
minutes:

```sh
*/10 * * * * /bin/bash -c '. /home/localuser/gliderdata/slocum/realtime/.env && /usr/local/bin/slocum_rt_watcher.bash >> /home/localuser/gliderdata/slocum/realtime/.stdout 2>> /home/localuser/gliderdata/slocum/realtime/.stderr'
```

## Configuration

### Global

`/home/localuser/gliderdata/slocum/realtime/.env`:

```bash
export RTD_API_USER="…"
export RTD_API_KEY_READ="…"
export RTD_API_KEY_CREATE="…"

# Global `.cac` cache:
export SLOCUM_CAC_CACHE="…"
```

### Active project and mission

Project and mission metadata must be updated prior to each launch, in order to
isolate and archive data locally, and in blob storage.

Example for open access project `jellysafe` with planned launch on `2025-12-16`:

```bash
export RTD_PROJECT="jellysafe"
export RTD_PROJECT_ACCESS="open" # open | restricted
RTD_MISSION_ISODATE="2025-12-16"
```

# Blob storage

Everything under /srv/akvaplan-{open|restricted}* is archived in Azure blob
storage account `gliderdata`: https://gliderdata.blob.core.windows.net
