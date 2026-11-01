from django.contrib import admin

from .models import WeatherReading

@admin.register(WeatherReading)
class WeatherReadingAdmin(admin.ModelAdmin):
    list_display = ('station_id', 
                    'received_at', 
                    'aht20_temperature_C', 
                    'aht20_humidity_pct', 
                    'bmp280_pressure_pa',
                    )
    list_filter = ('station_id',)
    search_fields = ('received_at',)