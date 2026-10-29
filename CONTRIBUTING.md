# Contributing

Clone the repository and open the folder in VS Code with the PlatformIO
extension. The hardware wiring, serial CSV schema, and build instructions
are documented in [README.md](README.md).

## Make a change

Start from an up-to-date `main` branch and create a branch for your work:

```sh
git switch main
git pull --ff-only
git switch -c feature/describe-your-change
```

Build the firmware with **PlatformIO: Build** or `pio run` before committing.
If you change sensor readings or serial output, also check the output on
hardware when available and update the CSV format documentation. Describe
any hardware checks you could not perform in the pull request.

Stage the files you changed, commit, and publish your branch. For example,
for a change to the firmware and its documentation:

```sh
git add src/main.cpp README.md
git commit -m "Describe the change"
git push -u origin feature/describe-your-change
```

Open a pull request on GitHub targeting `main` so collaborators can review
the change before it is merged. Include what changed and how you checked it.

## Files to share

Commit source code, PlatformIO configuration, and documentation. Generated
build files, local Python environments, collected CSV readings, local
databases, and `.env` credentials are ignored by Git. If a small example
dataset is needed, agree on a location and update `.gitignore` deliberately.
