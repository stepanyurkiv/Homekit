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



const cipherText = new Buffer("ced3858c3bec475165c788822031425bd94d3d818082e7a101bdc09be04acb86394f17bb277ac660a0b1ef1aa4747a853f8894f2141c78f8c6e58841e1f0b5f8db8b0c9b166321dabea99c7e8108fb9fdcd977c3", "hex");
const mac = new Buffer("2f424a51034faf395a6b608055da21ad", "hex");
const AAD = new Buffer("5400", "hex");
const nonce = new Buffer("0000000000000000", "hex");
const key = new Buffer("750c668c78c70e5d9c341019e969052bf15424976ddd28b7d18a3de6b8510d0e", "hex");


var encrypted = verifyAndDecrypt(cipherText, mac, AAD, nonce, key);
console.log(encrypted.toString('hex'));

