#include "cmd.h"

void list(){
    struct buffer_head* bh;
    struct m_inode* inode;
    int size =  current->pwd->i_size / (sizeof(struct dir_entry));
    bh = bread(current->pwd->i_dev, current->pwd->i_zone[0]);
    struct dir_entry * de = (struct dir_entry*) bh->b_data;
    printf("___________________________当前文件目录\n");
    for(int i=0; i < size;i++, de++){
        if(de->inode){
            inode = iget(1,  de->inode);
            if(S_ISDIR(inode->i_mode)){
                printf("drwxrwxrwx %s\n", de->name);
            }else if(S_ISLNK(inode->i_mode)){
                bh = bread(inode->i_dev, inode->i_zone[0]);
                printf("lrwxrwxrwx %s -> %s\n", de->name, bh->b_data);
            }else if(S_ISREG(inode->i_mode)){
                printf("-rwxrwxrwx %s\n", de->name);
            }
        }
    } 
}

void mkdir(){
    char name[50];
    memset(name, 0, sizeof(50));
    // printf("输入目录名称:\n");
    scanf("%s", name);
    sys_mkdir(name, 0777 | S_IFDIR);
}

void touch(){
    char name[50];
    memset(name, 0, sizeof(50));
    // printf("请输入文件名称:\n");
    scanf("%s", name);
    hsc_creat(name, 0777 | S_IFREG);
}

void shell_write(){
    char name[50];
    memset(name, 0, sizeof(50));
    int fd, off;
    scanf("%s", name);
    fd = hsc_open(name, O_RDWR, 0777);
    scanf("%s", name);
    scanf("%d", &off);
    lseek(fd, off, SEEK_SET);
    write(fd, name, 50);
    hsc_close(fd);
}

void shell_read(){
    char name[50];
    memset(name, 0, sizeof(50));
    int fd, off;
    // printf("请输入文件名称:\n");
    scanf("%s", name);
    scanf("%d", &off);
    fd = hsc_open(name, O_RDWR, 0777);
    lseek(fd, off, SEEK_SET);
    read(fd, name, 50);
    printf("_____________文件内容\n");
    printf("%s\n", name); 
}

void ln(){
    char name[50];
    memset(name, 0, sizeof(50));
    int fd;
    // printf("请输入文件名称:\n");
    scanf("%s", name);
    fd = hsc_creat(name, 0777 | S_IFLNK);
    // printf("请输入链接文件名:\n");
    scanf("%s", name);
    write(fd, name, 50);
}

void cd() {
    char name[50];
    memset(name, 0 , 50);
    scanf("%s", name);
    for(int i = 0; i < sizeof(name); i++){
        if(!name[i]){
            name[i] = '/';
            name[i+1] = '.';
            break;
        }
    }
    current->pwd =  get_dir(name, NULL);
}

void rm(){
    char name[50];
    memset(name, 0, 50);
    scanf("%s", name);
    if(!unlink(name)){
        printf("删除失败!\n");
    }
}

void rmdir(){
    char name[50];
    memset(name, 0, 50);
    scanf("%s", name);
    if(!d_rmdir(name)){
        printf("删除失败\n");
    }
}

void help(){
    printf("you fool, there are some suggestions\n");
    printf("mkdir [目录文件]...创建目录\n\n");
    printf("ls ...列出当前目录所有文件\n\n");
    printf("cd [路径]...切换目录\n\n");
    printf("touch [文件]...创建普通文件\n\n");
    printf("ln [链接文件]  [源文件]...创建符号连接文件\n\n");
    printf("rm [文件]...删除文件\n\n");
    printf("rmdir [文件]...删除目录文件\n\n");
    printf("read [文件] [偏移值]...读文件\n\n");
    printf("write [文件] [内容] [偏移值]\n\n");
}