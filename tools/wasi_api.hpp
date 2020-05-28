#pragma once

#include <eosio/vm/argument_proxy.hpp>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <cstring>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "wasi_errno.hpp"

extern char** environ;

namespace eosio { namespace vm {

   template <typename T, std::size_t Extent = eosio::vm::dynamic_extent>
   using span = eosio::vm::span<T, Extent>;

   template <typename T, std::size_t Align = alignof(T)>
   using legacy_ptr = eosio::vm::argument_proxy<T*, Align>;

   template <typename T, std::size_t Align = alignof(T)>
   using legacy_span = eosio::vm::argument_proxy<eosio::vm::span<T>, Align>;

   using wasi_size_t = uint32_t;
   using wasi_clockid_t = uint32_t;
   using wasi_timestamp_t = uint64_t;

   using wasi_fd_t = int;
   using wasi_filetype_t = uint8_t;
   using wasi_fdflags_t = uint16_t;
   using wasi_rights_t = uint64_t;

   constexpr wasi_filetype_t WASI_FILETYPE_UNKNOWN = 0;
   constexpr wasi_filetype_t WASI_FILETYPE_BLOCK_DEVICE = 1;
   constexpr wasi_filetype_t WASI_FILETYPE_CHARACTER_DEVICE = 2;
   constexpr wasi_filetype_t WASI_FILETYPE_DIRECTORY = 3;
   constexpr wasi_filetype_t WASI_FILETYPE_REGULAR_FILE = 4;
   constexpr wasi_filetype_t WASI_FILETYPE_SYMBOLIC_LINK = 7;

   constexpr wasi_fdflags_t WASI_FDFLAG_APPEND = (1 << 0);
   constexpr wasi_fdflags_t WASI_FDFLAG_DSYNC = (1 << 1);
   constexpr wasi_fdflags_t WASI_FDFLAG_NONBLOCK = (1 << 2);
   constexpr wasi_fdflags_t WASI_FDFLAG_RSYNC = (1 << 3);
   constexpr wasi_fdflags_t WASI_FDFLAG_SYNC = (1 << 4);

   using wasi_ptr_t = uint32_t;

   struct wasi_fdstat_t {
      wasi_filetype_t fs_filetype;
      wasi_fdflags_t fs_flags;
      wasi_rights_t fs_rights_base;
      wasi_rights_t fs_rights_inheriting;
   };

   using wasi_preopentype_t = uint8_t;

   constexpr wasi_preopentype_t WASI_PREOPENTYPE_DIR = 0;

   struct wasi_prestat_dir_t {
      wasi_size_t pr_name_len;
   };

   struct wasi_prestat_u_t {
      wasi_prestat_dir_t dir;
   };

   struct wasi_prestat_t {
      wasi_preopentype_t tag;
      wasi_prestat_u_t u;
   };

   struct wasi_iovec_t {
      //uint8_t* buf;
      wasi_ptr_t buf;
      wasi_size_t buf_len;
   };

   struct wasi_ciovec_t {
      //const uint8_t* buf;
      wasi_ptr_t buf;
      wasi_size_t buf_len;
   };

   using wasi_filedelta_t = int64_t;
   using wasi_whence_t = uint8_t;
   using wasi_filesize_t = uint64_t;

   constexpr wasi_whence_t WASI_WHENCE_SET = 0;
   constexpr wasi_whence_t WASI_WHENCE_CUR = 1;
   constexpr wasi_whence_t WASI_WHENCE_END = 2;

   using wasi_device_t = uint64_t;
   using wasi_inode_t = uint64_t;
   using wasi_linkcount_t = uint64_t;

   struct wasi_filestat_t {
      wasi_device_t dev;
      wasi_inode_t ino;
      wasi_filetype_t filetype;
      wasi_linkcount_t nlink;
      wasi_filesize_t size;
      wasi_timestamp_t atim;
      wasi_timestamp_t mtim;
      wasi_timestamp_t ctim;
   };

   using wasi_lookupflags_t = uint32_t;
   using wasi_oflags_t = uint16_t;

   constexpr wasi_oflags_t WASI_OFLAGS_CREAT = (1 << 0);
   constexpr wasi_oflags_t WASI_OFLAGS_DIRECTORY = (1 << 1);
   constexpr wasi_oflags_t WASI_OFLAGS_EXCL = (1 << 2);
   constexpr wasi_oflags_t WASI_OFLAGS_TRUNC = (1 << 3);

   using wasi_exitcode_t = uint32_t;

   struct preopen {
      int fd;
      const char* path;
   };
   
