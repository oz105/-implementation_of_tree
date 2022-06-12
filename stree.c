// Help links 
// https://stackoverflow.com/questions/19233568/how-to-know-if-a-file-directory-is-hidden-in-programming-in-c
// https://man7.org/linux/man-pages/man3/ftw.3.html
// https://stackoverflow.com/questions/50418621/tree-command-in-c-exact-output
// https://github.com/kddnewton/tree/blob/main/tree.c

#define _XOPEN_SOURCE 600 // for nftw

#include <ftw.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Counters */

int dirs = 0;
int files = 0;

int count_no_slash (const char* str);
bool is_hidden(const char *name);
char check_file_type(mode_t m);
int list(const char *name, const struct stat *status, int type, struct FTW *ftwb);


int main(int argc, char *argv[]) {

    int flag = 0;

    if (argc == 1) {
        nftw(".", list, FTW_DEPTH, flag);
    }
    else if (argc == 2) {
        nftw(argv[1], list, FTW_DEPTH, flag);
    }
    else {
        fprintf(stderr, "ERROR ! You should write ./stree \"dir name\" or ./stree\n");
        exit(1);
    }

    printf("\n");
    printf("%d directories, %d files\n", dirs, files);
        
    return 0;
}


int count_no_slash (const char* str){
    int count = 0 ;
    int index = strlen(str) - 1 ;
    while (str[index] != '/' & index >= 0){
        count ++; index -- ;
    }
    return count;
}


bool is_hidden(const char *name) {
    size_t i = 0;
    while (name[i] != '\0'){
        if (name[i] == '/' & name[i + 1] == '.')
            return true;
        i++;
    }
    return false;
}


char check_file_type(mode_t m) {
    char c;

    // using mask for check only the type
    switch (m & S_IFMT){

    case S_IFBLK: 
        c = 'b';
        break;
    case S_IFCHR: // character device
        c = 'c';
        break;
    case S_IFDIR: //dir
        c = 'd';
        break;
    case S_IFIFO:
        c = 'p';
        break;
    case S_IFLNK:
        c = 'l';
        break;
    case S_IFREG: // file
        c = '-';
        break;
    case S_IFSOCK: // socket
        c = 's';
        break;
    default:
        c = '?';
        break;
    }
    return (c);
}


int list(const char *name, const struct stat *status, int type, struct FTW *ftwb) {

    // if file is of unknown type, return
    if (type == FTW_NS){
        return 0;
    } 
     
    // if file is hidden, return
    if (is_hidden(name)){ 
        return 0;  
    }

    if (ftwb->level == 0){
        printf("%s\n", name);
        return 0;
    }

    struct stat l_stat;
    lstat(name, &l_stat);


    // from ftw1.c 

    uid_t user_id = status->st_uid;
    gid_t gid = status->st_gid;
    off_t size = status->st_size;
    size_t path_len = strlen(name);
    char file_name[path_len];
    char permissions[10];


    for (size_t i = 0; i < 10; i++){ permissions[i] = '-';} // set all the arr '-' 

    mode_t check_mode = l_stat.st_mode;


    // the first is the type of the file else '-' 
    // all the other is the permissions of user group and other
    permissions[0] = check_file_type(status->st_mode);
    if (check_mode & S_IRUSR)
        permissions[1] = 'r';
    if (check_mode & S_IWUSR)
        permissions[2] = 'w';
    if (check_mode & S_IXUSR)
        permissions[3] = 'x';
    if (check_mode & S_IRGRP)
        permissions[4] = 'r';
    if (check_mode & S_IWGRP)
        permissions[5] = 'w';
    if (check_mode & S_IXGRP)
        permissions[6] = 'x';
    if (check_mode & S_IROTH)
        permissions[7] = 'r';
    if (check_mode & S_IWOTH)
        permissions[8] = 'w';
    if (check_mode & S_IXOTH)
        permissions[9] = 'x';



    // from ftw1.c 
    // get user name
    struct passwd *pass = getpwuid(user_id);
    char *user_name = pass->pw_name;

    // from ftw1.c 
    // get group name
    struct group *grp = getgrgid(gid);
    char *group_name = grp->gr_name;


    // parse file name

    if(path_len == 1){
         strcpy(file_name, name);
    }

    else{
        int len = count_no_slash(name);
        strcpy(file_name, name + (path_len - len));
    }


    if (ftwb->level != 0) {
        for (int i = 1; i < ftwb->level; i++) {
            printf("│   ");
        }
        printf("├── ");
    }


    if (type == FTW_F)
            files++;
    if (type == FTW_D && strcmp(".", name) != 0)
            dirs++;
   
    printf("[%s %s\t %s\t\t%ld]  %s\n", permissions, user_name, group_name ,size, file_name);

    return 0;

}


