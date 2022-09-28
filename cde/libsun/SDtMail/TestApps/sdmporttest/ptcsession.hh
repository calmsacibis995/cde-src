class SdmSession;

typedef enum sessionType_T { st_unknown, st_input, st_output}  sessionType;

sessionType parseSessionType(char *type, char *&sessionName, SdmSession **&sesptr);

int Csession(int argc, char **argv);
extern char *Hsession;
