from datetime import timedelta

from django.shortcuts import render
from django.utils import timezone

from .models import WeatherReading


def dashboard(request):
    reading = WeatherReading.objects.order_by('-received_at', '-pk').first()

    is_stale = (
        reading is None
        or timezone.now() - reading.received_at > timedelta(seconds= 15)
    )

    return render(request, 'readings/dashboard.html', {
        'reading': reading,
        'is_stale': is_stale,
    })
