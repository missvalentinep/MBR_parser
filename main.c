#include <stdio.h>

int AttributeParser(unsigned char *file, int attributeOffset) {

    if ((int) file[attributeOffset + 4] == 0x00 || file[attributeOffset] == 0xFF) {
        printf("Аттрибуты закончились\n");
        return -1;
    }
    int headerSize = 24;
    printf("Смещение аттрибута: %i\n", attributeOffset);
    int attributeSize=(int) file[attributeOffset + 4];
    printf("Размер аттрибута: %i\n", attributeSize);

    printf("Тип аттрибута: ");

    switch (file[attributeOffset]) {
        case 0x10:
            printf("$STANDARD_INFORMATION \n");
            break;
        case 0x20:
            printf("$ATTRIBUTE_LIST\n");
            break;
        case 0x30:
            printf("$FILE_NAME\n");
            printf("Ссылка на родительский каталог:");
            for (int i=0; i<3; i++) printf("%.2x ", file[attributeOffset + headerSize+ i]);
            printf("\n");
            printf("Длина имени: %i символов\n", (int) file[attributeOffset + headerSize + 64]);
            printf("Имя файла: ");
            for (int i = 0; i < (int) file[attributeOffset + headerSize + 64] * 2; i++) {
                printf("%c", file[attributeOffset + headerSize + 66 + i]);
            }
            char buf;
            break;
        case 0x50:
            printf("$SECURITY_DESCRIPTOR\n");
            break;

        case 0x80:
            printf("$DATA\n");
            for (int i=0; i<attributeSize-headerSize-4; i++){
                printf("%c", file[attributeOffset+headerSize+i]);
            }
            break;
        case 0x90:
            printf("$INDEX_ROOT\n");
            break;
        case 0xa0:
            printf("$INDEX_ALLOCATION\n");
            break;
        case 0xb0:
            printf("$BITMAP\n");
            break;
        default:
            printf("Unable to identify %.2x\n", file[attributeOffset]);

    }
    printf("\n*********\n");

    return attributeOffset + (int) file[attributeOffset + 4];

}



void FileParser(unsigned char *file) {
    int i;
    int nextAttributeOffset;
    int fileSize=1024;
    int actualSize = 0;
    i = 0;

    while (i < fileSize) {
        printf("[%i]0x%.2x ", i, file[i]);
        //printf("0x%.2x ", file[i]);

        if ((i + 1) % 4 == 0) printf("  ");
        if (i != 0 && (i + 1) % 16 == 0) printf("\n");
        i++;

    }

    if (file[8]==0x01) printf("NON RESIDENT!!!!!!!!!!!!!!!!!!!!!!!!!****************************************\n");
    switch (file[22]) {
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
            printf("");
            //printf("\nОшибка определения типа файловой записи\n");
    }
    printf("Реальный размер файловой записи: 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", file[24], file[25], file[26],
           file[27]);
    actualSize = file[25] * 256 + file[24];
    printf("Размер в байтах: %i\n", actualSize);
    printf("Выделенный размер файловой записи: %.2x %.2x %.2x %.2x\n", file[28], file[29], file[30],
           file[31]);


    //printf("Update sequence number: %.2x %.2x\n", file[4], file[5]);
    //printf("Update sequence number & Array: %.2x %.2x\n", file[6], file[7]);
    //printf("$Log file sequence number: ");
    //for (i = 0; i < 8; i++) printf("%.2x ", file[8 + i]);
    //printf("\nSequence number: %.2x %.2x\n", file[16], file[17]);
    //printf("Счетчик жестких ссылок: %.2x %.2x\n", file[18], file[19]);
//    printf("Смещение перовго атрибута: %.2x %.2x\n", file[20], file[21]);
    printf("Ссылка на базовую файловую запись или ноль, если данная файловая запись базовая: ");
    for (i = 0; i < 8; i++) printf("%.2x ", file[32 + i]);
//    printf("\nИдентификатор следующего атрибута: %.2x\n", file[40]);
    printf("\n---Атрибуты---\n");
    nextAttributeOffset = AttributeParser(file, (int) file[20]);
    while (nextAttributeOffset != -1) {
        nextAttributeOffset = AttributeParser(file, nextAttributeOffset);
//        printf("\nNext attribute starts at: %i\n", nextAttributeOffset);
    }
//    nextAttributeOffset = AttributeParser(file, (int)file[20]);
//    printf("\nNext attribute starts at: %i\n", nextAttributeOffset);
//    nextAttributeOffset = AttributeParser(file, nextAttributeOffset);
//    printf("\nNext attribute starts at: %i\n", nextAttributeOffset);


}



