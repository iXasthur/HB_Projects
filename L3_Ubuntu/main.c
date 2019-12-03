#include <stdio.h>
#include <sys/io.h>
#include <stdint.h>
#include "pci_c_header.h"

#define bool int
#define true 1
#define false 0

void printVendor(int vendor) {
	bool flag = true;
	int i = 0;
	while ((i < PCI_VENTABLE_LEN) && flag) {
		if (PciVenTable[i].VenId == vendor) {
			printf("Vendor: %s\n", PciVenTable[i].VenShort);
			flag = false;
		}
		i++;
	}
	if (flag == true) {
		printf("Vendor: Unknown VendorID\n");
	}
}

void printDev(int device, int vendor) {
	bool flag = true;
	int i = 0;
	while ((i < PCI_DEVTABLE_LEN) && flag) {
		if (PciDevTable[i].DevId == device && PciDevTable[i].VenId == vendor) {
			printf("Device: %s\n", PciDevTable[i].Chip);
			flag = false;
		}
		i++;
	}
	if (flag == true) {
		printf("Device: Unknown DeviceID\n");
	}
}

void outputIDInfo(uint32_t regResult){
	int VendorID = regResult & (uint32_t)0xFFFF;
	int DeviceID = (regResult >> 16) & (uint32_t)0xFFFF;
    printf("VendorID: %04X\n",VendorID);
    printVendor(VendorID);
	printf("DeviceID: %04X\n",DeviceID);
	printDev(DeviceID, VendorID);
}

int isBridge(uint32_t regResult) {
	int HeaderTypeFirstBit = (regResult >> 16) & (uint32_t)0b10000000;
	return HeaderTypeFirstBit;
}

void outputInterruptPin(uint32_t regResult) {
	uint32_t pin = (regResult >> 8) & 0b11111111;
	printf("Interrupt pin: ");
	switch (pin)
	{
	case 0:
		printf("Isn\'t used");
		break;
	case 1:
		printf("INTA#");
		break;
	case 2:
		printf("INTB#");
		break;
	case 3:
		printf("INTC#");
		break;
	case 4:
		printf("INTD#");
		break;
	case 5:
		printf("FFh - reserved");
		break;
	default:
		printf("Unknown state");
		break;
	}
	printf("\n");
}

void outputIOBase(uint32_t regResult) {
	uint32_t IOBase = regResult & 0b11111111;
	printf("I/O Base: %d\n",IOBase);
}

void outputIOBaseUpper(uint32_t regResult) {
	uint32_t IOBaseU = regResult & 0xFFFF;
	printf("I/O Base Upper: %d\n",IOBaseU);
}

void outputClassCode(uint32_t regResult) {
	regResult = regResult >> 8;
	uint32_t ClassCode_Int = regResult & 0b11111111;
	regResult = regResult >> 8;
	uint32_t ClassCode_Sub = regResult & 0b11111111;
	regResult = regResult >> 8;
	uint32_t ClassCode_Base = regResult & 0b11111111;

	bool flag = true;
	int i = 0;
	while ((i < PCI_CLASSCODETABLE_LEN) && flag) {
		if (PciClassCodeTable[i].BaseClass == ClassCode_Base &&
			PciClassCodeTable[i].SubClass == ClassCode_Sub &&
			PciClassCodeTable[i].ProgIf == ClassCode_Int) {
			printf("Base class(%d): ", ClassCode_Base);
			printf("%s\n", PciClassCodeTable[i].BaseDesc);
			printf("Subclass(%d): ", ClassCode_Sub);
			printf("%s\n", PciClassCodeTable[i].SubDesc);
			printf("Interface(%d): ", ClassCode_Int);
			printf("%s\n", PciClassCodeTable[i].ProgDesc);
			flag = false;
		}
		i++;
	}

	if (flag) {
		printf("Base class: %d\n", ClassCode_Base);
		printf("Subclass: %d\n", ClassCode_Sub);
		printf("Interface: %d\n", ClassCode_Int);
	}
	
}

int main() {
	if (iopl(3)) {
		printf("I/O Privilege level change error.\n");
		return 1;
	}
	else if (iopl(3) == 0){
		printf("I/O Privilege level changed successfully.\n\n");
			for(int i = 0; i < 256; i++) {
				for(int j = 0; j < 32; j++) {
					for(int k = 0; k < 8; k++) {	
						const uint32_t addr = (1 << 31) + (i << 16) + (j << 11) + (k << 8);
						outl(addr, 0xCF8);
						uint32_t regResult = inl(0xCFC);
						if (((regResult >> 16) & (uint32_t)0xFFFF) != (uint32_t)0xFFFF) {
							printf("%d.%d.%d\n", i, j, k);
							outputIDInfo(regResult);
							
							uint32_t buffAddr;
							buffAddr = addr + ((uint32_t)0x0C);
							outl(buffAddr, 0xCF8);
							regResult = inl(0xCFC);

							printf("Bridge: ");
							if (isBridge(regResult) != 0) {
								printf("YES\n");
							} else {
								printf("NO\n");
								buffAddr = addr + ((uint32_t)0x3C);
								outl(buffAddr, 0xCF8);
								regResult = inl(0xCFC);
								outputInterruptPin(regResult);

								buffAddr = addr + ((uint32_t)0x1C);
								outl(buffAddr, 0xCF8);
								regResult = inl(0xCFC);
								outputIOBase(regResult);

								buffAddr = addr + ((uint32_t)0x30);
								outl(buffAddr, 0xCF8);
								regResult = inl(0xCFC);
								outputIOBaseUpper(regResult);

								buffAddr = addr + ((uint32_t)0x08);
								outl(buffAddr, 0xCF8);
								regResult = inl(0xCFC);
								outputClassCode(regResult);
							}
							



							printf("\n");
						}
					}
				}
			}
	}		
	return 0;
}
