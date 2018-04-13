#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#include <math.h>


bool DEBUG = false;
FILE *diskImage;
int startOfPartition;
int startOfMft;
int clusterSize = 4096;
const int SIZE_OF_CHILDREN_ARRAY = 1024;


struct Attribute {
    int attributeOffset;
    unsigned char attributeType;
    int nextAttributeOffset;
    unsigned char attributeContent[50];    //TODO: 50 - only for parent location and name, change later!!!
    bool redident;

};

struct outerAttribute {
    int startingSectorNumber;
    int size;
    unsigned char attributeType; //0xa0 - index allocation, 0x80 - data
};

struct File {

    unsigned char contents[1024];
    int sizeInBytes;
    int numberInMFT;
    struct Attribute attributes[10];
    int parentNumberInMFT;
    char fileName[50];
    int nameLength;
    char fullPath[100];
    int childrenNumbersInMFT[SIZE_OF_CHILDREN_ARRAY];
    int numOfChildren;
    int startingSector;
    int endingSector;
    struct outerAttribute outerAttributes[10];
    int numOfOuterAttributes;

};

struct Attribute AttributeParser(unsigned char *file, int attributeOffset);

int MBRParser(unsigned char *MBR);

struct File getChildrenForDirectory(struct File file, struct Attribute attribute);

struct File indexAllocationParser(struct File file, struct Attribute attribute);

struct File dataAttributeParser(struct File file, struct Attribute attribute);

struct File FileParser(struct File file);

struct File printParentFile(int index, struct File arrayOfFiles[100], struct File originalFile);

int main(int argc, char *argv[]);

struct Attribute AttributeParser(unsigned char *file, int attributeOffset) {

    struct Attribute newAttribute;
    newAttribute.redident = true;
    if ((int) file[attributeOffset + 4] == 0x00 || file[attributeOffset] == 0xFF) {
        printf("Аттрибуты закончились\n");
        newAttribute.nextAttributeOffset = -1;
        return newAttribute;
    }
    int headerSize = 24;
    printf("Смещение аттрибута: %i\n", attributeOffset);
    newAttribute.attributeOffset = attributeOffset;

    int attributeSize = file[attributeOffset + 4] + file[attributeOffset + 5] * 256;
    printf("Размер аттрибута: %i\n", attributeSize);

    printf("Тип аттрибута: ");

    newAttribute.attributeType = file[attributeOffset];

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
            for (int i = 0; i < 3; i++) {
                printf("%.2x ", file[attributeOffset + headerSize + i]);
                newAttribute.attributeContent[i] = file[attributeOffset + headerSize + i];
            }
            printf("\n");
            printf("Длина имени: %i символов\n", (int) file[attributeOffset + headerSize + 64]);
            printf("Имя файла: ");

            newAttribute.attributeContent[4] = file[attributeOffset + headerSize + 64];  //filename length

            for (int i = 0; i < (int) file[attributeOffset + headerSize + 64] * 2; i++) {
                printf("%c", file[attributeOffset + headerSize + 66 + i]);
                newAttribute.attributeContent[5 + i] = file[attributeOffset + headerSize + 66 + i];

            }
//
//            for (int i=0; i<20; i++){
//                newAttribute.attributeContent[3 + i] = file[attributeOffset + headerSize + 66 + i];
//            }
            break;
        case 0x50:
            printf("$SECURITY_DESCRIPTOR\n");
            break;

        case 0x80:
            printf("$DATA\n");
            for (int i = 0; i < attributeSize - headerSize - 4; i++) {
                printf("%c", file[attributeOffset + headerSize + i]);

            }
            if (file[attributeOffset + 8] == 0x01) {
                printf("\n?????????????????????? NON RESIDENT!!!!!!!!\n");
                newAttribute.redident = false;
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

    newAttribute.nextAttributeOffset = attributeOffset + attributeSize;
    return newAttribute;

}


struct File FileParser(struct File file) {
    int i;
    int nextAttributeOffset;
    int fileSize = 1024;
    int actualSize = 0;
    i = 0;
    int parentNumber = -1;


