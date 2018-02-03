#include "XBlock.h"
#include "XStorage.h"

XBlock::XBlock()
{
}

XBlock::~XBlock()
{
}

int cheatcoin_create_block()
{
    struct cheatcoin_block b[2];
    int i, j, res, res0, mining, defkeynum, keysnum[CHEATCOIN_BLOCK_FIELDS], nkeys, outsigkeyind = -1;
    struct cheatcoin_public_key *defkey = cheatcoin_wallet_default_key(&defkeynum), *key;
    cheatcoin_hash_t hash, min_hash;
    struct block_internal *ref, *pretop = pretop_block(), *pretop_new;
    res0 = 1 + (outsigkeyind < 0 ? 2 : 0);
    if(res0 > CHEATCOIN_BLOCK_FIELDS) return -1;
    cheatcoin_time_t send_time = get_timestamp(), mining = 0;
    res0 += mining;

    res = res0;
    memset(b, 0, sizeof(struct cheatcoin_block)); i = 1;
    b[0].field[0].type = cheatcoin_field_type::CHEATCOIN_FIELD_HEAD;
    b[0].field[0].time = send_time;
    b[0].field[0].amount = 0;

    if(res < CHEATCOIN_BLOCK_FIELDS && mining && pretop && pretop->time < send_time)
    {
        log_block("Mintop", pretop->hash, pretop->time, pretop->storage_pos);
        setfld(CHEATCOIN_FIELD_OUT, pretop->hash, cheatcoin_hashlow_t); res++;
    }
    for(ref = noref_first; ref && res < CHEATCOIN_BLOCK_FIELDS; ref = ref->ref) if(ref->time < send_time)
    {
        setfld(CHEATCOIN_FIELD_OUT, ref->hash, cheatcoin_hashlow_t); res++;
    }

    if(outsigkeyind < 0) b[0].field[0].type |= (uint64_t)(CHEATCOIN_FIELD_SIGN_OUT * 0x11) << ((i + j) * 4);
    if(outsigkeyind < 0)
    {
        hash_for_signature(b, defkey, hash);
        cheatcoin_sign(defkey->key, hash, b[0].field[i].data, b[0].field[i + 1].data);
    }

    cheatcoin_hash(b, sizeof(struct cheatcoin_block), min_hash);
    b[0].field[0].transport_header = 1;
    log_block("Create", min_hash, b[0].field[0].time, 1);
    res = cheatcoin_add_block(b);
    if(res > 0)
    {
        cheatcoin_send_new_block(b);
        res = 0;
    }
    return res;
}

void XBlock::GenerateBlock()
{

}