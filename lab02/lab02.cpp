#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;

struct PAKO_HEADER{
    uint32_t magic;
    int32_t off_str;
    int32_t off_dat;
    uint32_t n_files;
};

struct FILE_E{
    int32_t off_filename;
    uint32_t filesize; // Big Endian
    int32_t off_content;
    uint64_t checkSum; // Big Endian
    string filename;
};

    
void ReadFileInfo(FILE_E* file, int& fd){
    unsigned char temp[8];
    read(fd, &file->off_filename, 4);
    read(fd, temp, 4);
    file->filesize = ((uint32_t)temp[0]<<24 | (uint32_t)temp[1]<<16 | (uint32_t)temp[2]<<8 | (uint32_t)temp[3]);
    read(fd, &file->off_content, 4);
    read(fd, temp, 8);
    file->checkSum = ((uint64_t)temp[0]<<56 | (uint64_t)temp[1]<<48 | (uint64_t)temp[2]<<40 | (uint64_t)temp[3]<<32 | (uint64_t)temp[4]<<24 | (uint64_t)temp[5]<<16 | (uint64_t)temp[6]<<8 | (uint64_t)temp[7]);
    return ;
}


bool checkSum(PAKO_HEADER* pako, FILE_E* file, int& fd){
    lseek(fd, pako->off_dat+file->off_content, SEEK_SET);
    int rem = file->filesize%8;
    uint64_t buffer = 0;
    for(int i = 0; i+7 < file->filesize; i+=8){
        uint64_t temp = 0;
        read(fd, &temp, 8);
        buffer = buffer ^ temp;
    }
    if(rem != 0){
        uint64_t temp = 0;
        read(fd, &temp, rem);
        buffer = buffer ^ temp;
    }
    return (buffer == file->checkSum);
}


int main(int argc, char** argv){
    PAKO_HEADER pako;
    int fd;
    if(access(argv[1], F_OK) == 0)
        fd = open(argv[1], O_RDONLY);
    read(fd, &pako, sizeof(PAKO_HEADER));
    cout << pako.magic << " " << pako.off_str << " " << pako.off_dat << " " << pako.n_files << endl;
    FILE_E  *fileArray = new FILE_E [pako.n_files];
    for(int i = 0; i < pako.n_files; i++){
        ReadFileInfo(&fileArray[i], fd);
        cout << (i+1) << ": ";
        cout << fileArray[i].off_filename << " " << fileArray[i].filesize << " ";
        cout << fileArray[i].off_content << " " << fileArray[i].checkSum << endl;
    }

    lseek(fd, pako.off_str, SEEK_SET);
    for(int i = 0; i < pako.n_files; i++){
        char buffer;
        while( read(fd, &buffer, 1) && buffer != 0)
            fileArray[i].filename += buffer;
        cout << (i+1) << ": " << fileArray[i].filename << endl;
    } 
    cout << endl;

    int fd2;
    for(int i = 0; i < pako.n_files; i++){
        bool temp = checkSum(&pako, &fileArray[i], fd);
        cout << (i+1) << "'s checkSum: " << temp << endl;
        if(temp){
            lseek(fd, pako.off_dat+fileArray[i].off_content, SEEK_SET);
            cout << "Create file: " << fileArray[i].filename << endl;
            string path = argv[2] + ("/" + fileArray[i].filename);
            cout << path << endl;
            if(access(path.c_str(), F_OK) == 0)
                fd2 = open(path.c_str(), O_WRONLY);
            else
                fd2 = creat(path.c_str(), 0644);
            char buffer[100];
            int idx = 0;
            while(idx+100 < fileArray[i].filesize){
                int bytes_read = read(fd, buffer, 100);
                write(fd2, buffer, bytes_read);
                idx += bytes_read;
                cout << idx << endl;
            }
            int bytes_read = read(fd, buffer, fileArray[i].filesize-idx);
            write(fd2, buffer, bytes_read);
        }
    }
    close(fd);
    close(fd2);
    return 0;
}