    file.parentNumberInMFT = parentNumber; //default is root directory
    file.nameLength = 0; //default name length is 0

    //file.parentNumberInMFT=-1;
    actualSize = file.contents[25] * 256 + file.contents[24];
    file.sizeInBytes = actualSize;

    if (actualSize > 1024) printf("\n?????????????????\n");
//    while (i < actualSize-4) {

    if (DEBUG) {

        while (i < fileSize) {
            printf("[%i]0x%.2x ", i, file.contents[i]);
//            printf("0x%.2x ", file.contents[i]);

            if ((i + 1) % 4 == 0) printf("  ");
            if (i != 0 && (i + 1) % 16 == 0) printf("\n");
            i++;

        }
    }


    for (int i = 0; i < SIZE_OF_CHILDREN_ARRAY; i++) {
        file.childrenNumbersInMFT[i] = 0;
    }

    file.numOfChildren = 0;
    file.numOfOuterAttributes = 0;

    if (file.contents[8] == 0x01)
        printf("NON RESIDENT!!!!!!!!!!!!!!!!!!!!!!!!!****************************************\n");

    switch (file.contents[22]) {
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
    printf("Реальный размер файловой записи: 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", file.contents[24], file.contents[25],
           file.contents[26],
           file.contents[27]);
    actualSize = file.contents[25] * 256 + file.contents[24];
    printf("Размер в байтах: %i\n", actualSize);
    printf("Выделенный размер файловой записи: %.2x %.2x %.2x %.2x\n", file.contents[28], file.contents[29],
           file.contents[30],
           file.contents[31]);


    //--------------------------------------------- update sequence array & fix up

    int updateSequenceOffset = file.contents[4] + file.contents[5] * 256;
    printf("Fix Up numbers are: %.2x %.2x", file.contents[updateSequenceOffset],
           file.contents[updateSequenceOffset + 1]);
    printf("Old ending of first sector = %.2x %.2x\n", file.contents[510], file.contents[511]);
    file.contents[510] = file.contents[updateSequenceOffset + 2];
    file.contents[511] = file.contents[updateSequenceOffset + 3];
    printf("New ending of first sector = %.2x %.2x\n", file.contents[510], file.contents[511]);
    file.contents[1022] = file.contents[updateSequenceOffset + 2];
    file.contents[1023] = file.contents[updateSequenceOffset + 3];

    //-------------------------------------------------

    //printf("Update sequence number: %.2x %.2x\n", file[4], file[5]);
    //printf("Update sequence number & Array: %.2x %.2x\n", file.contents[6], file.contents[7]);
    //printf("$Log file sequence number: ");
    //for (i = 0; i < 8; i++) printf("%.2x ", file[8 + i]);
    //printf("\nSequence number: %.2x %.2x\n", file[16], file[17]);
    //printf("Счетчик жестких ссылок: %.2x %.2x\n", file[18], file[19]);
//    printf("Смещение перовго атрибута: %.2x %.2x\n", file[20], file[21]);

    printf("Ссылка на базовую файловую запись или ноль, если данная файловая запись базовая: ");
    for (i = 0; i < 8; i++) printf("%.2x ", file.contents[32 + i]);
    printf("\n---Атрибуты---\n");
    int numOfAttributes = 0;
    struct Attribute newAttribute = AttributeParser(file.contents, (int) file.contents[20]);
    file.attributes[0] = newAttribute;
    numOfAttributes++;
    nextAttributeOffset = newAttribute.nextAttributeOffset;

    while (nextAttributeOffset != -1) {
        newAttribute = AttributeParser(file.contents, nextAttributeOffset);
        nextAttributeOffset = newAttribute.nextAttributeOffset;
        file.attributes[numOfAttributes] = newAttribute;
        numOfAttributes++;

        if (newAttribute.attributeType == 0x30) {     //Adding parent to the file info
            parentNumber = newAttribute.attributeContent[0] + newAttribute.attributeContent[1] * 256;
            file.parentNumberInMFT = parentNumber;
            file.nameLength = ((int) newAttribute.attributeContent[4]) * 2;

            //Через символ идет пустой символ, искажает вид файла, поэтому убираем
            for (int j = 0; j < file.nameLength; j += 2) {
                file.fileName[j / 2] = newAttribute.attributeContent[5 + j];

            }

            //заполняем остаток имени пустыми символами

            for (int j = file.nameLength / 2; j < file.nameLength; j++) {
                file.fileName[j] = '\0';
            }

        } else if (newAttribute.attributeType == 0x90) {

            file = getChildrenForDirectory(file, newAttribute);


        } else if (newAttribute.attributeType == 0xa0) {
            file = indexAllocationParser(file, newAttribute);

        } else if (newAttribute.attributeType == 0x80) {
            file = dataAttributeParser(file, newAttribute);
        }


//        printf("\nNext attribute starts at: %i\n", nextAttributeOffset);
    }

//    if (indexRootFound && !indexAllocationFound){
//        file = getChildrenForDirectory(file, newAttribute);
//
//    }


    return file;
}


struct File getChildrenForDirectory(struct File file, struct Attribute attribute) {


