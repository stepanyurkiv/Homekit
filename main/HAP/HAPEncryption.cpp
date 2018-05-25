//
// HAPEncryption.cpp
// Homekit
//
//  Created on: 08.05.2018
//      Author: michael
//

#include "HAPEncryption.hpp"
#include "HAPLogger.hpp"
#include "HAPHelper.hpp"
#include "HAPGlobals.hpp"

int HAPEncryption::pad(size_t *padded_buflen_p, uint8_t *msg, 
        const uint8_t *buf, size_t unpadded_buflen, size_t blocksize, 
        size_t max_msglen, bool zeroPadded )
{
    unsigned char          *tail;
    size_t                  i;
    size_t                  xpadlen;
    size_t                  xpadded_len;
    volatile unsigned char  mask;
    unsigned char           barrier_mask;

    if (blocksize <= 0U) {
        return -1;
    }
    xpadlen = blocksize - 1U;

    if ((blocksize & (blocksize - 1U)) == 0U) {
        xpadlen -= unpadded_buflen & (blocksize - 1U);
    } else {
        xpadlen -= unpadded_buflen % blocksize;
    }
    if ((size_t) SIZE_MAX - unpadded_buflen <= xpadlen) {
        //ESP_LOGE("Missuse");
        return -1;
    }
    xpadded_len = unpadded_buflen + xpadlen;
    

    if (max_msglen != 0) {
        if (xpadded_len >= max_msglen) {
            return -1;
        } 
    }
   
    
    if (padded_buflen_p != NULL) {
        *padded_buflen_p = xpadded_len + 1U;
    }
    
    if (msg == NULL) {
        return -2;
    }


    memcpy(msg, buf, unpadded_buflen);
    tail = &msg[xpadded_len];

    mask = 0U;
    for (i = 0; i < blocksize; i++) {
        barrier_mask = (unsigned char) (((i ^ xpadlen) - 1U) >> 8);

        if (zeroPadded) {
            tail[-i] = (tail[-i] & mask) | (0x00 & barrier_mask);
        } else {
            tail[-i] = (tail[-i] & mask) | (0x80 & barrier_mask);    
        }
        
        mask |= barrier_mask;
    }
    return 0;
}


int HAPEncryption::begin(){
    int result = sodium_init();
    if (result < 0) {
        /* panic! the library couldn't be initialized, it is not safe to use */
        LogE("[ERROR] Sodium couldn't be initialized!", true);
        return result;
    }
    return result;
}

int HAPEncryption::unpad(size_t *unpadded_buflen_p, const unsigned char *buf,
             size_t padded_buflen, size_t blocksize)
{
    const unsigned char *tail;
    unsigned char        acc = 0U;
    unsigned char        c;
    unsigned char        valid = 0U;
    volatile size_t      pad_len = 0U;
    size_t               i;
    size_t               is_barrier;

    if (padded_buflen < blocksize || blocksize <= 0U) {
        return -1;
    }
    tail = &buf[padded_buflen - 1U];

    for (i = 0U; i < blocksize; i++) {
        c = tail[-i];
        is_barrier =
            (( (acc - 1U) & (pad_len - 1U) & ((c ^ 0x80) - 1U) ) >> 8) & 1U;
        acc |= c;
        pad_len |= i & (1U + ~is_barrier);
        valid |= (unsigned char) is_barrier;
    }
    *unpadded_buflen_p = padded_buflen - 1U - pad_len;

    return (int) (valid - 1U);
}




// function computePoly1305(cipherText, AAD, nonce, key) {
//     if (AAD == null) {
//         AAD = Buffer.alloc(0);
//     }

//     const msg =
//         Buffer.concat([
//             AAD,
//             getPadding(AAD, 16),
//             cipherText,
//             getPadding(cipherText, 16),
//             UInt53toBufferLE(AAD.length),
//             UInt53toBufferLE(cipherText.length)
//         ])

//     const polyKey = Sodium.crypto_stream_chacha20(32, nonce, key);
//     const computed_hmac = Sodium.crypto_onetimeauth(msg, polyKey);
//     polyKey.fill(0);

//     return computed_hmac;
// }

int HAPEncryption::computePoly1305(uint8_t* hmac, uint8_t* cipherText, 
            size_t cipherTextLength, uint8_t* AAD, uint8_t *nonce, 
            uint8_t *key) {


    LogD("Handle computePoly1305 ...", true);

    begin();

    if (AAD == nullptr) {
        //AAD = Buffer.alloc(0);
    }

    size_t block_size = 16;

    int paddedCipherLength  = paddedLength(cipherTextLength, block_size);
    int paddedAADLength     = paddedLength(HAP_AAD_LENGTH, block_size);

    int paddedAADLengthNum       = paddedLength(HAP_AAD_LENGTH, 8);
    int paddedCipherLengthNum    = paddedLength( HAPHelper::numDigits(paddedCipherLength), 8);

    int paddedLength        = paddedAADLength 
                            + paddedCipherLength 
                            + paddedAADLengthNum 
                            + paddedCipherLengthNum;


    uint8_t msg[paddedLength] = { 0, };
    
#if HAP_ENCRYPTION_DEBUG    
    Serial.printf("paddedLength: %d\n", paddedLength);
#endif
    
    int aad_len = HAP_AAD_LENGTH;

    memcpy(msg, AAD, aad_len);
    memcpy(msg + paddedAADLength, cipherText, cipherTextLength);
    memcpy(msg + paddedAADLength + paddedCipherLength, &aad_len, 1);
    memcpy(msg + paddedAADLength + paddedCipherLength + paddedAADLengthNum, &cipherTextLength, HAPHelper::numDigits(cipherTextLength) );


#if HAP_ENCRYPTION_DEBUG
    Serial.printf("msg: %d = %d\n", paddedLength, sizeof msg);
    HAPHelper::arrayPrint(msg, paddedLength);
#endif

    uint8_t polyKey[HAP_ENCRYPTION_KEY_SIZE] = { 0, };
    if ( crypto_stream_chacha20_ietf(polyKey, HAP_ENCRYPTION_KEY_SIZE, nonce, key) != 0 ) {
        LogE("[ERROR] Generating polyKey failed!", true);
        return -1;
    }

#if HAP_ENCRYPTION_DEBUG    
    Serial.println("polyKey: ");
    HAPHelper::arrayPrint(polyKey, HAP_ENCRYPTION_KEY_SIZE);
#endif

    // uint8_t hmac[crypto_onetimeauth_BYTES];

    if ( crypto_onetimeauth(hmac, msg, paddedLength, polyKey) != 0 ) {
        LogE("[ERROR] Generating crypto_onetimeauth!", true);
        return -1;
    }
    
#if HAP_ENCRYPTION_DEBUG    
    Serial.println("generated hmac:");
    HAPHelper::arrayPrint(hmac, crypto_onetimeauth_BYTES);
    //Serial.printf("msg: %d\n", sizeof msg);
    //HAPHelper::arrayPrint(msg, sizeof msg);
#endif

    LogD("OK", true);
    return 0;
}



