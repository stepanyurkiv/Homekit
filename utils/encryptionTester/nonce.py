HAP_CRYPTO_TLS_NONCE_LEN=12

def _pad_tls_nonce(nonce, total_len=HAP_CRYPTO_TLS_NONCE_LEN):
    """ Pads a nonce with zeroes so that total_len is reached. """
    return nonce.rjust(total_len, b"\x00")


nonce = _pad_tls_nonce(b"\x01")
print "nonce: {}".format(nonce.encode('hex_codec'))
