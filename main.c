#include <stdio.h>
#include <stdlib.h>


void MFTparser(unsigned char *buffer) {
    int i;
    for (i = 0; i < 512; i++) {
        printf("0x%.2X", buffer[i]);
        //printf("(%i)0x%.2X", i, buffer[i]);
        printf(" ");

        if (i % 16 == 0) printf("\n");

    }
}


void FileParser(unsigned char *file){
    int i;
    char size[4];

    switch (file[22]){
        case 0x00:
            printf("\nФайловая запись не используется\n");
            break;
        case 0x01:
            printf("\nФайловая запись используется и описывает файл\n");
            break;
        case 0x02:
            printf("\nФайловая запись используется и описывает каталог\n");
            break;
        default:
            printf("\nОшибка определения типа файловой записи\n");
    }
    printf("Реальный размер (real size) файловой записи: 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", file[24], file[25], file[26], file[27]);
    for (i = 0; i< 4; i++) size[i]=file[24+i];

    int number=strtol(file, NULL, 16);

    printf("Size in bytes: %i\n", number);


    printf("Update sequence number: %.2x %.2x\n", file[4], file[5]);
    printf("Размер в словах номера последовательности обновления и массива обновления: %.2x %.2x\n", file[6], file[7]);
    printf("$Log file sequence number: "); for (i=0; i<8; i++) printf("%.2x ",file[8+i]);
    printf("\nSequence number: %.2x %.2x\n", file[16], file[17]);
    printf("Счетчик жестких ссылок: %.2x %.2x\n", file[18], file[19]);
    printf("Смещение перовго атрибута: %.2x %.2x\n", file[20], file[21]);

}

int main(int argc, char *argv[]) {

    FILE *diskImage;
    FILE *out;
    int i;
    int j, byteCount;
    unsigned char buffer[1500000];
    unsigned char file[512];

    diskImage = fopen(argv[1], "rb");
    out = fopen("file.txt", "w");
    printf("Opened file \n");

    fread(buffer, 1, 1500000, diskImage);


    printf("Read file\n");
    fclose(diskImage);
    int a = 0;
    int numOfFiles = 0;
    for (i = 0; i < 1500000; i++) {
        if (i == 0) fprintf(out, "00000000  ");
        fprintf(out, " 0x%.2X ", buffer[i]);

        if ((i + 1) % 4 == 0) fprintf(out, "  ");


        if (i != 0 && (i + 1) % 16 == 0) {
            a += 16;

            fprintf(out, "\n%08x  ", a);

        }

        if (buffer[i] == 0x46 && buffer[i + 1] == 0x49 && buffer[i + 2] == 0x4c && buffer[i + 3] == 0x45) {
            numOfFiles++;
            printf("\nFile %i:       ", numOfFiles);
            printf("%08x\n", a);
            j = i;
            byteCount = 0;
            while (!(buffer[j] == 0xFF && buffer[j + 1] == 0xFF && buffer[j + 2] == 0xFF && buffer[j + 3] == 0xFF)) {

                file[byteCount] = buffer[j];
                j++;
                byteCount++;
            }
            for (int k=0; k<4; k++){
                file[byteCount+k]=0xff;
            }

            for (int k = 0; k < byteCount+4; k++) printf("%.2x ", file[k]);

            FileParser(file);


        }

    }
    printf("\n\nDisk signature: %.2X %.2X %.2X %.2X\n", buffer[440], buffer[441], buffer[442], buffer[443]);
    printf("Первый раздел:\n");
    for (i = 446; i < 462; i++) {
        printf("%.2X ", buffer[i]);
    }

    printf("\nBoot indicator: %.2X  -- not active\n", buffer[446]);
    printf("Starting CHS (Cylinder - Head - Sector) values: %.2X %.2X %.2X\n", buffer[449], buffer[448], buffer[447]);
    printf("File system: %.2X -- Linux native partition\n", buffer[450]);
    printf("Ending CHS values: %.2X %.2X %.2X\n", buffer[453], buffer[452], buffer[451]);
    printf("Starting sector: %.2X %.2X %.2X %.2X -- Sector 800h\n", buffer[454], buffer[455], buffer[456], buffer[457]);
    printf("Size of partition: %.2X %.2X %.2X %.2X -- 202752 sectors = 103 809 024 bytes = 103.8 MB \n", buffer[458],
           buffer[459], buffer[460], buffer[461]);

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

    printf("Num of files: %i", numOfFiles);
//    MFTparser(mft);


    return 0;

}