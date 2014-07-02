#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define NAME_LEN 100
#define MODE_LEN 8
#define UID_LEN 8
#define GID_LEN 8
#define SIZE_LEN 12
#define MTIME_LEN 12
#define CHKSUM_LEN 8
#define TYPEFLAG_LEN 1
#define LINKNAME_LEN 100
#define MAGIC_LEN 6
#define VERSION_LEN 2
#define UNAME_LEN 32
#define GNAME_LEN 32
#define DEVMAJOR_LEN 8
#define DEVMINOR_LEN 8
#define PREFIX_LEN 155
#define BUFSIZE 4096

/**
 *  Program
 *
 *  @author Steve Choo
 * */

/*
 * Headers are 512 bytes...read them in 512 byte chunks,
 * then parse accordingly
 */
struct tar_header {
   char name[NAME_LEN];
   char mode[MODE_LEN];
   char uid[UID_LEN];
   char gid[GID_LEN];
   char size[SIZE_LEN];
   char mtime[MTIME_LEN];
   unsigned char chksum[CHKSUM_LEN];
   char typeflag;
   char magic[MAGIC_LEN];
   char version[VERSION_LEN];
   char uname[UNAME_LEN];
   char gname[GNAME_LEN];
   char devmajor[DEVMAJOR_LEN];
   char devminor[DEVMINOR_LEN];
   char prefix[PREFIX_LEN];
};
typedef struct tar_header tar_header;

int opt_verbose = 0;

int set_flags(int *createArch, int *printTOC, int *extractArch,
      int *verbosity, int *archFileName, int *strict, char *options); 
void write_dir_header(struct stat dirStat);
void create_archive(int argc, char *argv[]);
int main(int argc, char *argv[]) {
   /* flags */
   /* required_COUNT will be set if one of the required options is
    * already found...used at a check to ensure other flags do not
    * get set
    */
   int createArch_FLAG = 0, printTOC_FLAG = 0, extractArch_FLAG = 0, 
       verbosity_FLAG = 0, archFileName_FLAG = 0, strict_FLAG = 0,
       required_COUNT = 0;
   char *options;
   char *cwd;
   struct stat fileStat;

   cwd = get_current_dir_name();
   
   if(argc == 1) {
      if(cwd == NULL) {
         perror("get_current_dir_name");
         exit(1);
      }

      printf("%s: ", cwd);
      printf("Must specify an option\n");
      exit(1);
   }

   /*
    * argv[1] should be the options...
    */
   options = argv[1];

   required_COUNT = set_flags(&createArch_FLAG, &printTOC_FLAG, 
         &extractArch_FLAG, &verbosity_FLAG, &archFileName_FLAG,
         &strict_FLAG, options);

   if(required_COUNT != 1) {
      if(cwd == NULL) {
         perror("get_current_dir_name");
         exit(1);
      }

      printf("%s: ", cwd);
      printf("Only one of ctx is required\n");
      exit(1);
   }

  
   if(createArch_FLAG) {
      create_archive(argc, argv);
   }

   return 0;
}

void create_archive(int argc, char *argv[]) {
   int fd_tarFile, fd_currFile,  i;
   char *tarName = argv[2];
   unsigned char buffer[BUFSIZE];
   struct stat currFileStat;
   tar_header currTarHeader;
   size_t MAX_NAME_SIZE = 100;

   fd_tarFile = open(tarName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR |
         S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

   if(fd_tarFile == -1) {
      perror("TarFile");
      exit(2);
   }

   for(i = 3; i < argc; i++) {
      fd_currFile = open(argv[i], O_RDONLY);

      if(fd_currFile == -1) {
         perror(argv[i]);
         continue;
      }

      if(-1 == fstat(fd_currFile, &currFileStat)) {
         perror(argv[i]);
         continue;
      }

      if(S_ISDIR(currFileStat.st_mode)) {
         write_dir_header(currFileStat);
      } else if(S_ISREG(currFileStat.st_mode)) {
         /*write_file_header();*/
      } else if(S_ISLNK(currFileStat.st_mode)) {
         /*write_sym_header();*/
      } else {
         continue;
      }
   }
}

void write_dir_header(struct stat dirStat) {
   /*
    * Pseudocode:
    *
    * opendir();
    * loop all files in dir {
    *    if dir {
    *       recurse;
    *    } else {
    *       file found, call write_file_header function
    *    }
    */

}

int read_header(int fd, unsigned char *buf) {
   return 0;
}

int set_flags(int *createArch, int *printTOC, int *extractArch,
      int *verbosity, int *archFileName, int *strict, char *options) { 
   int required = 0;


   if(strchr(options, 'c')) {
      *createArch = 1;
      required++;
   }

   if(strchr(options, 't')) {
      *printTOC = 1;
      required++;
   }

   if(strchr(options, 'x')) {
      *extractArch = 1;
      required++;
   }

   if(strchr(options, 'v')) {
      *verbosity = 1;
   }

   if(strchr(options, 'f')) {
      *archFileName = 1;
   }

   if(strchr(options, 'S')) {
      *strict = 1;
   }

   return required;
}