    //int residencyFlagOffset = 9;
    int nameLengthOffset = 9;
    int nameLength = file.contents[attribute.attributeOffset + nameLengthOffset];
//    printf("attribute offset: %i, name offset: %i", attribute.attributeOffset,
//           attribute.attributeOffset + nameLengthOffset);
    int indexEntryOffset = 32;
    int indexEntrySize;

    int attributeBodyOffset = 24 + nameLength * 2;

    int attributeBody = attribute.attributeOffset + attributeBodyOffset;

    int fileReference = attributeBody + indexEntryOffset;

//    printf("\n!!!!!!!!!!!!!!!!!!!!!!!First file reference: %i\n", file.contents[fileReference]);
//    indexEntrySize = file.contents[fileReference + 8];
    while (file.contents[fileReference] != 0) {
        file.childrenNumbersInMFT[file.numOfChildren] =
                file.contents[fileReference] + file.contents[fileReference + 1] * 256;
        printf("\n!!!!!!!!!!!!!!!!!!!!!!! File reference: %i\n",
               file.contents[fileReference] + file.contents[fileReference + 1] * 256);
        indexEntrySize = file.contents[fileReference + 8];
        fileReference += indexEntrySize;
        file.numOfChildren++;
    }

    return file;

}


struct File dataAttributeParser(struct File file, struct Attribute attribute) {

    int nameLength = file.contents[attribute.attributeOffset + 9];
    int dataRunsOffset = attribute.attributeOffset + nameLength * 2 + 64;
    if (DEBUG) printf("\nData Runs start at: %i\n", dataRunsOffset);

    unsigned char dataRunHeader;
    int lengthOfOffset;
    int offsetAsInteger;

    while (1) {
        dataRunHeader = file.contents[dataRunsOffset];

        if (dataRunHeader == 0x00) {
            break;
        }
        offsetAsInteger = 0;

        lengthOfOffset = (int) dataRunHeader / 16;
        if (DEBUG) printf("\nLength of offset: %i\n", lengthOfOffset);
        unsigned char offset[lengthOfOffset];

        dataRunsOffset += 2;
        for (int i = 0; i < lengthOfOffset; i++) {
            offset[i] = file.contents[dataRunsOffset];
            printf("%.2x\n", offset[i]);
            dataRunsOffset++;
        }

        printf("\n");

        for (int i = lengthOfOffset - 1; i >= 0; i--) {
            offsetAsInteger += offset[i] * (int) pow(256, i);
        }
    }

    return file;
}


struct File indexAllocationParser(struct File file,
                                  struct Attribute attribute) {


