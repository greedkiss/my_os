#include "rwblock.h"

FILE* rw;
//读磁盘
void ll_read_block(struct buffer_head* bh){
    if(bh->b_dev == 1){
        if((rw = fopen(DISK1, "r+")) == NULL){
            printf("读设备号为1的磁盘失败");
        }  
        // printf("读磁盘dev:%d, block:%lu\n", bh->b_dev, bh->b_blocknr);
        int block = 1024 * bh->b_blocknr;
        fseek(rw, block, SEEK_SET);
        fread(bh->b_data, 1024, 1, rw);
        fclose(rw);
    }
    bh->b_uptodate = 1;
}
//写磁盘
void ll_write_block(struct buffer_head* bh){
    if(bh->b_dev == 1){
        // printf("写磁盘dev:%d,block:%lu\n", bh->b_dev, bh->b_blocknr);
        if((rw = fopen(DISK1, "r+")) == NULL){
            printf("写设备号为1的磁盘失败");
        }
        fseek(rw, 1024*bh->b_blocknr, SEEK_SET);
        fwrite(bh->b_data, 1024, 1, rw);
        fclose(rw);
    }
    bh->b_dirt = 0;
}

//读交换分区
void read_swap_page(int nr, char * buffer){
    if((rw = fopen(SWAP_DISK, "r+")) == NULL){
        printf("读交换分区失败\n");
    }
    fseek(rw, 4096*nr, SEEK_SET);
    fread(buffer, 4096, 1, rw);
    fclose(rw);
}

//写交换分区
void write_swap_page(int nr, char * buffer){
    if((rw = fopen(SWAP_DISK, "r+")) == NULL){
        printf("读交换分区失败\n");
    }
    fseek(rw, 4096*nr, SEEK_SET);
    fwrite(buffer, 4096, 1, rw);
    fclose(rw);
}