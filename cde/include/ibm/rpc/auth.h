extern "C" {
struct opaque_auth;
struct sockaddr;
#define authdes_create ____authdes_create
#include </usr/include/rpc/auth.h>
#undef authdes_create
extern AUTH *authdes_create(char*,u_int,struct sockaddr*,des_block*);
#undef auth_destroy
#define auth_destroy(auth) ((*((void (*)(AUTH *))((auth)->ah_ops->ah_destroy)))(auth))
}
