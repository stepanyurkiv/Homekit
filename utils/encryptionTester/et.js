'use strict';

var _sodium = require('sodium');
var _number = require('./number');

function getPadding(buffer, blockSize) {
    return buffer.length % blockSize === 0 ? Buffer.alloc(0) : Buffer.alloc(blockSize - buffer.length % blockSize);
}

function computePoly1305(cipherText, AAD, nonce, key) {
    if (AAD == null) {
        AAD = Buffer.alloc(0);
    }


    console.log(">>>>>>>>>>>>>>> computePoly1305:");
    console.log("AAD: " + AAD.toString('hex'));
    console.log("AAD length: " + AAD.length);
    // console.log("getPadding(AAD, 16): " + (16 - AAD.length % 16));
    console.log("cipherText: " + cipherText.toString('hex'));
    console.log("cipherText length: " +  cipherText.length);
    // console.log("getPadding(cipherText, 16): " + (16 - cipherText.length % 16));

    const msg = Buffer.concat([AAD, getPadding(AAD, 16), cipherText, getPadding(cipherText, 16), (0, _number.UInt53toBufferLE)(AAD.length), (0, _number.UInt53toBufferLE)(cipherText.length)]);

    console.log("msg:");
    console.log(msg.toString('hex'));


    console.log("nonce:");
    console.log(nonce.toString('hex'));

    console.log("key:");
    console.log(key.toString('hex'));



    const polyKey = _sodium.api.crypto_stream_chacha20(32, nonce, key);

    console.log("polyKey:");
    console.log(polyKey.toString('hex'));


    const computed_hmac = _sodium.api.crypto_onetimeauth(msg, polyKey);
    polyKey.fill(0);


    console.log("computed_hmac:");
    console.log(computed_hmac.toString('hex'));


    console.log(">>>>>>>>>>>>>>> end computePoly1305:");
    return computed_hmac;
}

function computePoly1305_IETF(cipherText, AAD, nonce, key) {
    if (AAD == null) {
        AAD = Buffer.alloc(0);
    }


    console.log(">>>>>>>>>>>>>>> computePoly1305_IETF:");
    console.log("AAD: " + AAD.toString('hex'));
    console.log("AAD length: " + AAD.length);
    // console.log("getPadding(AAD, 16): " + (16 - AAD.length % 16));
    console.log("cipherText: " + cipherText.toString('hex'));
    console.log("cipherText length: " +  cipherText.length);
    // console.log("getPadding(cipherText, 16): " + (16 - cipherText.length % 16));

    const msg = Buffer.concat([AAD, getPadding(AAD, 16), cipherText, getPadding(cipherText, 16), (0, _number.UInt53toBufferLE)(AAD.length), (0, _number.UInt53toBufferLE)(cipherText.length)]);

    console.log("msg:");
    console.log(msg.toString('hex'));


    console.log("nonce:");
    console.log(nonce.toString('hex'));

    console.log("key:");
    console.log(key.toString('hex'));



    const polyKey = _sodium.api.crypto_stream_chacha20_ietf(32, nonce, key);

    console.log("polyKey:");
    console.log(polyKey.toString('hex'));


    const computed_hmac = _sodium.api.crypto_onetimeauth(msg, polyKey);
    polyKey.fill(0);


    console.log("computed_hmac:");
    console.log(computed_hmac.toString('hex'));


    console.log(">>>>>>>>>>>>>>> end computePoly1305_IETF:");
    return computed_hmac;
}

// i'd really prefer for this to be a direct call to
// Sodium.crypto_aead_chacha20poly1305_decrypt()
// but unfortunately the way it constructs the message to
// calculate the HMAC is not compatible with homekit
// (long story short, it uses [ AAD, AAD.length, CipherText, CipherText.length ]
// whereas homekit expects [ AAD, CipherText, AAD.length, CipherText.length ]
function verifyAndDecrypt(cipherText, mac, AAD, nonce, key) {
    
    console.log(">>>>>>>>>>>>>>> verifyAndDecrypt:");
    console.log("cipherText:");
    console.log(cipherText.toString('hex'));

    console.log("mac:");
    console.log(mac.toString('hex'));

    console.log("AAD:");
    if (AAD != null)
        console.log(AAD.toString('hex'));

    console.log("nonce:");
    console.log(nonce.toString('hex'));

    console.log("key:");
    console.log(key.toString('hex'));

    const matches = _sodium.api.crypto_verify_16(mac, computePoly1305(cipherText, AAD, nonce, key));

    if (matches === 0) {
        const decrypted = _sodium.api.crypto_stream_chacha20_xor_ic(cipherText, nonce, 1, key);
        console.log("decrypted: " + decrypted.toString('hex'));

        console.log(">>>>>>>>>>>>>>> end verifyAndDecrypt:");
        return decrypted;

    } else {
        console.log("ERROR");
        console.log(">>>>>>>>>>>>>>> end verifyAndDecrypt:");
    }

    
    return null;
}



