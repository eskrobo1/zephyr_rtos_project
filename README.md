# Zephyr RTOS - Demonstracija osnovnih koncepata

Ovaj repozitorij sadrži praktične primjere implementacije osnovnih funkcionalnosti Zephyr RTOS-a na QEMU simulatoru i Raspberry Pi 4B platformi.

## Struktura projekta
├── 01_zephyr-thread-management/     # Kreiranje i upravljanje nitima
├── 02_zephyr-thread-communication/  # Message queue i pipe komunikacija
├── 03_zephyr-thread-synchronization/ # Mutex i semafor mehanizmi
└── 04_zephyr-timers/                # Softverski timeri i GPIO kontrola
│   ├── build/
│   ├── src/
│   │   └── main.c
│   ├── CMakeLists.txt
│   ├── deploy.sh
│   └── prj.conf

Struktura projekta

├── 01_zephyr-thread-management/     # Kreiranje i upravljanje nitima
├── 02_zephyr-thread-communication/  # Message queue i pipe komunikacija
├── 03_zephyr-thread-synchronization/ # Mutex i semafor mehanizmi
└── 04_zephyr-timers/                # Softverski timeri i GPIO kontrola
    ├── build/
    ├── src/
    │   └── main.c
    ├── CMakeLists.txt
    ├── deploy.sh
    └── prj.conf

zephyr-rtos-demo/
├── 01_zephyr-thread-management/ # Kreiranje i upravljanje nitima
├── 02_zephyr-thread-communication/ # Message queue i pipe komunikacija
├── 03_zephyr-thread-synchronization/ # Mutex i semafor mehanizmi
└──04_zephyr-timers/ # Softverski timeri i GPIO kontrola
│  ├── build/ # (generira se prilikom build-a)
│  ├── src/
│  │ └── main.c 
│  ├── CMakeLists.txt # CMake konfiguracija
│  ├── deploy.sh # Skripta za deployment na SD karticu
│  └── prj.conf # Konfiguracija projekta

## Preduslovi

Prije pokretanja projekta, potrebno je imati instalirane sljedeće komponente:

- Zephyr SDK 0.16.5 ili noviji  
- West build alat  
- Python 3.12+  
- CMake 3.20.5+  

Za detaljna uputstva o postavljanju razvojnog okruženja, pratite zvaničnu Zephyr dokumentaciju:  
https://docs.zephyrproject.org/latest/develop/getting_started/index.html

---
## Preduslovi

Prije pokretanja projekta, potrebno je imati instalirane sljedeće komponente:

- Zephyr SDK 0.16.5 ili noviji  
- West build alat  
- Python 3.12+  
- CMake 3.20.5+  

Za detaljna uputstva o postavljanju razvojnog okruženja, pratite zvaničnu Zephyr dokumentaciju:  
https://docs.zephyrproject.org/latest/develop/getting_started/index.html

---

## Instalacija i pokretanje primjera

```bash
# Dodaj alias za aktivaciju Zephyr okruženja
echo 'alias zephyr-env="source ~/zephyrproject/.venv-py312/bin/activate && source ~/zephyrproject/zephyr/zephyr-env.sh"' >> ~/.bashrc
source ~/.bashrc

# Kloniraj repozitorij i inicijalizuj Zephyr workspace
git clone https://github.com/eskrobo1/zephyr_rtos_project.git
cd zephyr_rtos_project
# Aktivacija environment-a
source ~/.bashrc
zephyr-env

# Pokreni QEMU primjer
cd 01_zephyr-thread-management
west build -p always -b qemu_cortex_a53 .
west build -t run

# Pokreni primjer za Raspberry Pi 4B
cd ../04_zephyr-timers
west build -p always -b rpi_4b .
./deploy.sh
