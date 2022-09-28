#define FORMAT     "format"
#define UNFORMAT   "unformatted"
#define UNLABEL    "unlabeled"

typedef enum {Format, Unformatted, Unlabeled} Popup_Type;

extern Popup_Type popup_type;

extern char rawdev[];
extern char mntpt[];
