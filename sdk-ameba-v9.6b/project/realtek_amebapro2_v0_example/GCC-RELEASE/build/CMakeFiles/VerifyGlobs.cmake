# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.22
cmake_policy(SET CMP0009 NEW)

# MBEDTLS_SRC at application/application.cmake:443 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/*.c")
set(OLD_GLOB
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/aes.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/aes_alt.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/aesni.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/arc4.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/aria.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/asn1parse.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/asn1write.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/base64.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/bignum.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/blowfish.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/camellia.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ccm.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/certs.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/chacha20.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/chachapoly.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/cipher.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/cipher_wrap.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/cmac.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/constant_time.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ctr_drbg.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/debug.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/des.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/dhm.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ecdh.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ecdsa.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ecjpake.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ecp.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ecp_curves.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/entropy.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/entropy_alt.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/entropy_poll.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/error.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/gcm.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/havege.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/hkdf.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/hmac_drbg.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/md.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/md2.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/md4.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/md5.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/memory_buffer_alloc.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/mps_reader.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/mps_trace.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/net_sockets.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/nist_kw.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/oid.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/padlock.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pem.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pk.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pk_wrap.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pkcs11.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pkcs12.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pkcs5.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pkparse.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/pkwrite.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/platform.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/platform_util.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/poly1305.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_aead.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_cipher.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_client.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_driver_wrappers.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_ecp.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_hash.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_mac.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_rsa.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_se.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_slot_management.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_crypto_storage.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/psa_its_file.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ripemd160.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/rsa.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/rsa_internal.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/sha1.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/sha256.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/sha512.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_cache.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_ciphersuites.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_cli.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_cookie.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_msg.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_srv.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_ticket.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_tls.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/ssl_tls13_keys.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/threading.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/timing.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/version.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/version_features.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509_create.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509_crl.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509_crt.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509_csr.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509write_crt.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/x509write_csr.c"
  "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/application/../../../../component/ssl/mbedtls-2.28.1/library/xtea.c"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/build/CMakeFiles/cmake.verify_globs")
endif()