#include <qr/ecc.h>
#include <stdlib.h>
#include <string.h>

#define GF_SIZE 256
#define PRIMITIVE 0x11D

static uint8_t gf_log[GF_SIZE];
static uint8_t gf_antilog[GF_SIZE];
static int gf_tables_initialized = 0;

static void gf_init_tables(void)
{
    if (gf_tables_initialized) return;

    uint8_t x = 1;
    for (int i = 0; i < GF_SIZE - 1; i++)
    {
        gf_antilog[i] = x;
        gf_log[x] = i;
        x = (x << 1) ^ ((x & 0x80) ? PRIMITIVE : 0);
    }

    gf_log[0] = 0;
    gf_tables_initialized = 1;
}

static inline uint8_t gf_mul(uint8_t a, uint8_t b)
{
    if (a == 0 || b == 0) return 0;
    return gf_antilog[(gf_log[a] + gf_log[b]) % (GF_SIZE - 1)];
}

static void generate_poly(uint8_t *poly, size_t degree)
{
    poly[0] = 1;
    for (size_t i = 1; i <= degree; i++)
    {
        poly[i] = 0;
    }

    for (size_t i = 1; i < degree; i++)
    {
        uint8_t coef = gf_antilog[i];
        for (size_t j = degree; j > 0; j--)
        {
            poly[j] = poly[j-1] ^ gf_mul(poly[j], coef);
        }
        poly[0] = gf_mul(poly[0], coef);
    }
}

static void poly_div(uint8_t *dividend, size_t dividend_len, const uint8_t *divisor, size_t divisor_len)
{
    for (size_t i = 0; i < dividend_len - divisor_len + 1; i++)
    {
        if (dividend[i] == 0) continue;

        uint8_t coef = dividend[i];
        for (size_t j = 0; j < divisor_len; j++)
        {
            dividend[i + j] ^= gf_mul(divisor[j], coef);
        }
    }
}

qr_ec *qr_ec_create(size_t data_length, size_t ecc_length)
{
    qr_ec *ec = (qr_ec *)calloc(1, sizeof(qr_ec));
    if (!ec) return NULL;

    ec->data = (uint8_t *)malloc(data_length);
    ec->ecc = (uint8_t *)calloc(ecc_length, 1);
    ec->generator = (uint8_t *)malloc(ecc_length + 1);

    if (!ec->data || !ec->ecc || !ec->generator)
    {
        qr_ec_destroy(ec);
        return NULL;
    }

    ec->data_length = data_length;
    ec->ecc_length = ecc_length;

    gf_init_tables();
    generate_poly(ec->generator, ecc_length);

    return ec;
}

void qr_ec_destroy(qr_ec *ec)
{
    if (!ec) return;

    free(ec->data);
    free(ec->ecc);
    free(ec->generator);
    free(ec);
}

int qr_ec_encode(qr_ec *ec, const uint8_t *data, uint8_t *ecc)
{
    if (!ec || !data || !ecc) return -1;

    uint8_t *msg = (uint8_t *)malloc(ec->data_length + ec->ecc_length);
    if (!msg) return -1;

    memcpy(msg, data, ec->data_length);
    memset(msg + ec->data_length, 0, ec->ecc_length);

    poly_div(msg, ec->data_length + ec->ecc_length, ec->generator, ec->ecc_length + 1);

    memcpy(ecc, msg + ec->data_length, ec->ecc_length);
    free(msg);
    return 0;
}
