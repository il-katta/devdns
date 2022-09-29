extern "C" {
#include <gnutls/abstract.h>
#include <sys/stat.h>
#include "config.h"
#include "json.h"
#include "crypto.h"

typedef gnutls_privkey_t privkey_t;

#define PRODUCTION_URL "https://acme-v02.api.letsencrypt.org/directory"
#define STAGING_URL "https://acme-staging-v02.api.letsencrypt.org/directory"
#define DEFAULT_CONFDIR SYSCONFDIR "/ssl/uacme"

typedef struct acme {
    privkey_t key;
    json_value_t *json;
    json_value_t *account;
    json_value_t *dir;
    json_value_t *order;
    char *nonce;
    char *kid;
    char *headers;
    char *body;
    char *type;
    unsigned char alt_fp[32];
    size_t alt_fp_len;
    size_t alt_n;
    const char *eab_keyid;
    const char *eab_key;
    const char *directory;
    const char *hook;
    const char *email;
    char *keyprefix;
    char *certprefix;
} acme_t;

#if !HAVE_STRCASESTR
char *strcasestr(const char *haystack, const char *needle);
#endif

char *find_header(const char *headers, const char *name);

int acme_get(acme_t *a, const char *url);

int acme_post(acme_t *a, const char *url, const char *format, ...);

int hook_run(const char *prog, const char *method, const char *type, const char *ident, const char *token,
             const char *auth);

bool check_or_mkdir(bool allow_create, const char *dir, mode_t mode);

char *identifiers(char *const *names);

bool acme_error(acme_t *a);

bool acme_bootstrap(acme_t *a);

char *eab_encode(acme_t *a, const char *url);

bool account_new(acme_t *a, bool yes);

bool account_retrieve(acme_t *a);

bool account_update(acme_t *a);

bool account_keychange(acme_t *a, bool never, keytype_t type, int bits);

bool account_deactivate(acme_t *a);

bool authorize(acme_t *a);

bool cert_issue(acme_t *a, char *const *names, const char *csr);

bool cert_revoke(acme_t *a, const char *certfile, int reason_code);

bool validate_identifier_str(const char *s);

bool eab_parse(acme_t *a, char *eab);

bool alt_parse(acme_t *a, char *alt);

void usage(const char *progname);

void version(const char *progname);

//int main(int argc, char **argv);
}