#############################################################################
# DEFINE

PL_LIBSSH_NACL = false
PL_LIBSSH_PTHREAD=true
PL_LIBSSH_SERVER=true
PL_LIBSSH_GSSAPI=false
PL_LIBSSH_ZLIB=true
PL_LIBSSH_SSH1=true
PL_LIBSSH_SFTP=true
PL_LIBSSH_GCRYPT=false
###########################################################################
MODULEDIR = component/ssh
#PLINCLUDE += -I$(OPENSSH_ROOT)/include
#OS
LIBSSHOBJS += agent.o \
  auth.o \
  base64.o \
  bignum.o \
  ssh_buffer.o \
  callbacks.o \
  channels.o \
  client.o \
  config.o \
  connect.o \
  curve25519.o \
  dh.o \
  ecdh.o \
  error.o \
  getpass.o \
  init.o \
  kex.o \
  known_hosts.o \
  legacy.o \
  log.o \
  match.o \
  messages.o \
  misc.o \
  options.o \
  packet.o \
  packet_cb.o \
  packet_crypt.o \
  pcap.o \
  pki.o \
  pki_container_openssh.o \
  pki_ed25519.o \
  poll.o \
  session.o \
  scp.o \
  socket.o \
  string.o \
  threads.o \
  wrapper.o \
  bcrypt_pbkdf.o \
  blowfish.o \
  ed25519.o \
  fe25519.o \
  ge25519.o \
  sc25519.o


  
  
ifeq ($(strip $(PL_LIBSSH_GCRYPT)),true)
LIBSSHOBJS += libgcrypt.o \
        gcrypt_missing.o \
        pki_gcrypt.o
        
PLDEFINE +=-DHAVE_LIBGCRYPT
else
LIBSSHOBJS += pki_crypto.o \
        libcrypto.o 
   # if(OPENSSL_VERSION VERSION_LESS "1.1.0")
LIBSSHOBJS += libcrypto-compat.o
  #  endif
#PLDEFINE +=-DHAVE_LIBGCRYPT
endif

ifeq ($(strip $(PL_LIBSSH_SFTP)),true)
LIBSSHOBJS += sftp.o
ifeq ($(strip $(PL_LIBSSH_SERVER)),true)
LIBSSHOBJS += sftpserver.o
endif
endif

ifeq ($(strip $(PL_LIBSSH_SSH1)),true)
LIBSSHOBJS += auth1.o \
    channels1.o \
    crc32.o \
    kex1.o \
    packet1.o
endif

ifeq ($(strip $(PL_LIBSSH_SERVER)),true)
LIBSSHOBJS += server.o \
    bind.o
endif

ifeq ($(strip $(PL_LIBSSH_ZLIB)),true)
LIBSSHOBJS += gzip.o
endif

ifeq ($(strip $(PL_LIBSSH_GSSAPI)),true)
#if (PL_LIBSSH_GSSAPI AND GSSAPI_FOUND)
LIBSSHOBJS += gssapi.o
#endif (PL_LIBSSH_GSSAPI AND GSSAPI_FOUND)
endif

ifeq ($(strip $(PL_LIBSSH_NACL)),false)
#if (NOT PL_LIBSSH_NACL)
LIBSSHOBJS += curve25519_ref.o
#endif (NOT PL_LIBSSH_NACL)
endif
ifeq ($(strip $(PL_LIBSSH_PTHREAD)),true)
LIBSSHOBJS += pthread.o 
PLDEFINE +=-DHAVE_PTHREAD
endif 


OBJS += ssh_util.o
OBJS += ssh_connect.o
OBJS += ssh_keymgt.o
OBJS += ssh_authentication.o
OBJS += ssh_client.o
OBJS += ssh_sftp.o
OBJS += ssh_sftpd.o
OBJS += ssh_scp.o
OBJS += sshd_main.o
OBJS += ssh_api.o
#############################################################################
# LIB
###########################################################################
LIBS = libssh.a
