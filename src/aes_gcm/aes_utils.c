#include "aes_utils.h"

/*************  ✨ Windsurf Command ⭐  *************/
/**
 * @brief Decompose a GCM packet into its components.
 *
 * A GCM packet consists of [IV | CIPHERTEXT | TAG].
 *
 * @param input input buffer containing the packet
 * @param input_len length of the packet in bytes
 * @param out_packet output struct to hold the decomposed packet
 * @param iv_len length of the IV in bytes
 * @param tag_len length of the TAG in bytes
 *
/*******  fc2cb839-84ee-4f9c-9bcc-072d34ebbf57  *******/
bool parse_aes_gcm_packet(const uint8_t *input, size_t input_len, AesGcmPacket *out_packet,
                          size_t iv_len, size_t tag_len)
{
    if (input_len < iv_len + tag_len)
        return false;

    out_packet->iv = input;
    out_packet->iv_len = iv_len;

    out_packet->ciphertext = input + iv_len;
    out_packet->ciphertext_len = input_len - iv_len - tag_len;

    out_packet->tag = input + input_len - tag_len;
    out_packet->tag_len = tag_len;

    return true;
}