int MBRParser(unsigned char *MBR){
    int i;
    int startOfSector;
    printf("Первый раздел:\n");

    for (i = 446; i < 462; i++) {
        printf("%.2X ", MBR[i]);
    }

    printf("\nBoot indicator: %.2X  (0 - not active)\n", MBR[446]);
    printf("Starting CHS (Cylinder - Head - Sector) values: %.2X %.2X %.2X\n", MBR[449], MBR[448], MBR[447]);
    printf("Ending CHS values: %.2X %.2X %.2X\n", MBR[453],MBR[452], MBR[451]);
    startOfSector =MBR[456]*4096+MBR[455]*256 + MBR[454];
    startOfSector*=512;
    printf("Starting sector: %.2X%.2X%.2X%.2X -- position:%i \n",MBR[457], MBR[456], MBR[455],MBR[454], startOfSector);
    printf("Size of partition: %.2X %.2X %.2X %.2X \n", MBR[458],
           MBR[459], MBR[460], MBR[461]);


    printf("\nВторой раздел:\n");
    for (i = 462; i < 478; i++) {
        printf("%.2X ", MBR[i]);
    }

    printf("\nТретий раздел:\n");
    for (i = 478; i < 494; i++) {
        printf("%.2X ",  MBR[i]);
    }

    printf("\nЧетвертый раздел:\n");
    for (i = 494; i < 510; i++) {
        printf("%.2X ",  MBR[i]);
    }
    printf("\n");

    return startOfSector;

}

int main(int argc, char *argv[]) {

    FILE *diskImage;
    FILE *out;
    int i;
    int j, byteCount;
    unsigned char MBR[512];
    unsigned char buffer[50];
    unsigned char file[1024];
    int startOfPartition;

    diskImage = fopen(argv[1], "rb");
    out = fopen("file.txt", "w");
    printf("Opened file \n");

    fread(MBR, 1, 512, diskImage);
    printf("Read $MBR\n");
    startOfPartition =MBRParser(MBR);



    fseek (diskImage , startOfPartition, SEEK_SET );
    fread (buffer, 1, 50, diskImage);

    printf("\n");
    int clusterSize;
    int sectorSize=512;

    clusterSize= buffer[13]*sectorSize;

    int startOfMft = buffer[48]*clusterSize+startOfPartition;

    printf("Start of MFT: %i\n", startOfMft);
    fseek(diskImage, startOfMft, SEEK_SET);
    int fileCount=0;
    for (i =0 ; i<100; i++){

        fread(file, 1, 1024, diskImage);

        if ((file[25] * 256 + file[24])==0){
            printf("End of files\n");
            break;
        }
        printf("\nФайл номер: %i\n", fileCount);
        FileParser(file);
        fileCount++;
    }

  /*  fread(buffer, 1, 1500000, diskImage);


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
            int fileSize = buffer[i + 25] * 256 + buffer[i + 24] - 4;
            numOfFiles++;
            printf("\n\nFile %i, size: %i       ", numOfFiles, fileSize);
            printf("%08x\n", a);
            j = i;
            byteCount = 0;
            //while (!(buffer[j] == 0xFF && buffer[j + 1] == 0xFF && buffer[j + 2] == 0xFF && buffer[j + 3] == 0xFF && buffer[j+4]==0x00)) {
            while (j < (i + fileSize)) {

                file[byteCount] = buffer[j];
                j++;
                byteCount++;
            }

            for (int k = 0; k < 4; k++) {
                file[byteCount + k] = 0xff;
            }

            //for (int k = 0; k < byteCount+4; k++) printf("%.2x ", file[k]);

            FileParser(file, fileSize);


        }

    }
//    printf("\n\nDisk signature: %.2X %.2X %.2X %.2X\n", buffer[440], buffer[441], buffer[442], buffer[443]);
//    printf("Первый раздел:\n");
//    for (i = 446; i < 462; i++) {
//        printf("%.2X ", buffer[i]);
//    }
//
//    printf("\nBoot indicator: %.2X  -- not active\n", buffer[446]);
//    printf("Starting CHS (Cylinder - Head - Sector) values: %.2X %.2X %.2X\n", buffer[449], buffer[448], buffer[447]);
//    printf("File system: %.2X -- Linux native partition\n", buffer[450]);
//    printf("Ending CHS values: %.2X %.2X %.2X\n", buffer[453], buffer[452], buffer[451]);
//    printf("Starting sector: %.2X %.2X %.2X %.2X -- Sector 800h\n", buffer[454], buffer[455], buffer[456], buffer[457]);
//    printf("Size of partition: %.2X %.2X %.2X %.2X -- 202752 sectors = 103 809 024 bytes = 103.8 MB \n", buffer[458],
//           buffer[459], buffer[460], buffer[461]);
//
//    printf("\nВторой раздел:\n");
//    for (i = 462; i < 478; i++) {
//        printf("%.2X ", buffer[i]);
//    }
//
//    printf("\nТретий раздел:\n");
//    for (i = 478; i < 494; i++) {
//        printf("%.2X ", buffer[i]);
//    }
//
//    printf("\nЧетвертый раздел:\n");
//    for (i = 494; i < 510; i++) {
//        printf("%.2X ", buffer[i]);
//    }
//    printf("\n");
//
//    printf("Num of files: %i", numOfFiles);
////    MFTparser(mft);

*/
    return 0;

}