#include "init.h"

FILE* disk;

extern void initIntroduce(char* DISK){
    if((disk = fopen(DISK, "r+"))== NULL){
        fclose(disk);
        printf("引导块初始化失败\n");
    }else{
        char buffer[1024] = {'0'};
        fseek(disk, INTRO_START, SEEK_SET);
        fwrite(buffer, strlen(buffer), 1, disk);
        fclose(disk);
        printf("引导块初始化\n");
    }
}

extern void initSuper(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        //fclose(disk);
        printf("超级块初始化失败\n");
    }else{
        struct d_super_block super;
        super.s_ninodes = 16*1024;
        super.s_nzones = 16*1024;
        super.s_imap_blocks = 2;
        super.s_zmap_blocks = 2;
        super.s_firstdatazone = 505;
        super.s_log_zone_size = 50;
        super.s_max_size = 1024*7 + 1024*1024 + 1024*1024*1024;
        super.s_magic = 0x137f;
        fseek(disk, SUPER_START, SEEK_SET);
        fwrite(&super, sizeof(struct d_super_block), 1, disk);
        fclose(disk);
        printf("超级块初始化\n");
    }
}

void initimap(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        fclose(disk);
        printf("初始化i节点位图失败\n");
    }else{
        char buffer[1024];
        memset(buffer, 255, sizeof(buffer));
        fseek(disk, INODE_MAP, SEEK_SET);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fclose(disk);
        printf("i节点位图初始化\n");
    }
}

void initzmap(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        fclose(disk);
        printf("初始化数据块位图失败\n");
    }else{
        char buffer[1024];
        memset(buffer, 255, sizeof(buffer));
        fseek(disk, BLOCK_MAP, SEEK_SET);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fclose(disk);
        printf("数据块位图初始完成\n");
    }
}

void inititable(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        fclose(disk);
        printf("i节点表初始失败\n");
    }else{
        struct d_inode d_inode;
        d_inode.i_mode = 0;
        d_inode.i_uid = 0;
        d_inode.i_size =0;
        d_inode.i_gid = 0;
        d_inode.i_mtime = 0;
        d_inode.i_nlinks = 0;
        d_inode.i_zone[9] = 0;
        for(int j = 0; j < 500 ; j++){
            fseek(disk, INODE_AREA + (j * BLOCK_SIZE), SEEK_SET);
            for(int i =0 ;i < INODE_PER_BLOCK; i++)
                fwrite(&d_inode, sizeof(struct d_inode), 1, disk);
        }
        fclose(disk);
        printf("i节点初始化成功\n");
    }
}

//磁盘初始化
void initFileSys(void ){
    initIntroduce(DISK1);
    initSuper(DISK1);
    initimap(DISK1);
    initzmap(DISK1);
    inititable(DISK1);
}