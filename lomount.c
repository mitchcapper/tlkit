/*
 *   lomount
 *   Copyright (C) 2007 Alejandro Liu Ly
 *
 *   lomount is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of 
 *   the License, or (at your option) any later version.
 *
 *   lomount is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program.  If not, see 
 *   <http://www.gnu.org/licenses/>
 */

/*
 *++
 * NAME
 *    lomount 1
 * SUMMARY
 *    allow normal users to mount loop images
 * SYNOPSIS
 *    *lomount* _[options]_ _file_ _dir_
 *
 *    *loumount* _dir_
 * DESCRIPTION
 *    *lomount* will mount a loop image on behalf of a normal user.
 *    This means that allows a normal user to call =/bin/mount=
 *    without having to configure the =/etc/fstab= file.
 *
 *    It is esseintally equivalent to doing:
 *
 *    : mount -o loop,nosuid,noexec,nodev file /mnt/point
 *
 *    Mounts are always given =nosuid,nodev,noexec= options, for security
 *    reasons.
 * OPTIONS
 *    * *-u,* *--umount*
 *      Unmount the image.
 *    * *-h,* *--help*
 *      Show a help message.
 *    * *-v,* *--version*
 *      Print the version string and exit.
 *    * *-o,* *--options,* *--opts* _mount_options_
 *      Pass these options to the mount command.
 *    * *-r,* *--read-only*
 *      Read-only mount.
 *    * *-w,* *--read-write*
 *      Read-write mount.
 * SEE ALSO
 *    *losetup(8)*
 *    *mount(8)*
 *--
 */
#define GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <getopt.h>
//#include <kitlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>

typedef enum {FALSE = 0, TRUE } bool_t;
#define MAXARGTAB 1024

#ifdef VERSION
const char version[] = VERSION;
#else
const char version[] = "Exp";
#endif
const char copyright[] = "Copyright (c) 2007 Alejandro Liu Ly";
const char *cmdname = "lomount";

const char *mntcmd = "/bin/mount";
const char *umntcmd = "/bin/umount";

//const char *mntcmd = "/bin/echo";
//const char *umntcmd = "/bin/echo";

const char *stdopts = ",loop,nosuid,noexec,nodev";
const char *uidopts=",uid=%d,gid=%d";

bool_t opt_umount = FALSE;
char *opt_mntopts = "";
bool_t is_readonly = FALSE;
const char *m_optstr = "ho:rvw";
const struct option m_longopts[] = {
  { "--options",1,0,'o' },
  { "--opts",1,0,'o' },
  { "--help",0,0,'h' },
  { "--version",0,0,'v' },
  { "--read-only",0,0,'r'},
  { "--read-write",0,0,'w'},
  { "--rw",0,0,'w'},
  { 0,0,0,0 }
};
const char *u_optstr = "hv";
const struct option u_longopts[] = {
  { "--help",0,0,'h' },
  { "--version",0,0,'v' },
  { 0,0,0,0 }
};
uid_t myuid;
const char *myusrname;
const char UMOUNT[] = "umount";

void pvs(FILE *out) {
  fprintf(out,"%s v%s\n%s\n",cmdname,version,copyright);
}

void usage() {
  fprintf(stderr,"Usage:\n\t%s [options] <file> <mntpoint>\nOptions:\n",cmdname);
  fprintf(stderr,"\t--umount, --unmount, -u\n\tUnmount file system\n");
  fprintf(stderr,"\t--opts, --options, -o [mount options]\n\tMount options\n");
  fprintf(stderr,"\t--read-only, -r\n\tRead only mount\n");
  fprintf(stderr,"\t--read-write, -w\n\tRead-Write mount\n");
  pvs(stderr);
  exit(1);
}

/**********************************************************************/
/*+*/
void fatal(const char *fmt,...) {
  /*    Report a fatal error
   * ARGS
   *    * fmt - printf format
   *    * varargs
   *-*/
  extern const char *cmdname;
  fputs(cmdname,stderr);  putc(':',stderr);   putc(' ',stderr);
  va_list ap;  va_start(ap,fmt);  vfprintf(stderr,fmt,ap);  va_end(ap);
  putc('\n',stderr);
  exit(1);
}
/*+*/
void pfatal(const char *fmt,...) {
  /*    Report a fatal error message
   * ARGS
   *    * fmt - printf format
   *    * varargs
   *-*/
  extern const char *cmdname;
  fputs(cmdname,stderr);  putc(':',stderr);  putc(' ',stderr);
  va_list ap;  va_start(ap,fmt);  vfprintf(stderr,fmt,ap);  va_end(ap);
  perror(" ");
  exit(1);
}

/*+*/
static inline void *xmalloc(size_t n) {
  void *p = malloc(n);
  if (!p) pfatal("xmalloc(%d)",n);
  return p;
}
static inline char *xstrdup(const char *n) {
  char *p= strdup(n);
  if (!p) pfatal("xstrdup(%s)",n);
  return p;
}

