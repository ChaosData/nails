#include <uwsgi.h>


int nails_init(void);
int nails_request(struct wsgi_request *);

struct uwsgi_plugin nails_plugin = {

        .name = "nails",
        .modifier1 = 150,
        .init = nails_init,
        .request = nails_request,

};

