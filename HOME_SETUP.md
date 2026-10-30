# Continue the weather station project at home

Checkpoint: 2026-10-30. These instructions assume another Windows computer.
The user wants to perform the setup and learn each step; guide them rather
than installing or implementing the remaining features automatically.

## 1. Get the code

Install Git and VS Code if needed, then in PowerShell:

```powershell
git clone https://github.com/altctrlcode/weatherStation_Wemos.git
cd weatherStation_Wemos
code .
```

If you already cloned the repository, open that folder and run `git status`.
If your working tree is clean, run `git pull --ff-only` instead of cloning again.
Preserve any local changes before updating.

## 2. Recreate the Python environment

The original computer uses Python 3.14.7, Django 5.2.18, and mysqlclient 2.3.0.
Install 64-bit Python 3.14 from https://www.python.org/downloads/windows/.
Use the current patch release of that Python series. Restart VS Code after
installation. From the repository root:

```powershell
py -3.14 --version
cd web
py -3.14 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
python -m django --version
python manage.py check
```

The Django version should be `5.2.18`; the check should report no issues.
Do not run `startproject` again: `manage.py` and `config/` are already present.
In VS Code, use **Python: Select Interpreter** and select
`web\.venv\Scripts\python.exe`.

If PowerShell blocks activation, activation is optional. Use this form for
each Python command instead, without changing execution policy:

```powershell
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe manage.py check
```

If mysqlclient installation fails, keep the full error and resolve it before
continuing with the MariaDB connection. Do not substitute another database
driver silently.

## 3. Install a new local MariaDB server

Download the Windows x86-64 MSI for a supported MariaDB Community LTS release
(12.3 LTS is a suitable choice): https://mariadb.org/download/.
The original computer has MariaDB 13.0; its installation is not copied by Git.

In the installer:

1. Install the server, database instance, and command-line client.
2. Choose a new root password and keep it private.
3. Leave remote root access and anonymous accounts disabled.
4. Install as a Windows service named `MariaDB`.
5. Enable TCP/IP, using port 3306 if it is available. If you choose another
   port, use it consistently in the client commands and Django settings.

For local development, keep the server accessible only locally. There is no
need to add an inbound firewall rule for this setup. If you restrict the
server explicitly, use `bind-address=127.0.0.1` in its `[mysqld]` configuration
and restart the service from Windows Services.

Open a **new** PowerShell terminal and connect:

```powershell
mariadb -u root -p
```

If `mariadb` is not recognized, use its full path. Adjust the version folder
to match the server you installed:

```powershell
& "C:\Program Files\MariaDB 12.3\bin\mariadb.exe" -u root -p
```

Optionally add that `bin` folder to your user PATH, then restart VS Code.

## 4. Create the database and accounts on the NEW server

The following SQL is for a fresh home server. Use `weather_station` as the
database name on this new machine. The original database's spelling was not
confirmed (`weather_station` versus `wather_station`), so do not assume an
existing server has the same spelling.

At the MariaDB root prompt, replace the password placeholders with new,
different passwords of your own, then run:

```sql
SELECT VERSION();

CREATE DATABASE weather_station CHARACTER SET utf8mb4;

CREATE USER 'weather_django'@'localhost'
  IDENTIFIED BY 'REPLACE_WITH_YOUR_DJANGO_PASSWORD';

GRANT ALL PRIVILEGES ON weather_station.*
  TO 'weather_django'@'localhost';

CREATE USER 'weather_logger'@'localhost'
  IDENTIFIED BY 'REPLACE_WITH_YOUR_LOGGER_PASSWORD';

GRANT SELECT, INSERT ON weather_station.*
  TO 'weather_logger'@'localhost';

SHOW DATABASES;
EXIT;
```

`weather_django` is a local development account: it can create and change
tables through Django migrations, but its privileges are scoped to this
database. `weather_logger` is optional for a future standalone collector.
The proposed Django management-command collector will initially use the
Django connection and models. No `FLUSH PRIVILEGES` is needed after these
account-management statements.

If a database or account already exists, stop and inspect it instead of
dropping or replacing it. Never paste actual passwords into Git or Codex.

