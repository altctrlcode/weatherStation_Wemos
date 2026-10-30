# LOLIN32 Lite weather station

Reads all four environmental measurements from an AHT20 + BMP280 breakout:

- AHT20 temperature in degrees Celsius and relative humidity in %RH.
- BMP280 temperature in degrees Celsius and atmospheric pressure in Pa.
- Estimated altitude in metres, calculated from pressure.

## Wiring

| Breakout pin | LOLIN32 Lite pin |
| --- | --- |
| VCC / VIN | 3V3 |
| GND | GND |
| SCL | GPIO 4 |
| SDA | GPIO 16 |

Use 3.3 V power and I2C pull-ups to 3.3 V. Breakouts usually include pull-ups;
if yours does not, add them as specified by its manufacturer.

## Build and run

1. Open this folder in VS Code with the PlatformIO extension.
2. Select **PlatformIO: Build**. PlatformIO installs the libraries in
   `platformio.ini` automatically.
3. Select **PlatformIO: Upload** with the board connected over USB.
4. Open **PlatformIO: Serial Monitor** at **115200 baud**. Press reset to see
   the CSV header if the monitor was opened after boot.

With the PlatformIO CLI installed, the equivalent commands are:

```sh
pio run
pio run --target upload
pio device monitor --baud 115200
```

## Serial CSV format

The firmware sends one header per boot, followed by one row about every two
seconds. Example output (illustrative readings):

```csv
uptime_ms,aht20_temperature_c,aht20_humidity_pct,bmp280_temperature_c,bmp280_pressure_pa,estimated_altitude_m
2100,23.45,48.20,23.81,100650.00,56.35
4200,23.46,48.18,23.82,100648.00,56.52
```

| Column | Meaning |
| --- | --- |
| `uptime_ms` | 64-bit milliseconds since this boot, at the start of sampling |
| `aht20_temperature_c` | AHT20 temperature in degrees Celsius |
| `aht20_humidity_pct` | AHT20 relative humidity in percent |
| `bmp280_temperature_c` | BMP280 temperature in degrees Celsius |
| `bmp280_pressure_pa` | Atmospheric pressure in pascals; divide by 100 for hPa |
| `estimated_altitude_m` | Calculated altitude in metres |

Every row has exactly six fields. Missing or non-finite readings are empty
fields; your Python code can store them as empty CSV cells or SQL `NULL`.
For example, a missing AHT20 produces:

```csv
2100,,,23.81,100650.00,56.35
```

If both sensors are unavailable, the row contains uptime followed by five
empty fields: `2100,,,,,`. The firmware does not print labels, units, status
messages, or blank lines into the data stream.

For the Python listener:

- Connect at **115200 baud** and use Python's `csv` module to parse complete
  lines. Close PlatformIO Serial Monitor first so Python can open the port.
- Ignore startup noise and partial lines. ESP32 boot messages can appear
  before application output. Recognize the exact header above; if connecting
  after startup, you can use the documented column names without waiting
  for a new header.
- Skip repeated headers after a reset. Accept data rows only when there are
  six fields, uptime is a non-negative integer, and every other field is
  either empty or a finite number.
- Add a UTC receive timestamp on the computer for each accepted row.
  Device uptime is not a calendar timestamp and restarts when the board resets.
- Write the file header only when creating an empty CSV file; avoid appending
  a duplicate header when reconnecting.

The AHT20 uses `0x38`; the firmware tries the BMP280 at `0x76`, then `0x77`.
If readings are empty, check the wiring and power, then reset the board.
A working sensor continues to be read if the other fails initialization.

Altitude is an estimate, not a separate sensor measurement. Set
`SEA_LEVEL_PRESSURE_HPA` in `src/main.cpp` to the current local sea-level
pressure (QNH) for a better estimate; the default is 1013.25 hPa.
The temperatures from the two chips may differ, particularly if the breakout
is warmed by the ESP32 or nearby components.

API references: [ESP32 I2C](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/i2c.html),
[Adafruit AHTX0](https://github.com/adafruit/Adafruit_AHTX0), and
[Adafruit BMP280](https://github.com/adafruit/Adafruit_BMP280_Library).

## Collaboration

For the existing Django project and setup on a new Windows computer, start
with [HOME_SETUP.md](HOME_SETUP.md). Current progress and pending decisions
are recorded in [PROJECT_HANDOFF.md](PROJECT_HANDOFF.md).

See [CONTRIBUTING.md](CONTRIBUTING.md) for the branch and pull request workflow.
Build artifacts, local Python environments, collected CSV files, and local
databases are excluded from version control by `.gitignore`.