// i'd really prefer for this to be a direct call to
// Sodium.crypto_aead_chacha20poly1305_decrypt()
// but unfortunately the way it constructs the message to
// calculate the HMAC is not compatible with homekit
// (long story short, it uses [ AAD, AAD.length, CipherText, CipherText.length ]
// whereas homekit expects [ AAD, CipherText, AAD.length, CipherText.length ]
function verifyAndDecrypt_IETF(cipherText, mac, AAD, nonce, key) {
    
    console.log(">>>>>>>>>>>>>>> verifyAndDecrypt_IETF:");
    console.log("cipherText:");
    console.log(cipherText.toString('hex'));

    console.log("mac:");
    console.log(mac.toString('hex'));

    console.log("AAD:");
    if (AAD != null)
        console.log(AAD.toString('hex'));

    console.log("nonce:");
    console.log(nonce.toString('hex'));

    console.log("key:");
    console.log(key.toString('hex'));

    const matches = _sodium.api.crypto_verify_16(mac, computePoly1305_IETF(cipherText, AAD, nonce, key));

    if (matches === 0) {
        const decrypted = _sodium.api.crypto_stream_chacha20_ietf_xor_ic(cipherText, nonce, 1, key);
        console.log("decrypted: " + decrypted.toString('hex'));

        console.log(">>>>>>>>>>>>>>> end verifyAndDecrypt_IETF:");
        return decrypted;

    } else {
        console.log("ERROR");
        console.log(">>>>>>>>>>>>>>> end verifyAndDecrypt_IETF:");
    }

    
    return null;
}


// See above about calling directly into libsodium.
function encryptAndSeal(plainText, AAD, nonce, key) {

    const cipherText = _sodium.api.crypto_stream_chacha20_xor_ic(plainText, nonce, 1, key);

    const hmac =
        computePoly1305(cipherText, AAD, nonce, key);

    return [ cipherText, hmac ];
}


// See above about calling directly into libsodium.
function encryptAndSeal_IETF(plainText, AAD, nonce, key) {
    console.log(">>>>>>>>>>>>>>> encryptAndSeal_IETF:");
    const cipherText =
        _sodium.api.crypto_stream_chacha20_ietf_xor_ic(plainText, nonce, 1, key);

    const hmac =
        computePoly1305_IETF(cipherText, AAD, nonce, key);

    console.log(">>>>>>>>>>>>>>> end encryptAndSeal_IETF:");
    return [ cipherText, hmac ];
}





const cipherText = new Buffer("6785ED52858ECCA6BA3708A7E563358D82841AB34557F6DC1D196D319FE0AE0A0653B7D7963BC656C2408585F1729B7D0A62F48051E51D30A7AC77793B770AD3FE6900A36B", "hex");
const mac = new Buffer("B202BD82544884D9E492FD8F808195C8", "hex");
const AAD = new Buffer("4500", "hex");
//const nonce = new Buffer("0100000000000000", "hex");
const nonce_IETF = new Buffer("000000000000000000000000", "hex");
const decryptKey = new Buffer("B91816CF0E38F24F5E92922F481E9213C82FDADD630DF82FFBBB3E68840CDD5D", "hex");



const plainText = new Buffer("474554202f6163636573736f7269657320485454502f312e310d0a486f73743a203139322e3136382e3137382e3132333a35313632380d0a417574686f72697a6174696f6e3a20756e646566696e65640d0a0d0a", "hex");
const encryptNonce = new Buffer("0000000000000000", "hex");
const encryptNonce_IETF = new Buffer("000000000000000000000000", "hex");
const encryptAAD = new Buffer("5400", "hex");
const encryptKey = new Buffer("341ca8dbef41baf2bb8b28805f75f7c18804d9aaa98006f6d775f45d4fe64f89", "hex");



// console.log("======================================================================================");
// console.log("");
// let [ encrypted_I, hmac_I ] = encryptAndSeal_IETF(plainText, encryptAAD, encryptNonce_IETF, encryptKey);
// console.log("IETF encrypted: " + encrypted_I.toString('hex'));
// console.log(encrypted_I.toString());


// console.log("");
// let [ encrypted_n, hmac_n ] = encryptAndSeal(plainText, encryptAAD, encryptNonce, encryptKey);
// console.log("encrypted: " + encrypted_n.toString('hex'));
// console.log(encrypted_n.toString());



console.log("======================================================================================");
// console.log("");
// var hmac = computePoly1305(cipherText, AAD, nonce, decryptKey);
// console.log("hmac: " + hmac.toString('hex'));


console.log("");
var hmac_IETF = computePoly1305_IETF(cipherText, AAD, nonce_IETF, decryptKey);
console.log("IETF hmac: " + hmac_IETF.toString('hex'));






console.log("======================================================================================");
console.log("");
var decrypted_IETF = verifyAndDecrypt_IETF(cipherText, mac, AAD, nonce_IETF, decryptKey);
console.log("IETF decrypted: " + decrypted_IETF.toString('hex'));
console.log("");
console.log(decrypted_IETF.toString());


// console.log("");
// var decrypted = verifyAndDecrypt(cipherText, mac, AAD, nonce, decryptKey);
// console.log("decrypted: " + decrypted.toString('hex'));
// console.log("");
// console.log(decrypted.toString());

