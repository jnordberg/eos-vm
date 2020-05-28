#pragma once

namespace eosio { namespace vm {

   //using wasi_errno_t = uint16_t;
   using wasi_errno_t = uint32_t;

   constexpr wasi_errno_t WASI_ERRNO_SUCCESS = 0;
   constexpr wasi_errno_t WASI_ERRNO_2BIG = 1;
   constexpr wasi_errno_t WASI_ERRNO_ACCES = 2;
   constexpr wasi_errno_t WASI_ERRNO_ADDRINUSE = 3;
   constexpr wasi_errno_t WASI_ERRNO_ADDRNOTAVAIL = 4;
   constexpr wasi_errno_t WASI_ERRNO_AFNOSUPPORT = 5;
   constexpr wasi_errno_t WASI_ERRNO_AGAIN = 6;
   constexpr wasi_errno_t WASI_ERRNO_ALREADY = 7;
   constexpr wasi_errno_t WASI_ERRNO_BADF = 8;
   constexpr wasi_errno_t WASI_ERRNO_BADMSG = 9;
   constexpr wasi_errno_t WASI_ERRNO_BUSY = 10;
   constexpr wasi_errno_t WASI_ERRNO_CANCELED = 11;
   constexpr wasi_errno_t WASI_ERRNO_CHILD = 12;
   constexpr wasi_errno_t WASI_ERRNO_CONNABORTED = 13;
   constexpr wasi_errno_t WASI_ERRNO_CONNREFUSED = 14;
   constexpr wasi_errno_t WASI_ERRNO_CONNRESET = 15;
   constexpr wasi_errno_t WASI_ERRNO_DEADLK = 16;
   constexpr wasi_errno_t WASI_ERRNO_DESTADDRREQ = 17;
   constexpr wasi_errno_t WASI_ERRNO_DOM = 18;
   constexpr wasi_errno_t WASI_ERRNO_DQUOT = 19;
   constexpr wasi_errno_t WASI_ERRNO_EXIST = 20;
   constexpr wasi_errno_t WASI_ERRNO_FAULT = 21;
   constexpr wasi_errno_t WASI_ERRNO_FBIG = 22;
   constexpr wasi_errno_t WASI_ERRNO_HOSTUNREACH = 23;
   constexpr wasi_errno_t WASI_ERRNO_IDRM = 24;
   constexpr wasi_errno_t WASI_ERRNO_ILSEQ = 25;
   constexpr wasi_errno_t WASI_ERRNO_INPROGRESS = 26;
   constexpr wasi_errno_t WASI_ERRNO_INTR = 27;
   constexpr wasi_errno_t WASI_ERRNO_INVAL = 28;
   constexpr wasi_errno_t WASI_ERRNO_IO = 29;
   constexpr wasi_errno_t WASI_ERRNO_ISCONN = 30;
   constexpr wasi_errno_t WASI_ERRNO_ISDIR = 31;
   constexpr wasi_errno_t WASI_ERRNO_LOOP = 32;
   constexpr wasi_errno_t WASI_ERRNO_MFILE = 33;
   constexpr wasi_errno_t WASI_ERRNO_MLINK = 34;
   constexpr wasi_errno_t WASI_ERRNO_MSGSIZE = 35;
   constexpr wasi_errno_t WASI_ERRNO_MULTIHOP = 36;
   constexpr wasi_errno_t WASI_ERRNO_NAMETOOLONG = 37;
   constexpr wasi_errno_t WASI_ERRNO_NETDOWN = 38;
   constexpr wasi_errno_t WASI_ERRNO_NETRESET = 39;
   constexpr wasi_errno_t WASI_ERRNO_NETUNREACH = 40;
   constexpr wasi_errno_t WASI_ERRNO_NFILE = 41;
   constexpr wasi_errno_t WASI_ERRNO_NOBUFS = 42;
   constexpr wasi_errno_t WASI_ERRNO_NODEV = 43;
   constexpr wasi_errno_t WASI_ERRNO_NOENT = 44;
   constexpr wasi_errno_t WASI_ERRNO_NOEXEC = 45;
   constexpr wasi_errno_t WASI_ERRNO_NOLCK = 46;
   constexpr wasi_errno_t WASI_ERRNO_NOLINK = 47;
   constexpr wasi_errno_t WASI_ERRNO_NOMEM = 48;
   constexpr wasi_errno_t WASI_ERRNO_NOMSG = 49;
   constexpr wasi_errno_t WASI_ERRNO_NOPROTOOPT = 50;
   constexpr wasi_errno_t WASI_ERRNO_NOSPC = 51;
   constexpr wasi_errno_t WASI_ERRNO_NOSYS = 52;
   constexpr wasi_errno_t WASI_ERRNO_NOTCONN = 53;
   constexpr wasi_errno_t WASI_ERRNO_NOTDIR = 54;
   constexpr wasi_errno_t WASI_ERRNO_NOTEMPTY = 55;
   constexpr wasi_errno_t WASI_ERRNO_NOTRECOVERABLE = 56;
   constexpr wasi_errno_t WASI_ERRNO_NOTSOCK = 57;
   constexpr wasi_errno_t WASI_ERRNO_NOTSUP = 58;
   constexpr wasi_errno_t WASI_ERRNO_NOTTY = 59;
   constexpr wasi_errno_t WASI_ERRNO_NXIO = 60;
   constexpr wasi_errno_t WASI_ERRNO_OVERFLOW = 61;
   constexpr wasi_errno_t WASI_ERRNO_OWNERDEAD = 62;
   constexpr wasi_errno_t WASI_ERRNO_PERM = 63;
   constexpr wasi_errno_t WASI_ERRNO_PIPE = 64;
   constexpr wasi_errno_t WASI_ERRNO_PROTO = 65;
   constexpr wasi_errno_t WASI_ERRNO_PROTONOSUPPORT = 66;
   constexpr wasi_errno_t WASI_ERRNO_PROTOTYPE = 67;
   constexpr wasi_errno_t WASI_ERRNO_RANGE = 68;
   constexpr wasi_errno_t WASI_ERRNO_ROFS = 69;
   constexpr wasi_errno_t WASI_ERRNO_SPIPE = 70;
   constexpr wasi_errno_t WASI_ERRNO_SRCH = 71;
   constexpr wasi_errno_t WASI_ERRNO_STALE = 72;
   constexpr wasi_errno_t WASI_ERRNO_TIMEDOUT = 73;
   constexpr wasi_errno_t WASI_ERRNO_TXTBSY = 74;
   constexpr wasi_errno_t WASI_ERRNO_XDEV = 75;
   constexpr wasi_errno_t WASI_ERRNO_NOTCAPABLE = 76;

#define CASE_ERRNO(name) \
   case E ## name: return WASI_ERRNO_ ## name;

