//
// Created by tika on 24-9-17.
//

#include <random>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <uuid/uuid.h>
#include "funcs.h"

/**
 * auxiliary function for hash functions
 * @param row params
 * @param func_name who calls this function
 * @param hash hash algorithm
 * @param argc number of params
 * @return result
 */
static Cell *hash_aux(Row *row, const String &func_name, const EVP_MD *hash, int argc = 1) {
    check_arg_nums(row, func_name, argc);

    val cell = row->at(0);
    if (check_null(cell)) {
        return new Cell();
    }

    val str = cell->to_string();
    val len = str.length();

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char value[EVP_MAX_MD_SIZE];
    unsigned int value_len;
    EVP_DigestInit_ex(ctx, hash, null);
    EVP_DigestUpdate(ctx, str.c_str(), len);
    EVP_DigestFinal_ex(ctx, value, &value_len);
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (var i = 0; i < value_len; ++i) {
        ss << std::hex << setw(2) << std::setfill('0') << (int) value[i];
    }

    return new Cell(ss.str());
}

static Cell *tk_from_base64(Row *row) {
    check_arg_nums(row, "from_base64", 1);

    val cell = row->at(0);
    if (check_null(cell) || cell->type != D_STRING) {
        return new Cell();
    }

    val len = (int) cell->text.length();
    char out[len];

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO *bio = BIO_new_mem_buf(cell->text.c_str(), len);
    bio = BIO_push(b64, bio);
    var size = BIO_read(bio, &out[0], len);
    out[size] = '\0';
    BIO_free_all(bio);

    std::stringstream ss;
    size = 0;
    while (out[size]) {
        ss << std::hex << setw(2) << setfill('0') << (int) out[size++];
    }

    val str = ss.str();
    if (str.empty()) {
        return new Cell();
    }

    return new Cell("0x" + str);
}

static Cell *tk_md5(Row *row) {
    return hash_aux(row, "md5", EVP_md5());
}

static Cell *tk_serial(Row *row) {
    check_arg_nums(row, "uuid_short", 0, 1);

    var len = 8;
    if (row->size() == 1) {
        val cell = row->at(0);
        if (cell->type != D_INTEGER || cell->number < 1) {
            return new Cell();
        } else {
            len = (int) cell->number;
        }
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 0x10);

    var str = String();
    str.reserve(len);
    for (var i = 0; i < len; i++) {
        val rand = (int) floor(dis(gen));
        if (rand > 9) {
            str += (char) (rand - 10 + 'a');
        } else {
            str += (char) (rand + '0');
        }
    }

    return new Cell(str);
}

static Cell *tk_sha(Row *row) {
    return hash_aux(row, "sha", EVP_sha1());
}

static Cell *tk_sha1(Row *row) {
    return hash_aux(row, "sha1", EVP_sha1());
}

static Cell *tk_sha2(Row *row) {
    if (row->size() == 1) {
        return hash_aux(row, "sha2", EVP_sha256());
    }

    check_arg_nums(row, "sha2", 2);

    val cell = row->at(1);
    if (check_null(cell) || cell->type != D_INTEGER) {
        return new Cell();
    }

    var hash_len = (int) cell->number;
    switch (hash_len) {
        case 224:
            return hash_aux(row, "sha2", EVP_sha224(), 2);
        case 0:
        case 256:
            return hash_aux(row, "sha2", EVP_sha256(), 2);
        case 384:
            return hash_aux(row, "sha2", EVP_sha384(), 2);
        case 512:
            return hash_aux(row, "sha2", EVP_sha512(), 2);
        default:
            return new Cell();
    }
}

static Cell *tk_sha224(Row *row) {
    return hash_aux(row, "sha224", EVP_sha224());
}

static Cell *tk_sha256(Row *row) {
    return hash_aux(row, "sha256", EVP_sha256());
}

static Cell *tk_sha384(Row *row) {
    return hash_aux(row, "sha384", EVP_sha384());
}

static Cell *tk_sha512(Row *row) {
    return hash_aux(row, "sha512", EVP_sha512());
}

static Cell *tk_to_base64(Row *row) {
    check_arg_nums(row, "from_base64", 1);

    val cell = row->at(0);
    if (check_null(cell)) {
        return new Cell();
    }

    val str = cell->to_string();
    val len = (int) str.length();

    BUF_MEM *ptr = null;
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO *bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, str.c_str(), len);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &ptr);
    String out(ptr->data, ptr->length);
    BIO_free_all(bio);

    return new Cell(out);
}

static Cell *tk_uuid(Row *row) {
    check_arg_nums(row, "uuid", 0);

    uuid_t uuid;
    char uuid_str[37];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);

    return new Cell(String(uuid_str));
}

void init_hash_funcs() {
    ADD_NORMAL(from_base64, D_STRING);
    ADD_NORMAL(md5, D_STRING);
    ADD_NORMAL(serial, D_STRING);
    ADD_NORMAL(sha, D_STRING);
    ADD_NORMAL(sha1, D_STRING);
    ADD_NORMAL(sha2, D_STRING);
    ADD_NORMAL(sha224, D_STRING);
    ADD_NORMAL(sha256, D_STRING);
    ADD_NORMAL(sha384, D_STRING);
    ADD_NORMAL(sha512, D_STRING);
    ADD_NORMAL(to_base64, D_STRING);
    ADD_NORMAL(uuid, D_STRING);
}
