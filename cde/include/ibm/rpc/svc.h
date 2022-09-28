extern "C" {
struct sockaddr_in;
struct opaque_auth;
#define svc_register ____svc_register
#define svc_unregister ____svc_unregister
#define xprt_register ____xprt_register
#define xprt_unregister ____xprt_unregister
#define svc_sendreply ____svc_sendreply
#define svcerr_decode ____svcerr_decode
#define svcerr_weakauth ____svcerr_weakauth
#define svcerr_noproc ____svcerr_noproc
#define svcerr_progvers ____svcerr_progvers
#define svcerr_auth ____svcerr_auth
#define svcerr_noprog ____svcerr_noprog
#define svcerr_systemerr ____svcerr_systemerr
#define svc_getreqset ____svc_getreqset
#define svcudp_create ____svcudp_create
#define svctcp_create ____svctcp_create
#define svctcp_create ____svctcp_create
#include </usr/include/rpc/svc.h>
#undef svc_getargs
#define svc_getargs(xprt, xargs, argsp)                 \
      (*(bool_t (*)(SVCXPRT *, xdrproc_t, caddr_t))((xprt)->xp_ops->xp_getargs))((xprt), (xargs), (argsp))
#undef svc_freeargs
#define svc_freeargs(xprt, xargs, argsp)              \
      (*(bool_t (*)(SVCXPRT *, xdrproc_t, caddr_t))((xprt)->xp_ops->xp_freeargs))((xprt), (xargs), (argsp))
#undef svc_register
#undef svc_unregister
#undef xprt_register
#undef xprt_unregister
#undef svc_sendreply
#undef svcerr_decode
#undef svcerr_weakauth
#undef svcerr_noproc
#undef svcerr_progvers
#undef svcerr_auth
#undef svcerr_noprog
#undef svcerr_systemerr
#undef svc_getreqset
#undef svcudp_create
#undef svctcp_create
#undef svctcp_create
extern bool_t   svc_register(SVCXPRT *,u_long, u_long, void (*dispatch)(), int);
extern void     svc_unregister(u_long, u_long);
extern void     xprt_register(SVCXPRT *);
extern void     xprt_unregister(SVCXPRT *);
extern bool_t   svc_sendreply(SVCXPRT*,xdrproc_t,char*);
extern void     svcerr_decode(SVCXPRT *);
extern void     svcerr_weakauth(SVCXPRT *);
extern void     svcerr_noproc(SVCXPRT *);
extern void     svcerr_progvers(SVCXPRT *);
extern void     svcerr_auth(SVCXPRT *, enum auth_stat);
extern void     svcerr_noprog(SVCXPRT *);
extern void     svcerr_systemerr(SVCXPRT *);
extern void     svc_getreqset(fd_set *);
extern SVCXPRT *svcudp_create(int);
extern SVCXPRT *svctcp_create(int, u_int, u_int);
}