    int nameLength = file.contents[attribute.attributeOffset + 9];
    int dataRunsOffset = attribute.attributeOffset + nameLength * 2 + 64;
    if (DEBUG) printf("\nData Runs start at: %i\n", dataRunsOffset);
    unsigned char dataRunHeader = 0x90;
    int lengthOfOffset;
    int offsetAsInteger;

    int previousOffset = 0;

    while (1) {
        dataRunHeader = file.contents[dataRunsOffset];

        if ((int) dataRunHeader / 16 == 0) {
            break;
        }
        offsetAsInteger = 0;

        lengthOfOffset = (int) dataRunHeader / 16;
        printf("dataRunHeader is %.2x, which is is %i and %i\n", dataRunHeader, lengthOfOffset, dataRunHeader % 8);
        int nextOffset = lengthOfOffset + dataRunHeader % 8;
        if (DEBUG) printf("\nLength of offset: %i\n", lengthOfOffset);
        unsigned char offset[lengthOfOffset];

        dataRunsOffset = dataRunsOffset + 1 + dataRunHeader % 6;
        //printf("Original offset = %i, offset with second num = %i\n", dataRunsOffset, dataRunsOffset-1 + dataRunHeader%8);
        for (int i = 0; i < lengthOfOffset; i++) {
            offset[i] = file.contents[dataRunsOffset];
            printf("%.2x\n", offset[i]);
            dataRunsOffset++;
        }

        printf("\n");

        for (int i = lengthOfOffset - 1; i >= 0; i--) {
            offsetAsInteger += offset[i] * (int) pow(256, i);
        }

        offsetAsInteger += previousOffset; // data runs взаимосвязаны и смещение указано относительно предыдущего
        printf("Offset is =  %i\n", offsetAsInteger * clusterSize + startOfPartition + 28);

        unsigned char indxEntrySize[4];
        int bufferSize = 0;

        unsigned char sizeInWordsOfUpdateSequence[2];
        fseek(diskImage, offsetAsInteger * clusterSize + startOfPartition + 6, SEEK_SET);
        fread(sizeInWordsOfUpdateSequence, 1, 2, diskImage);
        printf("???? Size in words of the update sequence: %.2x %.2x\n",
               sizeInWordsOfUpdateSequence[0], sizeInWordsOfUpdateSequence[1]);

        fseek(diskImage, offsetAsInteger * clusterSize + startOfPartition + 40, SEEK_SET); // update sequence number
        unsigned char updateSequence[2];
        fread(updateSequence, 1, 2, diskImage);
        printf("????Update sequence is: %.2x %.2x\n", updateSequence[0], updateSequence[1]);
        printf("End of sector is located at: %i\n", offsetAsInteger * clusterSize + startOfPartition + 511);


        fseek(diskImage, offsetAsInteger * clusterSize + startOfPartition + 42, SEEK_SET); // update sequence array
        unsigned char updateSequenceArray[8];
        fread(updateSequenceArray, 1, 8, diskImage);
        for (int k = 0; k < 8; k++) {
            printf("%.2x ", updateSequenceArray[k]);
        }
        printf("\n");
        printf("!!!!!!!!!!!!");
        fseek(diskImage, offsetAsInteger * clusterSize + startOfPartition + 28, SEEK_SET);
        fread(indxEntrySize, 1, 4, diskImage); // Размер INDX Entry

        printf("1");
        for (int i = 3; i >= 0; i--) {
            bufferSize += indxEntrySize[i] * (int) pow(256, i);
        }
        unsigned char buffer[bufferSize];
        //printf("Index entry size is: %i\n", bufferSize);

        fseek(diskImage, offsetAsInteger * clusterSize + startOfPartition, SEEK_SET);
        printf("2");

        file.outerAttributes[file.numOfOuterAttributes].startingSectorNumber =
                (offsetAsInteger * clusterSize + startOfPartition) / 512;
        file.outerAttributes[file.numOfOuterAttributes].size = bufferSize;
        file.outerAttributes[file.numOfOuterAttributes].attributeType = 0x90;
        file.numOfOuterAttributes++;

        printf("3");

        fread(buffer, 1, bufferSize, diskImage);
        printf("???? Buffer size is: %i\n", bufferSize);

        if (bufferSize == 0){
            break;                          //TODO: CHECK IF THIS IS CORRECT -- WHY IT HAPPENS
        }
        if (false){
            LABEL:                          //TODO: THIS TOO
            break;
        }
        int j = 511;
        int indexOfUSA = 0;
        unsigned char valuesOfUSA[2];


        //--------------------------------------------- fix up
        bool badRecord = false;
        while (j < bufferSize) {                    //DOING FIX UP!!!!!!!!
            printf("Before fixup: j = %i, end of sector is: %.2x %.2x\n", j, buffer[j - 1], buffer[j]);

            if (buffer[j - 1] != updateSequence[0] || buffer[j] != updateSequence[1]) {
//                printf("ERROR!!!!!!!!!!!!!!!!!!!!!!!\n");
                badRecord = true;
                goto FAULTY_RECORD;

            }
            fseek(diskImage, offsetAsInteger * clusterSize + startOfPartition + 42 + indexOfUSA,
                  SEEK_SET); //переходим к конкретному элементу USA
            fread(valuesOfUSA, 1, 2, diskImage); //берем два нужных значения
            buffer[j - 1] = valuesOfUSA[0];
            buffer[j] = valuesOfUSA[1];
            indexOfUSA += 2;
            printf("After fixup: j = %i, end of sector is: %.2x %.2x\n", j, buffer[j - 1], buffer[j]);

            j += 512;
        }
        //---------------------------------------------

        int i = 0;
        while (i < bufferSize) {
            printf("[%i]0x%.2x ", i, buffer[i]);
//            printf("0x%.2x ", file.contents[i]);

            if ((i + 1) % 4 == 0) printf("  ");
            if (i != 0 && (i + 1) % 16 == 0) printf("\n");
            i++;

        }

        int firstIndexEntryOffset = 0;

        for (i = 3; i >= 0; i--) {
            firstIndexEntryOffset += buffer[24 + i] * (int) pow(256, i);
        }

        firstIndexEntryOffset += 24;
        printf("First index entry offset is %i\n",
               offsetAsInteger * clusterSize + startOfPartition + firstIndexEntryOffset);
        int fileReference = firstIndexEntryOffset;

        int indexEntrySize = 0;

        while (fileReference < bufferSize) {

            printf("!!!!!!!!! File reference: %i\n", buffer[fileReference] + buffer[fileReference + 1] * 256);
            file.childrenNumbersInMFT[file.numOfChildren] = buffer[fileReference] + buffer[fileReference + 1] * 256;
            indexEntrySize = buffer[fileReference + 8];
            indexEntrySize += buffer[fileReference + 9] * 256;
            //printf("File reference offset is: %i\n", offsetAsInteger * clusterSize + startOfPartition + fileReference);
            printf("Index entry size is: %i\n", indexEntrySize);

            fileReference += indexEntrySize;

            file.numOfChildren++;
            printf("Current num of children = %i\n", file.numOfChildren);
        }
        previousOffset = offsetAsInteger;

        FAULTY_RECORD:
        if (badRecord) {
            printf("!!!!!!!BAD RECORD\n");
            goto LABEL;
        }
    }

