#include <uwsgi.h>


int j_init(void);
int j_request(struct wsgi_request *);

struct uwsgi_plugin j_plugin = {

        .name = "j",
        .modifier1 = 151,
        .init = j_init,
        .request = j_request,

};

