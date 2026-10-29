# Weather station project handoff

This is a working-context summary for continuing in Codex on another machine,
not an export of the original conversation. Last updated: 2026-10-29.

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
- No Python collector or Django application has been implemented yet.

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

- Creating database `weather_station` with `utf8mb4` and local development
  account `weather_django` was explained, but completion has NOT been
  confirmed. The suggested development account has privileges limited to
  `weather_station.*`, including those needed by Django migrations.
- A separate optional `weather_logger` account with SELECT/INSERT privileges
  was suggested earlier; its existence has NOT been confirmed either.
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
3. Check Python, PlatformIO, MariaDB, serial-port availability, and database
   setup on that machine. The original machine's installation and database
   are not transferred by cloning the repository.
4. Confirm whether the database/account setup above was completed. Do not
   overwrite existing accounts or assume example passwords were used.
5. Create the Django application, its MariaDB configuration, and a reading
   model with station identity, UTC timestamp, device uptime, and the five
   sensor/derived values. Use nullable fields for missing readings.
6. Implement the serial collector with optional CSV recording, then the
   Django views/API for showing readings. Verify parsing, missing readings,
   resets, and database insertion with meaningful tests.

Environment files, Python virtual environments, CSV recordings, and local
database files are ignored by Git. Each machine needs its own environment
and database setup; share configuration examples and migrations through Git.
