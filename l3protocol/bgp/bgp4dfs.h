#ifdef NEW_BGP_WANTED

#ifndef BGP4DFS_H
#define BGP4DFS_H
#ifdef __cplusplus
extern "C" {
#endif

#define BGP4_DEFAULT_ROUTE_NUMBER 10000 
#define BGP4_DEFAULT_PATH_NUMBER    3000 
#define BGP4_DEFAULT_PEER_NUMBER  30
#define BGP4_SERVER_PORT			179 
#define BGP4_INVALID_SOCKET -1
#define BGP4_TCP_BUFFER_LEN 2000000
#define BGP4_RTSOCK_BUFFER_LEN 213000

#define BGP4_MAX_PROTO 16 

#define BGP4_PRIVATE_AS_MIN 64512/*private AS num is from this value to 65535*/


/*macro for af operation*/
#define flag_set(x, y) do{(x) |= (0x00000001 << (y)) ;}while(0)
#define flag_clear(x, y) do{(x) &= ~(0x00000001 << (y)) ;}while(0)
#define flag_isset(x, y) ((x) & (0x00000001 << (y)))

#define bgp_ip4(x)  (*((u_int*)(x)))


#ifdef __cplusplus
}
#endif   
#endif /* BGP4DFS_H */

#else

#ifndef BGP4DFS_H
#define BGP4DFS_H
#ifdef __cplusplus
      extern "C" {
     #endif
 
 #define BGP4_DEFAULT_ROUTE_NUMBER 10000 
 #define BGP4_DEFAULT_PATH_NUMBER    3000 
 #define BGP4_DEFAULT_PEER_NUMBER  30
 #define BGP4_SERVER_PORT           179 
 #define BGP4_CLIENT_PORT           179
 #define BGP4_INVALID_SOCKET -1
 #define BGP4_TCP_BUFFER_LEN 2000000
 #define BGP4_RTSOCK_BUFFER_LEN 213000

#define BGP4_MAX_PROTO 16 

#define BGP4_PRIVATE_AS_MIN 64512/*private AS num is from this value to 65535*/


/*macro for af operation*/
#define af_set(x, y) do{(x) |= (0x00000001 << (y)) ;}while(0)
#define af_clear(x, y) do{(x) &= ~(0x00000001 << (y)) ;}while(0)
#define af_isset(x, y) ((x) & (0x00000001 << (y)))

#define bgp_ip4(x)  (*((u_int*)(x)))


#ifdef __cplusplus
     }
     #endif   
#endif /* BGP4DFS_H */


#endif