int spawnl(const char *path,...) {
  pid_t pid=fork();
  int status;

  if (pid == -1) return -1;
  if (!pid) {
    char *args[MAXARGTAB];
    va_list ap;

    va_start(ap,path);
    status=0;
    do {
      args[status] = (char *)va_arg(ap,char *);
    } while (args[status++] && status < MAXARGTAB);
    va_end(ap);
    execv(path,args);
    exit(-1);
  }
  while (waitpid(pid,&status,0) == -1);
  return status;
}


/**********************************************************************/
void do_umount(int argc,char **argv) {
  int c;
  struct stat stb;
  char *umntdat;

  for (;;) {
    int option_idx = 0;
    c = getopt_long(argc,argv,u_optstr,u_longopts, &option_idx);
    if (c == -1) break;
    switch (c) {
    case 'h':
      usage();
      break;
    case 'v':
      pvs(stdout);
      exit(0);
    default:
      fprintf(stderr,"Invalid option\n");
      usage();
    }
  }
  if (optind+1 != argc) usage();

  umntdat = argv[optind];
  if (stat(umntdat,&stb) == -1) pfatal("stat %s",umntdat);
  if (stb.st_uid != myuid) fatal("You do not own %s",umntdat);

  setreuid(0,0);

  /* Attempt to unmount the thing... */
  c = spawnl(umntcmd,umntcmd,umntdat,NULL);
  exit(c);
}

void do_mount(int argc,char **argv) {
  int c, l;
  struct stat stb;
  char *loopfile;
  char *mntpnt;
  char *mopts;
  uid_t ruid = getuid();
  gid_t rgid = getgid();

  for (;;) {
    int option_idx = 0;
    c = getopt_long(argc,argv,m_optstr,m_longopts, &option_idx);
    if (c == -1) break;
    switch (c) {
    case 'h':
      usage();
      break;
    case 'v':
      pvs(stdout);
      exit(0);
    case 'o':
      opt_mntopts = optarg;
      break;
    case 'r':
      is_readonly = TRUE;
      break;
    case 'w':
      is_readonly = FALSE;
      break;
    default:
      fprintf(stderr,"Invalid option\n");
      usage();
    }
  }
  if (optind+2 != argc) usage();

  loopfile = argv[optind];
  mntpnt = argv[optind+1];

  if (stat(loopfile,&stb) == -1) pfatal("stat %s",loopfile);
  if (stb.st_uid != myuid) fatal("You do not own file %s", loopfile);
  if (!S_ISREG(stb.st_mode)) fatal("%s is not a regular file",loopfile);
  if ((S_IWUSR & stb.st_mode) != S_IWUSR && !is_readonly) {
    fprintf(stderr,"%s: will be mounted read-only\n",loopfile);
    is_readonly = TRUE;
  }
  if (stat(mntpnt,&stb) == -1) pfatal("stat %s",mntpnt);
  if (stb.st_uid != myuid) fatal("You do not own directory %s", mntpnt);
  if (!S_ISDIR(stb.st_mode)) fatal("%s is not a directory",mntpnt);

  l = strlen(opt_mntopts);
  mopts = xmalloc(l+strlen(stdopts)+strlen(uidopts)+20);

  setreuid(0,0);

  /*
   * First try... use uid and gid
   */
  if (l) {
    strcpy(mopts,opt_mntopts);
    sprintf(mopts+l,uidopts,ruid,rgid);
  } else {
    sprintf(mopts,uidopts+1,ruid,rgid);
  }
  strcat(mopts,stdopts);

  //printf("mopts=%s\n",mopts);

  c = spawnl(mntcmd,mntcmd,
	     (is_readonly ? "-r" : "-w"),
	     "-o",mopts,
	     loopfile,mntpnt,
	     NULL);

  if (!c) exit(0);	/* Succesful?  we are done! */

  /*
   * Second try ... without uid,gid...
   */
  if (c) {
    strcpy(mopts,opt_mntopts);
    strcat(mopts,stdopts);
  } else {
    strcpy(mopts,stdopts+1);
  }

  c = spawnl(mntcmd,mntcmd,
	     (is_readonly ? "-r" : "-w"),
	     "-o",mopts,
	     loopfile,mntpnt,
	     NULL);
  if (c) exit(1);
  exit(0);
}


int main(int argc,char **argv) {
  bool_t is_mount = TRUE;

  cmdname = argv[0];
  {
    unsigned l = strlen(cmdname);
    if (l >= sizeof(UMOUNT)-1)
      is_mount = strcmp(cmdname+l-sizeof(UMOUNT)+1,UMOUNT);
  }

  myuid = getuid();
  if (!myuid) fatal("You should not run this as root");
  {
    struct passwd *pwd = getpwuid(myuid);
    if (!pwd) pfatal("getpwuid");
    myusrname = xstrdup(pwd->pw_name);
  }
  if (geteuid()) fatal("%s must be installed setuid root", cmdname);

  if (argc == 1) usage();
  if (is_mount && (!strcmp(argv[1],"--umount") 
		   || !strcmp(argv[1],"--umount")
		   || !strcmp(argv[1],"-u"))) {
    ++argv;
    --argc;
    is_mount = FALSE;
  }
  if (is_mount) 
    do_mount(argc,argv);
  else
    do_umount(argc,argv);
  return -1;
}
