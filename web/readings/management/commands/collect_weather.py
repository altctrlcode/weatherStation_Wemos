import csv
import math

import serial
from django.core.management.base import BaseCommand, CommandError

from readings.models import WeatherReading


class Command(BaseCommand):
    help = "Read weather data from USB serial and save it to the database."

    def add_arguments(self, parser):
        parser.add_argument("--port", required=True)
        parser.add_argument("--station", default="weather-station-1")

    def handle(self, *args, **options):
        station = options["station"]
        if not station or len(station) > 64:
            raise CommandError("Station name must contain 1–64 characters.")

        try:
            with serial.Serial(
                options["port"], baudrate=115200, timeout=1
            ) as connection:
                self.stdout.write(
                    f"Listening on {options['port']}. Press Ctrl+C to stop."
                )

                # Discard the first fragment in case we joined mid-row.
                discard = True

                while True:
                    raw = connection.read_until(b"\n", size=512)
                    if not raw:
                        continue

                    # A timeout or size limit can return an incomplete line.
                    if not raw.endswith(b"\n"):
                        discard = True
                        continue

                    if discard:
                        discard = False
                        continue

                    try:
                        text = raw.decode("ascii").strip()
                        fields = next(csv.reader([text], strict=True))

                        # Ignore headers, startup messages, and invalid rows.
                        if len(fields) != 6:
                            continue
                        if fields[0] == "uptime_ms":
                            continue
                        if not fields[0].isascii() or not fields[0].isdigit():
                            continue

                        uptime = int(fields[0])
                        if uptime > 2**63 - 1:
                            continue

                        values = [
                            None if field.strip() == "" else float(field)
                            for field in fields[1:]
                        ]
                        if any(
                            value is not None and not math.isfinite(value)
                            for value in values
                        ):
                            continue
                    except (UnicodeDecodeError, ValueError, csv.Error):
                        continue

                    reading = WeatherReading.objects.create(
                        station_id=station,
                        uptime=uptime,
                        aht20_temperature_C=values[0],
                        aht20_humidity_pct=values[1],
                        bmp280_temperature_C=values[2],
                        bmp280_pressure_pa=values[3],
                        estimated_altitude_m=values[4],
                    )

                    self.stdout.write(f"Saved reading {reading.pk}")

        except serial.SerialException as error:
            raise CommandError(f"Serial connection failed: {error}") from error
        except KeyboardInterrupt:
            self.stdout.write("Collector stopped.")