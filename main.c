#include <stdio.h>


int main(int argc, char *argv[]) {

    FILE *diskImage;
    int i;
    unsigned char buffer[512];


    diskImage = fopen(argv[1], "rb");
    printf("Opened file\n");

    fread(buffer, 1, 512, diskImage);
    printf("Read file\n");
    fclose(diskImage);

    for (i = 0; i<512; i++){
        printf("0x%.2X", buffer[i]);
        //printf("(%i)0x%.2X", i, buffer[i]);
        printf(" ");

        if (i % 16 == 0) printf("\n");

    }
    printf("\n\nDisk signature: %.2X %.2X %.2X %.2X\n", buffer[440], buffer[441], buffer[442], buffer[443]);
    printf("Первый раздел:\n");
    for (i = 446; i < 462; i++) {
        printf("%.2X ", buffer[i]);
    }

    printf("\nBoot indicator: %.2X  -- not active\n", buffer[446]);
    printf("Starting CHS (Cylinder - Head - Sector) values: %.2X %.2X %.2X\n",buffer[449], buffer[448], buffer[447]);
    printf("File system: %.2X -- Linux native partition\n", buffer[450]);
    printf("Ending CHS values: %.2X %.2X %.2X\n", buffer[453], buffer[452], buffer[451]);
    printf("Starting sector: %.2X %.2X %.2X %.2X -- Sector 800h\n", buffer[454], buffer[455], buffer[456], buffer[457]);
    printf("Size of partition: %.2X %.2X %.2X %.2X -- 202752 sectors = 103 809 024 bytes = 103.8 MB \n", buffer[458], buffer[459], buffer[460], buffer[461]);

    printf("\nВторой раздел:\n");
    for (i = 462; i < 478; i++) {
        printf("%.2X ", buffer[i]);
    }

    printf("\nТретий раздел:\n");
    for (i = 478; i < 494; i++) {
        printf("%.2X ", buffer[i]);
    }

    printf("\nЧетвертый раздел:\n");
    for (i = 494; i < 510; i++) {
        printf("%.2X ", buffer[i]);
    }
    printf("\n");


    return 0;

}