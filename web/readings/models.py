from django.db import models

class WeatherReading(models.Model):
    station_id = models.CharField(max_length=64)
    received_at = models.DateTimeField(auto_now_add=True)
    uptime = models.PositiveBigIntegerField()

    aht20_temperature_C = models.FloatField(null=True, blank=True)
    aht20_humidity_pct = models.FloatField(null=True, blank=True)
    bmp280_temperature_C = models.FloatField(null=True, blank=True)
    bmp280_pressure_pa = models.FloatField(null=True, blank=True)
    estimated_altitude_m = models.FloatField(null=True, blank=True)

    class Meta:
        ordering = ['-received_at']
        indexes = [
            models.Index(fields=['station_id', 'received_at']),
        ]

    def __str__(self):
        return f'{self.station_id} - {self.received_at}'
    
