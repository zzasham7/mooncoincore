// Copyright (c) 2017-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_check.h>

#include <primitives/transaction.h>
#include <consensus/validation.h>
#include <uint256.h>
#include <algorithm>
#include <array>

static bool IsLegacyMooncoinCoinbaseLengthException(const CTransaction& tx)
{
    // Legacy Mooncoin chain contains a small number of historical coinbases
    // with scriptSig length > 100. Keep this list narrow and hash-anchored.
    static const std::array<uint256, 4> kAllowedTxids{
        uint256S("0xb3aaaed7565d0594128fffa5f5dee01df8eb24de4245365cb8df7ad0c6e93266"), // genesis
        uint256S("0xc70e7855948d77d62e64cbf9ceb3e20cb8424b0b3adebe96f6c106579429378f"),
        uint256S("0x0b0fa656945ec52e4a1014bc263923932c618c46bffb8c628f08911fdd4a8070"),
        uint256S("0x9f2761c643977c328f83a2cb9e7acb17cbf9ace04cef6fad3ba7de410e7c5f3e"),
    };
    const uint256 txid = tx.GetHash();
    return std::any_of(kAllowedTxids.begin(), kAllowedTxids.end(),
        [&txid](const uint256& allowed) { return allowed == txid; });
}

bool CheckTransaction(const CTransaction& tx, TxValidationState& state)
{
    // Basic checks that don't depend on any context
    if (!tx.IsMWEBOnly()) {
        if (tx.vin.empty())
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vin-empty");
        if (tx.vout.empty())
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-empty");
    }
	
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(tx, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS | SERIALIZE_NO_MWEB) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-oversize");

    // Check for negative or overflow output values (see CVE-2010-5139)
    CAmount nValueOut = 0;
    for (const auto& txout : tx.vout)
    {
        if (txout.nValue < 0)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-txouttotal-toolarge");
    }

    // Check for duplicate inputs (see CVE-2018-17144)
    // While Consensus::CheckTxInputs does check if all inputs of a tx are available, and UpdateCoins marks all inputs
    // of a tx as spent, it does not check if the tx has duplicate inputs.
    // Failure to run this check will result in either a crash or an inflation bug, depending on the implementation of
    // the underlying coins database.
    std::set<COutPoint> vInOutPoints;
    for (const auto& txin : tx.vin) {
        if (!vInOutPoints.insert(txin.prevout).second)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-inputs-duplicate");
    }

    if (tx.IsCoinBase())
    {
        const bool is_legacy_mooncoin_coinbase = IsLegacyMooncoinCoinbaseLengthException(tx);
        if ((tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100) &&
            !is_legacy_mooncoin_coinbase)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-cb-length");
    }
    else
    {
        for (const auto& txin : tx.vin)
            if (txin.prevout.IsNull())
                return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-prevout-null");
    }

    return true;
}