    return file;

}


int MBRParser(unsigned char *MBR) {
    int i;
    int startOfSector;
    printf("Первый раздел:\n");

    for (i = 446; i < 462; i++) {
        printf("%.2X ", MBR[i]);
    }

    printf("\nBoot indicator: %.2X  (0 - not active)\n", MBR[446]);
    printf("Starting CHS (Cylinder - Head - Sector) values: %.2X %.2X %.2X\n", MBR[449], MBR[448], MBR[447]);
    printf("Ending CHS values: %.2X %.2X %.2X\n", MBR[453], MBR[452], MBR[451]);
    startOfSector = MBR[456] * 4096 + MBR[455] * 256 + MBR[454];
    startOfSector *= 512;
    printf("Starting sector: %.2X%.2X%.2X%.2X -- position:%i \n", MBR[457], MBR[456], MBR[455], MBR[454],
           startOfSector);
    printf("Size of partition: %.2X %.2X %.2X %.2X \n", MBR[458],
           MBR[459], MBR[460], MBR[461]);


    printf("\nВторой раздел:\n");
    for (i = 462; i < 478; i++) {
        printf("%.2X ", MBR[i]);
    }

    printf("\nТретий раздел:\n");
    for (i = 478; i < 494; i++) {
        printf("%.2X ", MBR[i]);
    }

    printf("\nЧетвертый раздел:\n");
    for (i = 494; i < 510; i++) {
        printf("%.2X ", MBR[i]);
    }
    printf("\n");

    return startOfSector;

}

struct File printParentFile(int index, struct File arrayOfFiles[100], struct File originalFile) {

