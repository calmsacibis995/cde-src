/*
 * mmap len bytes of data using file fd.
 * This will allow the user not to eat up vmem for 
 * large/temporary areas of memory */
char *fs_mmap(int *fd, long len, char **name)
{
  caddr_t addr;

  *name = tempnam(NIL, "rmmap");
  if (*name == NULL) {
    *fd = -1;
    return NIL;
  }
  *fd = open(*name, O_RDWR | O_CREAT, 0666);
  if (*fd < 0) return NIL;

  /*  OK, mmap the bytes requested */
  addr = mmap((caddr_t)0, len, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 
	      (off_t)0);

  if (addr == MAP_FAILED) {
    close(*fd);
    *fd = -1;
    return NIL;
  } else
    return ((char *)addr);
}

int fs_mumap(int fd, char *name,caddr_t buf,size_t len)
{
  int ok = munmap(buf,len);
  close(fd);
  unlink(name);
  return ok;
}

int fs_mmapput(int fd, char c)
{
  int ok = write(fd, &c, 1);

  return ok;
}
