# Weather station project handoff

This is a working-context summary for continuing in Codex on another machine,
not an export of the original conversation. Last updated: 2026-10-30.

Start with [HOME_SETUP.md](HOME_SETUP.md) for the home-computer setup,
new MariaDB server SQL, and the exact point at which to resume.
The user wants to do the implementation themselves with step-by-step guidance.

## Goal

Read a weather station's sensors, collect its serial output in Python, store
readings initially in CSV and then MariaDB, and display readings on a Django
website. Collaborate through GitHub.

## Repository and existing implementation

- GitHub: https://github.com/altctrlcode/weatherStation_Wemos
- Main branch: `main`.
- Firmware: `src/main.cpp`; PlatformIO settings: `platformio.ini`.
- Hardware, CSV contract, and parser guidance: `README.md`.
- Contribution workflow: `CONTRIBUTING.md`.
- The firmware compiled successfully for `lolin32_lite` in this conversation.
  Hardware readings and the CSV stream have not been verified by the agent.
- The user created a Django project under `web/` and reported that its
  development server works. Inspection confirmed Python 3.14.7, Django 5.2.18,
  and mysqlclient 2.3.0 in `web/.venv/`. Reproducible dependencies are recorded
  in `web/requirements.txt`; the virtual environment is not committed.
- `web/config/settings.py` still uses SQLite. No `readings` app, sensor model,
  or serial collector has been implemented yet. MariaDB connection is next.
- The generated local Django secret key was replaced with an environment
  lookup and an explicit public development-only fallback before publishing.
  Local production secrets and database files are not committed.

## Hardware and serial contract

The board is a Wemos LOLIN32 Lite (ESP32) with a combined AHT20/BMP280 breakout.
SCL is GPIO 4 and SDA is GPIO 16. Power is 3V3 with shared GND.
The firmware uses Adafruit AHTX0 and BMP280 libraries and tries BMP280 at
`0x76`, then `0x77`; AHT20 is at `0x38`.

The serial connection is 115200 baud, with a header once per boot and a row
about every two seconds. The exact header is:

```csv
uptime_ms,aht20_temperature_c,aht20_humidity_pct,bmp280_temperature_c,bmp280_pressure_pa,estimated_altitude_m
```

Keep the six-column schema stable unless the receiver and documentation are
also updated. Empty readings represent missing values, not zero. Altitude is
calculated using `SEA_LEVEL_PRESSURE_HPA`, defaulting to 1013.25 hPa.
Device uptime resets at boot; the Python receiver should add a UTC timestamp.
Ignore ESP32 boot noise, partial/malformed rows, and repeated headers.
Only one process should open the serial port at a time.

## MariaDB status and intended Django design

- On the original Windows machine, the agent verified a running `MariaDB`
  service and client at `C:\Program Files\MariaDB 13.0\bin\mariadb.exe`.
- PowerShell initially could not find `mariadb` on PATH. The working full-path
  command supplied to the user was:

  ```powershell
  & "C:\Program Files\MariaDB 13.0\bin\mariadb.exe" -u root -p
  ```

- The user confirmed creating a weather database and a `weather_logger`
  account on the original computer. The exact database spelling remains
  unconfirmed: they wrote `wather_station`, while earlier instructions used
  `weather_station`. Check the real server rather than guessing.
- Creating a separate local `weather_django` account with migration privileges
  was explained, but completion has NOT been confirmed. On a fresh home
  server, use canonical name `weather_station` and follow HOME_SETUP.md.
- No database passwords have been supplied to Codex or stored in this file.
- Plan: define the reading table as a Django model and use migrations to
  create it. Connect Django through `django.db.backends.mysql` and
  `mysqlclient`, after selecting compatible Python/Django versions.
- Proposed local collector: a separate Django management command, such as
  `python manage.py collect_weather --port COM5`, using Django models to
  insert readings. The port name is an example and must be discovered locally.
- For a remotely hosted website, the proposed collector sends readings to an
  authenticated HTTPS endpoint; Django writes them to its production database.
  This hosting design is a proposal, not an implemented feature.

## Continue on another machine

1. Clone or pull this repository and open it in VS Code with Codex.
2. Read this file, `README.md`, `CONTRIBUTING.md`, and the actual source before
   editing. Check Git status and preserve existing changes.
3. Follow HOME_SETUP.md to recreate the Python environment and install a
   fresh local MariaDB server. Original installations, databases, accounts,
   and credentials are not transferred by cloning the repository.
4. Verify the home database and account setup. Do not overwrite existing
   accounts or assume example passwords were used.
5. Connect the EXISTING Django project to MariaDB, run its migrations, and
   then create the readings app and model with station identity, UTC timestamp,
   device uptime, and the five sensor/derived values. Use nullable fields for
   missing readings. Do not run `startproject` again.
6. Implement the serial collector with optional CSV recording, then the
   Django views/API for showing readings. Verify parsing, missing readings,
   resets, and database insertion with meaningful tests.

Environment files, Python virtual environments, CSV recordings, and local
database files are ignored by Git. Each machine needs its own environment
and database setup; share configuration examples and migrations through Git.
