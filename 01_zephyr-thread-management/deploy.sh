#!/bin/bash

# Boje za output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Zephyr RPi4 Deployment Script ===${NC}"

# Provjerite da li build postoji
if [ ! -f "build/zephyr/zephyr.bin" ]; then
    echo -e "${RED}Error: build/zephyr/zephyr.bin not found!${NC}"
    echo "Please run 'west build' first."
    exit 1
fi

# Provjerite da li je SD kartica dostupna
if ! lsblk | grep -q mmcblk0p1; then
    echo -e "${RED}Error: SD card not detected!${NC}"
    echo "Please insert SD card and try again."
    exit 1
fi

# Mount SD karticu
echo -e "${YELLOW}Mounting SD card...${NC}"
sudo mkdir -p /mnt/sdcard
sudo mount /dev/mmcblk0p1 /mnt/sdcard

# Provjerite da li je mount uspješan
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed to mount SD card!${NC}"
    exit 1
fi

# Backup management
BACKUP_DIR="/mnt/sdcard/backups"
CURRENT_KERNEL="/mnt/sdcard/kernel8.img"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Kreirajte backup direktorij ako ne postoji
sudo mkdir -p "$BACKUP_DIR"

# Ako postoji trenutni kernel, napravi backup
if [ -f "$CURRENT_KERNEL" ]; then
    echo -e "${YELLOW}Creating backup of current kernel...${NC}"
    BACKUP_NAME="kernel8_backup_${TIMESTAMP}.img"
    sudo cp "$CURRENT_KERNEL" "$BACKUP_DIR/$BACKUP_NAME"
    echo -e "${GREEN}Backup saved as: $BACKUP_NAME${NC}"
    
    # Održavaj samo poslednje 3 backup-a
    echo -e "${YELLOW}Cleaning old backups (keeping last 3)...${NC}"
    cd "$BACKUP_DIR"
    BACKUP_COUNT=$(ls -1 kernel8_backup_*.img 2>/dev/null | wc -l)
    
    if [ $BACKUP_COUNT -gt 3 ]; then
        # Briši najstarije backup-ove
        ls -1t kernel8_backup_*.img | tail -n +4 | xargs sudo rm -f
        echo -e "${GREEN}Old backups removed${NC}"
    fi
    cd - > /dev/null
fi

# Kopiraj novi kernel
echo -e "${YELLOW}Deploying new kernel...${NC}"
sudo cp build/zephyr/zephyr.bin "$CURRENT_KERNEL"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}New kernel deployed successfully!${NC}"
else
    echo -e "${RED}Error: Failed to copy kernel!${NC}"
    sudo umount /mnt/sdcard
    exit 1
fi

# Prikaži info o verzijama
echo -e "\n${GREEN}=== Deployment Info ===${NC}"
echo "Current kernel size: $(ls -lh $CURRENT_KERNEL | awk '{print $5}')"
echo "Build time: $(stat -c %y build/zephyr/zephyr.bin | cut -d'.' -f1)"
echo "Backups available:"
ls -1 "$BACKUP_DIR"/kernel8_backup_*.img 2>/dev/null | tail -3 | while read backup; do
    echo "  - $(basename $backup) ($(ls -lh $backup | awk '{print $5}'))"
done

# Sync i unmount
echo -e "\n${YELLOW}Syncing and unmounting...${NC}"
sudo sync
sudo umount /mnt/sdcard

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ SD card ready! You can remove it safely.${NC}"
else
    echo -e "${RED}Warning: Unmount may have failed. Check manually.${NC}"
fi

echo -e "\n${GREEN}Deployment complete!${NC}"
