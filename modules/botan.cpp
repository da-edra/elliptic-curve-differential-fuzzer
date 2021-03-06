// Copyright (c) 2018 Catena cyber
// Author Philippe Antoine <p.antoine@catenacyber.fr>


#include "../fuzz_ec.h"
#include <botan/ecdsa.h>
#include <botan/oids.h>
#include <botan/chacha_rng.h>
#include <stdlib.h>

#define BYTECEIL(numbits) (((numbits) + 7) >> 3)

static const Botan::OID eccurvetypeFromTlsId(uint16_t tlsid) {
    switch (tlsid) {
        case 18:
            return Botan::OIDS::str2oid_or_empty("secp192k1");
        case 19:
            return Botan::OIDS::str2oid_or_empty("secp192r1");
        case 20:
            return Botan::OIDS::str2oid_or_empty("secp224k1");
        case 21:
            return Botan::OIDS::str2oid_or_empty("secp224r1");
        case 22:
            return Botan::OIDS::str2oid_or_empty("secp256k1");
        case 23:
            return Botan::OIDS::str2oid_or_empty("secp256r1");
        case 24:
            return Botan::OIDS::str2oid_or_empty("secp384r1");
        case 25:
            return Botan::OIDS::str2oid_or_empty("secp521r1");
        case 26:
            return Botan::OIDS::str2oid_or_empty("brainpool256r1");
        case 27:
            return Botan::OIDS::str2oid_or_empty("brainpool384r1");
        case 28:
            return Botan::OIDS::str2oid_or_empty("brainpool512r1");
    }
    return Botan::OIDS::str2oid_or_empty("");
}

static void botan_to_ecfuzzer(Botan::PointGFp pointZ, fuzzec_output_t * output, size_t index, size_t byteLen) {
    if (pointZ.is_zero()) {
        output->pointSizes[index] = 1;
        output->points[index][0] = 0;
    } else {
        output->pointSizes[index] = 1 + 2 * byteLen;
        output->points[index][0] = 4;
        pointZ.get_affine_x().binary_encode(output->points[index]+1, byteLen);
        pointZ.get_affine_y().binary_encode(output->points[index]+1+byteLen, byteLen);
    }
}

extern "C" void fuzzec_botan_process(fuzzec_input_t * input, fuzzec_output_t * output) {

    //initialize
    const Botan::OID oid = eccurvetypeFromTlsId(input->tls_id);
    if (oid.to_string().length() == 0) {
        output->errorCode = FUZZEC_ERROR_UNSUPPORTED;
        return;
    }
    Botan::EC_Group group(oid);
    Botan::BigInt scalar(input->bignum, input->bignumSize);
    Botan::BigInt coordx(input->coordx, input->coordSize);
    Botan::BigInt coordy(input->coordy, input->coordSize);
    Botan::PointGFp point1 = group.point(coordx, coordy);

    //elliptic curve computations
    Botan::PointGFp point2 = point1 * scalar;

    //format output
    botan_to_ecfuzzer(point2, output, 0, BYTECEIL(input->groupBitLen));

    output->errorCode = FUZZEC_ERROR_NONE;

    return;
}

extern "C" void fuzzec_botanblind_process(fuzzec_input_t * input, fuzzec_output_t * output) {

    //initialize
    const Botan::OID oid = eccurvetypeFromTlsId(input->tls_id);
    if (oid.to_string().length() == 0) {
        output->errorCode = FUZZEC_ERROR_UNSUPPORTED;
        return;
    }
    Botan::EC_Group group(oid);
    Botan::BigInt scalar(input->bignum, input->bignumSize);
    Botan::BigInt coordx(input->coordx, input->coordSize);
    Botan::BigInt coordy(input->coordy, input->coordSize);
    Botan::PointGFp point1 = group.point(coordx, coordy);
    static std::vector<Botan::BigInt> ws(Botan::PointGFp::WORKSPACE_SIZE);
    static Botan::ChaCha_RNG rng(Botan::secure_vector<uint8_t>(32));
    //elliptic curve computations
    Botan::PointGFp point2 = group.blinded_var_point_multiply(point1, scalar, rng, ws);

    //format output
    botan_to_ecfuzzer(point2, output, 0, BYTECEIL(input->groupBitLen));

    output->errorCode = FUZZEC_ERROR_NONE;

    return;
}

extern "C" void fuzzec_botan_add(fuzzec_input_t * input, fuzzec_output_t * output) {
    const Botan::OID oid = eccurvetypeFromTlsId(input->tls_id);
    if (oid.to_string().length() == 0) {
        output->errorCode = FUZZEC_ERROR_UNSUPPORTED;
        return;
    }
    Botan::EC_Group group(oid);
    Botan::BigInt coordx(input->coordx, input->coordSize);
    Botan::BigInt coordy(input->coordy, input->coordSize);
    Botan::PointGFp point1 = group.point(coordx, coordy);
    Botan::BigInt coord2x(input->coord2x, input->coordSize);
    Botan::BigInt coord2y(input->coord2y, input->coordSize);
    Botan::PointGFp point2 = group.point(coord2x, coord2y);

    //elliptic curve computations
    Botan::PointGFp point3 = point1 + point2;

    //format output
    botan_to_ecfuzzer(point3, output, 0, BYTECEIL(input->groupBitLen));

    output->errorCode = FUZZEC_ERROR_NONE;

    return;
}