// i'd really prefer for this to be a direct call to
// Sodium.crypto_aead_chacha20poly1305_decrypt()
// but unfortunately the way it constructs the message to
// calculate the HMAC is not compatible with homekit
// (long story short, it uses [ AAD, AAD.length, CipherText, CipherText.length ]
// whereas homekit expects [ AAD, CipherText, AAD.length, CipherText.length ]
// 
// function verifyAndDecrypt(cipherText, mac, AAD, nonce, key) {
//     const matches =
//         Sodium.crypto_verify_16(
//             mac,
//             computePoly1305(cipherText, AAD, nonce, key)
//         );
//
//     if (matches === 0) {
//         return Sodium
//             .crypto_stream_chacha20_xor_ic(cipherText, nonce, 1, key);
//     }
//
//     return null;
// }

int HAPEncryption::verifyAndDecrypt(uint8_t *decrypted, uint8_t cipherText[], 
            uint16_t length, uint8_t mac[], uint8_t aad[], 
            int decryptCount, uint8_t key[]){


    LogD("Handle verifyAndDecrypt ...", false);

    begin();

    if ( length > 1024 + HAP_AAD_LENGTH + HAP_ENCRYPTION_HMAC_SIZE ){
        LogE("NOWNOW!!", true);
    }
    
    // nonce
    // the nonce is 12 byte long
    // 
    // the first 4 bytes are always 
    //      
    //      00 00 00 00 
    // 
    // the nonce is incremented each time a decryption with the same
    // encryption key is performed
    // 
    // !!! the counter starts at the last bits (10, 11) (!started by 0!)
    // 
    // thus the remaining 8 bytes will look as follows
    // 
    //      | 00 00 00 00 | 00 00 00 00 00 00 00 01 
    //          first 4       remaining 8 bytes   ^
    //                                            | -> this will be incremented each time this function is called
    //  
    //                                               
    uint8_t nonce[HAP_ENCRYPTION_NONCE_SIZE] = { 0, };            
    nonce[4] = decryptCount % 256;
    nonce[5] = decryptCount++ / 256;
    

#if HAP_ENCRYPTION_DEBUG    
    LogD("decryptCount: " + String(decryptCount), true);

    Serial.println("nonce:");
    HAPHelper::arrayPrint(nonce, HAP_ENCRYPTION_NONCE_SIZE);

    Serial.printf("cipherText: %d\n", length);
    HAPHelper::arrayPrint(cipherText, length);

    Serial.println("AAD:");
    HAPHelper::arrayPrint(aad, HAP_AAD_LENGTH);

    Serial.println("mac:");
    HAPHelper::arrayPrint(mac, HAP_ENCRYPTION_HMAC_SIZE);

    Serial.println("key:");
    HAPHelper::arrayPrint(key, HAP_ENCRYPTION_KEY_SIZE);

#endif

    // 16 bytes long
    uint8_t hmac[HAP_ENCRYPTION_HMAC_SIZE] = {0,};

    if ( computePoly1305(hmac, cipherText, length, aad, nonce, key) != 0 ) {
        LogE("[ERROR] computePoly1305 failed!", true);
        return -1;
    }

#if HAP_ENCRYPTION_DEBUG  
    Serial.println("computed hmac:");
    HAPHelper::arrayPrint(hmac, HAP_ENCRYPTION_HMAC_SIZE);
#endif

    if ( crypto_verify_16(mac, hmac) != 0 ) {
#if !HAP_ENCRYPTION_SUPPRESS_WARNINGS      
        LogW("[WARNING] crypto_verify_16 failed! - Trying to decrypt it anyway ...", true);
#endif

#if HAP_ENCRYPTION_EXIT_ON_FAILURE      
        return -1;
#endif
    }

    
    // 
    // The output from the AEAD is twofold:
    //   -  A ciphertext of the same length as the plaintext.
    //   -  A 128-bit tag, which is the output of the Poly1305 function.
    // 
    //uint8_t decrypted[length];    
    if ( crypto_stream_chacha20_ietf_xor_ic(decrypted, cipherText, length, nonce, 1, key) != 0 ) {
        LogE("[ERROR] crypto_stream_chacha20_xor_ic failed!", true);
        return -1;
    }

#if HAP_ENCRYPTION_DEBUG  
    Serial.printf("decrypted: %d\n", length );
    HAPHelper::arrayPrint(decrypted, length);
    Serial.println((char*) decrypted);
#endif


    LogD("OK", true);
    return 0;
}