    struct File currentFile = arrayOfFiles[index];

//    if (strcmp(originalFile.fileName, "") == 0){
//
//        return originalFile;
//    }

    if (currentFile.parentNumberInMFT == -1) {
        strcat(originalFile.fullPath, "No directory");
        return originalFile;
    }


    char *str1 = originalFile.fullPath;
    char *str2 = currentFile.fileName;


    if (strcmp(originalFile.fullPath, "") != 0) {        // После самого файла не печатает слеш
        strcat(str2, "/");
    }

    strcat(str2, str1);

    strcpy(originalFile.fullPath, str2);

    if (currentFile.fileName[0] != '.') {
        return printParentFile(currentFile.parentNumberInMFT, arrayOfFiles, originalFile);
    }

    return originalFile;

}

int main(int argc, char *argv[]) {


    //FILE *out;
    int i;
    //int j, byteCount;
    unsigned char MBR[512];
    unsigned char buffer[50];
    // unsigned char file[1024];

    struct File arrayOfFiles[100];

    diskImage = fopen(argv[1], "rb");

    printf("Opened file \n");

    fread(MBR, 1, 512, diskImage);
    printf("Read $MBR\n");
    startOfPartition = MBRParser(MBR);
    printf("Start of partition: %i\n", startOfPartition);


    fseek(diskImage, startOfPartition, SEEK_SET);
    fread(buffer, 1, 50, diskImage);

    printf("\n");

    int sectorSize = 512;

    clusterSize = buffer[13] * sectorSize;
    printf("Cluster size: %i\n", clusterSize);

    startOfMft = buffer[48] * clusterSize + startOfPartition;

    printf("Start of MFT: %i\n", startOfMft);
    fseek(diskImage, startOfMft, SEEK_SET);
    int fileCount = 0;
    for (i = 0; i < 100; i++) {    //100 файлов типо считывает???
        fseek(diskImage, startOfMft + 1024 * fileCount, SEEK_SET);
        struct File newFile;
        newFile.startingSector = (startOfMft + 1024 * fileCount) / 512;
        newFile.endingSector = newFile.startingSector + 1;
        newFile.numberInMFT = fileCount; //Record number in MFT

        fread(newFile.contents, 1, 1024, diskImage);

        if ((newFile.contents[25] * 256 + newFile.contents[24]) == 0) {
            printf("End of files\n");
            break;
        }
        printf("\nФайл номер: %i\n", fileCount);
        newFile = FileParser(newFile);
        arrayOfFiles[i] = newFile;
        fileCount++;


    }


    for (int i = 0; i < fileCount; i++) {
        printf("\n %i File Path:\n", i);
        arrayOfFiles[i] = printParentFile(i, arrayOfFiles, arrayOfFiles[i]);
        printf("%s \n", arrayOfFiles[i].fullPath);
    }

//    currentFile = printParentFile(13, arrayOfFiles, arrayOfFiles[13]);
//    printf("%s \n", currentFile.fullPath);




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

