#include "super.h"

struct super_block super_block[NR_SUPER];

struct file file_table[NR_FILE];


//从超级块数组中取得超级块
struct super_block* get_super(int dev){
    struct super_block* s;
    s = 0 + super_block;
    while(s < super_block + NR_SUPER){
        if(dev == s->s_dev)
            return s;
        else
            s++;
    }
    return NULL;
}
//从磁盘读超级块
struct super_block* read_super(int dev){
    struct super_block* s;
    struct buffer_head* bh;
    if(s = get_super(dev)){
        return  s;
    }
    for(s = 0 + super_block; ;s++){
        if(s > super_block + NR_SUPER)
            return NULL;
        if(!s->s_dev)
            break;
    }
    s->s_dev = dev;
    s->s_isup = NULL;
    s->s_imount = NULL;
    s->s_time = 0;
    s->s_dirt = 0;
    if(!(bh = bread(dev, 1))){
        printf("读超级块出错\n");
        return NULL;
    }
    //强转，bh->b_data无格式
    *((struct d_super_block*) s) = *((struct d_super_block *) bh->b_data);
    brelse(bh);
    printf("super: %d\n", s->s_magic);
    if(s->s_magic != SUPER_MAGIC){
        printf("不支持此类文件系统\n");
        return NULL;
    }
    for(int i = 0 ; i<I_MAP_NR; i++){
        s->s_imap[i] = NULL;
    }
    for(int i = 0; i<Z_MAP_NR; i++){
        s->s_zmap[i] = NULL;
    }
    int block = 2;
    for(int i = 0; i < s->s_imap_blocks ; i++){
        if(s->s_imap[i] = bread(dev, block)){
            block++;
        }else{
            printf("读i节点位图出错\n");
        }
    }
    for(int i = 0 ; i < s->s_zmap_blocks ; i++){
        if(s->s_zmap[i] = bread(dev, block)){
            block++;
        }else{
            printf("读数据块位图出错\n");
        }
    }
    BIT_CLEAR(s->s_imap[0]->b_data[0], 7);
    BIT_CLEAR(s->s_zmap[0]->b_data[0], 7);
    // find_first_zero_in_char(s->s_imap[0]->b_data[0]);
    return s;
}

//安装根文件系统
void mount_root(void){
    struct super_block* p;
    struct m_inode* mi;
    int i;
    for(i = 0; i < NR_FILE ;i++){
        file_table[i].f_count = 0;
    }
    for(p = 0 + super_block; p < super_block + NR_SUPER; p++){
        p->s_dev = 0;
        p->s_lock = 0;
    }
    if(!(p = read_super(ROOT_DEV)))
        printf("读根文件系统超级块失败\n");
    if(!(mi = iget(ROOT_DEV, ROOT_INO)))
        printf("读根i节点失败\n");
    mi->i_count += 3;
    p->s_isup = p->s_imount = mi;
    mi->i_mode |= S_IFDIR;
    mi->i_mode |= MAY_EXEC;
    current->pwd = mi;
    current->root = mi;
    mi = current->root;
}