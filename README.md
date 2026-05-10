# Final Project - KirSpaceB

AESD (Advanced Embedded Software Development) Final Project by Kirk Flores.

This project implements a Linux kernel character device driver (`leddriver`)
that controls a GPIO LED on a Raspberry Pi 4 Model B, along with a userspace
control application, all packaged into a custom Yocto kirkstone image.

## Project Overview & Schedule

Full project documentation lives in this repository's wiki:

- **[Project Overview](https://github.com/cu-ecen-aeld/final-project-KirSpaceB/wiki/Project-Overview)** — overview, video demonstration, challenges, and lessons learned
- **[Project Schedule](https://github.com/cu-ecen-aeld/final-project-KirSpaceB/wiki/Project-Schedule)** — sprint breakdown and milestones
- **[Wiki Home](https://github.com/cu-ecen-aeld/final-project-KirSpaceB/wiki)**

## Repository Structure

- `led-driver/` — Linux kernel character device driver source and Yocto recipe (`led-driver_git.bb`) for `/dev/leddriver`
- `app/` — Userspace LED control application (`ledcontrol.c`) and Yocto recipe (`ledcontrol_git.bb`)

## Hardware Target

- Raspberry Pi 4 Model B
- GPIO17 LED
- Custom Yocto image based on `core-image-aesd` (kirkstone branch)

## Build

This project is built as part of a Yocto image using the `meta-aesd` layer.
The kernel module and userspace app are included via their respective `.bb`
recipes referenced in the image configuration.

## Hardware Test Results

Deployed `core-image-aesd` to Raspberry Pi 4. Verified `leddriver` kernel
module loads successfully. GPIO17 LED blinks as expected via `/dev/leddriver`
and via the `ledcontrol` userspace application (`./ledcontrol on` / `./ledcontrol off`).
