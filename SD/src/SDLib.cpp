#include "SDLib.h"
#include "SD.h"
#include <cstdio>

File sdfile;

/**
 * Initialize SD card
 * Has to be called once before any other SDLib function
 */
int sdlib_initialize(void) {
  if (!SD.begin()) {
    return 1;
  }
  return 0;
}

/**
* Open a file. The currently supported modes are "w", "r", and "rb"
*/
bool sdlib_open (const char *pcFile, const char *pcMode) {
  uint8_t mode;
    
  if ((strcmp(pcMode,"r") == 0) || (strcmp(pcMode,"rb") == 0)) {
      mode = O_READ;
  } else if (strcmp(pcMode,"w") == 0) {
      mode = O_WRITE | O_CREAT;
      if (sdlib_exists(pcFile)) {
        if (!SD.remove(pcFile)) {
          printf ("(Error)~  sdlib_open: Failed to remove already existing file <%s>.\r\n",pcFile);
        }
      }
  } else {
      printf ("ff_fopen: pcMode:<%s> is not implemented.\r\n",pcMode);
      return false;
  }

  sdfile =  SD.open(pcFile, mode);
  if (sdfile) {
    return true;
  } else {
    return false;
  }
}

/**
 * Return true if given filepath exists
 */
bool sdlib_exists(const char *filepath) {
  return SD.exists(filepath);
}

/**
 * Sync the date with the SD filesystem
 */
void sdlib_sync(const char *filename) {
  if (sdfile) {
    // check if it is the same file
    if (strcmp(sdfile.name(),filename)==0) {
      sdfile.flush();
    } else {
      printf ("(Error)~  sdlib_sync: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_sync: No file is open.\r\n");
  }
  return;
}

/**
 * Return size of the file given by `filename`
 * or 0 if no such file exists
 * `filename` has to be the same as the currently open file. It will report an error and returns 0 otherwise.
 */
size_t sdlib_size(const char *filename) {
  if (sdfile) {
    // check if it is the same file
    if (strcmp(sdfile.name(),filename)==0) {
      return sdfile.size();
    } else {
      printf ("(Error)~  sdlib_size: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_size: No file is open.\r\n");
  }
  return 0;
}

/**
 * Seek by `new_offset`
 * `filename` has to be the same as the currently open file. It will report an error and returns false otherwise.
 */
bool sdlib_seek(const char *filename, uint32_t new_offset) {
  if (sdfile) {
    // check if it is the same file
    if (strcmp(sdfile.name(),filename)==0) {
      return sdfile.seek(new_offset);
    } else {
      printf ("(Error)~  sdlib_seek: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_seek: No file is open.\r\n");
  }
  // nothing worked, return 0
  return false;
}

/**
 * `tell` is called `position`
 * `filename` has to be the same as the currently open file. It will report an error and returns 0 otherwise.
 */
uint32_t sdlib_tell(const char *filename) {
  if (sdfile) {
    // check if it is the same file
    if (strcmp(sdfile.name(),filename)==0) {
      return sdfile.position();
    } else {
      printf ("(Error)~  sdlib_tell: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_tell: No file is open.\r\n");
  }
  return 0;
}

/**
 * Close the file, doesn't matter what filename it is, as long as it is open
 * `filename` has to be the same as the currently open file. It will report an error otherwise.
 */
void sdlib_close(const char *filename) {
  if (sdfile) {
    // check if it is the same file
    if (strcmp(sdfile.name(),filename)==0) {
      sdfile.close();
    } else {
      printf ("(Error)~  sdlib_close: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_close: No file is open.\r\n");
  }
  return;
}

/**
 * Open or create file specified by `filename` (O_READ | O_WRITE | O_CREAT | O_APPEND)
 * and write `buf` into it.
 * Return number of bytes written, or 0 if an error occured
 * `filename` has to be the same as the currently open file. It will report an error and returns 0 otherwise.
 */
size_t sdlib_write_to_file(const char *filename, const uint8_t *buf,
                          size_t size) {
  size_t r = 0;
  // is the file already open?
  if (sdfile) {
    // check filename
    if (strcmp(sdfile.name(),filename)==0) {
      r = sdfile.write((uint8_t *)buf, size);
      // return what we have
      return r;
    } else {
      printf ("(Error)~  sdlib_write_to_file: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_write_to_file: No file is open.\r\n");
  }
  return r;
}

/**
 * Read from a file specified by `filename` 
 * Return 0 if no bytes were read.
 * `filename` has to be the same as the currently open file. It will report an error and returns 0 otherwise.
 */
size_t sdlib_read_from_file(const char *filename, uint8_t *buf, size_t size) {
  size_t r = 0;
  if (sdfile) {
    // check filename
    if (strcmp(sdfile.name(),filename)==0) {
      // read from the file until there's nothing else in it or we fill the buffer
      while (sdfile.available() && r < size) {
        uint8_t c = sdfile.read();
        buf[(uint16_t)r] = c;
        r++;
      }
      // return what we have
      return r; 
    } else {
      printf ("(Error)~  sdlib_write_to_file: Called for <%s>, while <%s> is already opened.\r\n",filename,sdfile.name());
    }
  } else {
    printf ("(Error)~  sdlib_write_to_file: No file is open.\r\n");
  }
  return r;
}