Test the new Django account from PowerShell (use the full executable path
above if the short command is unavailable):

```powershell
mariadb --protocol=tcp -h localhost -P 3306 -u weather_django -p weather_station
```

At its SQL prompt:

```sql
SELECT DATABASE(), CURRENT_USER();
EXIT;
```

Expect `weather_station` and `weather_django@localhost`.
This new database is empty. Git transfers code, not the original server's
accounts or readings. No database export or restoration was performed.

## 5. Resume here: connect Django to MariaDB

The committed Django settings STILL use the default SQLite database. MariaDB
has not yet been wired into Django, and there is no readings model or collector.
Do this connection as the next guided exercise, before running migrations.

In `web/config/settings.py`, replace the existing `DATABASES` block with:

```python
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.mysql',
        'NAME': 'weather_station',
        'USER': 'weather_django',
        'PASSWORD': os.environ['WEATHER_DB_PASSWORD'],
        'HOST': 'localhost',
        'PORT': '3306',
        'OPTIONS': {'charset': 'utf8mb4'},
    }
}
```

`os` is already imported in the committed settings. Supply the database
password locally in your active PowerShell terminal. Keep the real value
out of the source code and Git:

```powershell
$env:WEATHER_DB_PASSWORD = 'YOUR_ACTUAL_DJANGO_DATABASE_PASSWORD'
```

This variable lasts only for that terminal session. A `.env` file is ignored
by Git, but the project does not yet load `.env` files automatically.

Then, from `web/` with the virtual environment active:

```powershell
python manage.py check --database default
python manage.py migrate --database default
python manage.py createsuperuser
python manage.py runserver
```

Open http://127.0.0.1:8000/admin/ and log in with the Django superuser you just
created. This is a website account, separate from MariaDB's database accounts.
Verify the Django tables exist in MariaDB with `SHOW TABLES;` while connected
to `weather_station`.

The committed settings use a public development-only fallback secret key so
the local welcome page works without secrets in Git. For a stable private
local key, generate one, then set `DJANGO_SECRET_KEY` locally and retain it
outside version control:

```powershell
python -c "from django.core.management.utils import get_random_secret_key; print(get_random_secret_key())"
```

`DEBUG=True` and the development key are for local work; production settings
are not configured at this checkpoint.

## 6. Build the next feature, one step at a time

1. Create the app with `python manage.py startapp readings` and register it
   in `INSTALLED_APPS`.
2. Define a `WeatherReading` model with station identity, UTC collection time,
   device uptime, both temperatures, humidity, pressure, and calculated
   altitude. Allow missing sensor values with nullable fields.
3. Run `makemigrations` and `migrate`; register the model in Django admin.
4. Add `readings/management/commands/collect_weather.py`, a separate process
   that validates the serial CSV and saves records through the Django model.
   Install and record `pyserial` when implementing that feature.
5. Connect the ESP32, discover its current COM port, and verify a real reading
   appears in MariaDB and Django admin. Close Serial Monitor before the
   collector opens the port. Without the board, clearly label simulated data.
6. Build the dashboard after this end-to-end milestone works.

Read `README.md` for the six-column serial contract and `PROJECT_HANDOFF.md`
for project decisions. Database setup at home does not require changing the
ESP32 firmware.

## Prompt to paste into Codex at home

> Read HOME_SETUP.md, PROJECT_HANDOFF.md, README.md, and CONTRIBUTING.md.
> I want to do the work myself, so guide me step by step. First verify my new
> local MariaDB server, weather_station database, weather_django account, and
> Python environment. Then guide me through connecting the existing Django
> project to MariaDB. Do not assume accounts or data transferred through Git,
> and do not create a second Django project or overwrite existing databases.

References: [MariaDB Windows installation](https://mariadb.com/docs/server/server-management/install-and-upgrade-mariadb/installing-mariadb/binary-packages/installing-mariadb-msi-packages-on-windows),
[Django database configuration](https://docs.djangoproject.com/en/5.2/ref/databases/),
and [Django Windows installation](https://docs.djangoproject.com/en/5.2/howto/windows/).