   constexpr wasi_errno_t errno_to_wasi(int err) {
      switch (err) {
         CASE_ERRNO(2BIG)
         CASE_ERRNO(ACCES)
         CASE_ERRNO(ADDRINUSE)
         CASE_ERRNO(ADDRNOTAVAIL)
         CASE_ERRNO(AFNOSUPPORT)
         CASE_ERRNO(AGAIN)
         CASE_ERRNO(ALREADY)
         CASE_ERRNO(BADF)
         CASE_ERRNO(BADMSG)
         CASE_ERRNO(BUSY)
         CASE_ERRNO(CANCELED)
         CASE_ERRNO(CHILD)
         CASE_ERRNO(CONNABORTED)
         CASE_ERRNO(CONNREFUSED)
         CASE_ERRNO(CONNRESET)
         CASE_ERRNO(DEADLK)
         CASE_ERRNO(DESTADDRREQ)
         CASE_ERRNO(DOM)
         CASE_ERRNO(DQUOT)
         CASE_ERRNO(EXIST)
         CASE_ERRNO(FAULT)
         CASE_ERRNO(FBIG)
         CASE_ERRNO(HOSTUNREACH)
         CASE_ERRNO(IDRM)
         CASE_ERRNO(ILSEQ)
         CASE_ERRNO(INPROGRESS)
         CASE_ERRNO(INTR)
         CASE_ERRNO(INVAL)
         CASE_ERRNO(IO)
         CASE_ERRNO(ISCONN)
         CASE_ERRNO(ISDIR)
         CASE_ERRNO(LOOP)
         CASE_ERRNO(MFILE)
         CASE_ERRNO(MLINK)
         CASE_ERRNO(MSGSIZE)
         CASE_ERRNO(MULTIHOP)
         CASE_ERRNO(NAMETOOLONG)
         CASE_ERRNO(NETDOWN)
         CASE_ERRNO(NETRESET)
         CASE_ERRNO(NETUNREACH)
         CASE_ERRNO(NFILE)
         CASE_ERRNO(NOBUFS)
         CASE_ERRNO(NODEV)
         CASE_ERRNO(NOENT)
         CASE_ERRNO(NOEXEC)
         CASE_ERRNO(NOLCK)
         CASE_ERRNO(NOLINK)
         CASE_ERRNO(NOMEM)
         CASE_ERRNO(NOMSG)
         CASE_ERRNO(NOPROTOOPT)
         CASE_ERRNO(NOSPC)
         CASE_ERRNO(NOSYS)
         CASE_ERRNO(NOTCONN)
         CASE_ERRNO(NOTDIR)
         CASE_ERRNO(NOTEMPTY)
         CASE_ERRNO(NOTRECOVERABLE)
         CASE_ERRNO(NOTSOCK)
         CASE_ERRNO(NOTSUP)
         CASE_ERRNO(NOTTY)
         CASE_ERRNO(NXIO)
         CASE_ERRNO(OVERFLOW)
         CASE_ERRNO(OWNERDEAD)
         CASE_ERRNO(PERM)
         CASE_ERRNO(PIPE)
         CASE_ERRNO(PROTO)
         CASE_ERRNO(PROTONOSUPPORT)
         CASE_ERRNO(PROTOTYPE)
         CASE_ERRNO(RANGE)
         CASE_ERRNO(ROFS)
         CASE_ERRNO(SPIPE)
         CASE_ERRNO(SRCH)
         CASE_ERRNO(STALE)
         CASE_ERRNO(TIMEDOUT)
         CASE_ERRNO(TXTBSY)
         CASE_ERRNO(XDEV)
         //CASE_ERRNO(NOTCAPABLE)
         default:
            return WASI_ERRNO_SUCCESS;
      }
   }

} }
