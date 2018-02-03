#pragma once

enum class cheatcoin_field_type
{
    CHEATCOIN_FIELD_NONCE,
    CHEATCOIN_FIELD_HEAD,
    CHEATCOIN_FIELD_IN,
    CHEATCOIN_FIELD_OUT,
    CHEATCOIN_FIELD_SIGN_IN,
    CHEATCOIN_FIELD_SIGN_OUT,
    CHEATCOIN_FIELD_PUBLIC_KEY_0,
    CHEATCOIN_FIELD_PUBLIC_KEY_1,
};


class XBlock
{
public:
    XBlock();
    ~XBlock();

    void GenerateBlock();
};