   preopen preopens[] = {
      { 0, "<stdin>" },
      { 1, "<stdout>" },
      { 2, "<stderr>" },
      { -1, "./"},
   };

   constexpr size_t preopens_cnt = 4;

   struct wasi_api {

      wasi_api(int argc, char** argv)
      : argc(argc), argv(argv) {
         for (auto i = 3; i < preopens_cnt; ++i) {
            preopens[i].fd = open(preopens[i].path, O_RDONLY);
         }
      }

      wasi_errno_t args_get(wasi_ptr_t* argv, wasi_ptr_t argv_buf) {
         for (auto i = 0; i < this->argc; ++i) {
            argv[i] = argv_buf;

            wasi_size_t len = std::strlen(this->argv[i]);
            std::memcpy(offset_to_ptr(argv_buf), this->argv[i], len);
            argv_buf += len;
            *reinterpret_cast<uint8_t*>(offset_to_ptr(argv_buf)) = 0;
            argv_buf += 1;
         }

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t args_sizes_get(wasi_size_t* argc, wasi_size_t* argv_buf_size) {
         wasi_size_t buflen = 0;

         for (auto i = 0; i < this->argc; ++i) {
            buflen += std::strlen(this->argv[i]) + 1;
         }

         *argc = this->argc;
         *argv_buf_size = buflen;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t environ_get(uint8_t** env, uint8_t* environ_buf) {
         // TODO
         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t environ_sizes_get(wasi_size_t* environc, wasi_size_t* environ_buf_size) {
         wasi_size_t envc = 0;
         wasi_size_t envb = 0;
         
         for (auto env = environ; *env; ++env) {
            envc += 1;
            envb += std::strlen(*env);
         }

         *environc = envc;
         *environ_buf_size = envb;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t clock_res_get(wasi_clockid_t id, wasi_timestamp_t* resolution) {
         // TODO
         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t clock_time_get(wasi_clockid_t id, wasi_timestamp_t precision, wasi_timestamp_t* time) {
         struct timespec ts;

         if (clock_gettime(0, &ts) != 0)
            return errno_to_wasi(errno);

         *time = (wasi_timestamp_t)ts.tv_sec * 1000000000 + ts.tv_nsec;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_close(wasi_fd_t fd) {
         return close(fd);
      }

      wasi_errno_t fd_datasync(wasi_fd_t fd) {
         return fdatasync(fd);
      }

      wasi_errno_t fd_fdstat_get(wasi_fd_t fd, legacy_ptr<wasi_fdstat_t> stat) {
         struct stat fd_stat;

         int flags = fcntl(fd, F_GETFL);
         if (flags < 0)
            return errno_to_wasi(errno);

         fstat(fd, &fd_stat);
         int mode = fd_stat.st_mode;

         stat->fs_filetype = (S_ISBLK(mode) ? WASI_FILETYPE_BLOCK_DEVICE : 0) |
                             (S_ISCHR(mode) ? WASI_FILETYPE_CHARACTER_DEVICE : 0) |
                             (S_ISDIR(mode) ? WASI_FILETYPE_DIRECTORY : 0) |
                             (S_ISREG(mode) ? WASI_FILETYPE_REGULAR_FILE : 0) |
                             (S_ISLNK(mode) ? WASI_FILETYPE_SYMBOLIC_LINK : 0);
         stat->fs_flags = ((flags & O_APPEND) ? WASI_FDFLAG_APPEND : 0) |
                          ((flags & O_DSYNC) ? WASI_FDFLAG_DSYNC : 0) |
                          ((flags & O_NONBLOCK) ? WASI_FDFLAG_NONBLOCK : 0) |
                          ((flags & O_SYNC) ? WASI_FDFLAG_SYNC : 0);
         stat->fs_rights_base = std::numeric_limits<uint64_t>::max();
         stat->fs_rights_inheriting = std::numeric_limits<uint64_t>::max();

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_fdstat_set_flags(wasi_fd_t fd, wasi_fdflags_t flags) {
         // TODO
         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_prestat_get(wasi_fd_t fd, legacy_ptr<wasi_prestat_t> buf) {
         if (fd < 3 || fd >= preopens_cnt)
            return WASI_ERRNO_BADF;

         buf->tag = WASI_PREOPENTYPE_DIR;
         buf->u.dir.pr_name_len = std::strlen(preopens[fd].path);

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_prestat_dir_name(wasi_fd_t fd, uint8_t* path, wasi_size_t path_len) {
         if (fd < 3 || fd >= preopens_cnt)
            return WASI_ERRNO_BADF;

         auto size = std::min(std::strlen(preopens[fd].path), (size_t)path_len);
         std::memcpy(path, preopens[fd].path, size);

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_read(wasi_fd_t fd, legacy_span<const wasi_iovec_t> wasi_iovs, wasi_size_t* nread) {
         struct iovec iovs[wasi_iovs.size()];

         for (auto i = 0; i < wasi_iovs.size(); ++i) {
            iovs[i].iov_base = offset_to_ptr(wasi_iovs[i].buf);
            iovs[i].iov_len = wasi_iovs[i].buf_len;
         }

         ssize_t ret = readv(fd, iovs, wasi_iovs.size());
         if (ret < 0)
            return errno_to_wasi(errno);

         *nread = ret;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_seek(wasi_fd_t fd, wasi_filedelta_t offset, wasi_whence_t whence, wasi_filesize_t* newoffset) {
         auto wasi_whence = whence == WASI_WHENCE_END ? SEEK_END :
                                      WASI_WHENCE_CUR ? SEEK_CUR : 0;

         auto ret = lseek(fd, offset, wasi_whence);
         if (ret < 0)
            return errno_to_wasi(errno);

         *newoffset = ret;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t fd_write(wasi_fd_t fd, legacy_span<const wasi_ciovec_t> wasi_iovs, wasi_size_t* nwritten) {
         struct iovec iovs[wasi_iovs.size()];

         for (auto i = 0; i < wasi_iovs.size(); ++i) {
            iovs[i].iov_base = offset_to_ptr(wasi_iovs[i].buf);
            iovs[i].iov_len = wasi_iovs[i].buf_len;
         }

         ssize_t ret = writev(fd, iovs, wasi_iovs.size());
         if (ret < 0)
            return errno_to_wasi(errno);

         *nwritten = ret;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t path_open(wasi_fd_t fd, wasi_lookupflags_t dirflags, const char* path, wasi_size_t path_len,
         wasi_oflags_t oflags, wasi_rights_t fs_rights_base, wasi_rights_t fs_rights_inheriting, wasi_fdflags_t fdflags,
         wasi_fd_t* opened_fd) {
         if (path_len >= 512)
            return WASI_ERRNO_INVAL;

         char host_path[path_len+1];

         std::memcpy(host_path, path, path_len);
         host_path[path_len] = '\0';

         auto flags = ((oflags & WASI_OFLAGS_CREAT) ? O_CREAT : 0) |
                      ((oflags & WASI_OFLAGS_EXCL) ? O_EXCL : 0) |
                      ((oflags & WASI_OFLAGS_TRUNC) ? O_TRUNC : 0) |
                      ((fdflags & WASI_FDFLAG_APPEND) ? O_APPEND : 0) |
                      ((fdflags & WASI_FDFLAG_DSYNC) ? O_DSYNC : 0) |
                      ((fdflags & WASI_FDFLAG_NONBLOCK) ? O_NONBLOCK : 0) |
                      ((fdflags & WASI_FDFLAG_SYNC) ? O_SYNC : 0);

         int mode = 0644;
         int host_fd = openat(fd, host_path, flags, mode);

         if (host_fd < 0) {
            return errno_to_wasi(errno);
         }

         *opened_fd = host_fd;

         return WASI_ERRNO_SUCCESS;
      }

      wasi_errno_t path_filestat_get(wasi_fd_t fd, wasi_lookupflags_t flags, const char* path, wasi_size_t path_len, legacy_ptr<wasi_filestat_t> buf) {
         // TODO
      }
         
      wasi_errno_t path_unlink_file(wasi_fd_t fd, const char* path, wasi_size_t path_len) {
         // TODO
      }

      void proc_exit(wasi_exitcode_t rval) {
         this->exit_code = rval;
         exit();
      }

      wasi_errno_t random_get(uint8_t* buf, wasi_size_t buf_len) {
         for (;;) {
            ssize_t len = 0;

            len = getentropy(buf, buf_len) < 0 ? -1 : buf_len;

            if (len < 0) {
               if (errno == EINTR || errno == EAGAIN) {
                  continue;
               }
               return errno_to_wasi(errno);
            } else if (len != buf_len) {
              buf += len;
              buf_len -= len;
            } else {
               break;
            }
         }

         return WASI_ERRNO_SUCCESS;
      }

      int argc;
      char** argv;
      int exit_code = 0;

      std::function<void()> exit;
      std::function<void*(uint32_t offset)> offset_to_ptr;
   };

} }
