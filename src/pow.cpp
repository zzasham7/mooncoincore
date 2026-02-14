// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <cmath>

namespace {
constexpr int kMooncoinPowMigrationHeight = 1;
constexpr int kMooncoinAlgoForkHeight = 1250000; // PowV4
constexpr int kMooncoinAlgoV5Height = 8250000;   // PowV5 (kept far in future as in v0.17)
constexpr int kMooncoinAlgoSmoothingPeriod = 8;

unsigned int GetNextWorkRequiredLtc(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight + 1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = params.DifficultyAdjustmentInterval() - 1;
    if ((pindexLast->nHeight + 1) != params.DifficultyAdjustmentInterval())
        blockstogoback = params.DifficultyAdjustmentInterval();

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int GetNextWorkRequiredMooncoinV1(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    const unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();
    const int64_t nTargetTimespan = 8 * 60 * 60; // legacy Mooncoin V1: 8 hours
    const int64_t nTargetSpacing = 90;           // legacy Mooncoin V1: 90 seconds
    const int64_t nInterval = nTargetTimespan / nTargetSpacing;

    if (pindexLast == nullptr) {
        return nProofOfWorkLimit;
    }

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight + 1) % nInterval != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + nTargetSpacing * 2) {
                return nProofOfWorkLimit;
            }

            // Return the last non-special-min-difficulty-rules-block
            const CBlockIndex* pindex = pindexLast;
            while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit) {
                pindex = pindex->pprev;
            }
            return pindex->nBits;
        }
        return pindexLast->nBits;
    }

    // Go back the full period unless it's the first retarget after genesis.
    int blockstogoback = nInterval - 1;
    if ((pindexLast->nHeight + 1) != nInterval) {
        blockstogoback = nInterval;
    }

    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++) {
        pindexFirst = pindexFirst->pprev;
    }
    assert(pindexFirst != nullptr);

    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    if (pindexLast->nHeight + 1 > 10000) {
        if (nActualTimespan < nTargetTimespan / 4) nActualTimespan = nTargetTimespan / 4;
        if (nActualTimespan > nTargetTimespan * 4) nActualTimespan = nTargetTimespan * 4;
    } else if (pindexLast->nHeight + 1 > 5000) {
        if (nActualTimespan < nTargetTimespan / 8) nActualTimespan = nTargetTimespan / 8;
        if (nActualTimespan > nTargetTimespan * 4) nActualTimespan = nTargetTimespan * 4;
    } else {
        if (nActualTimespan < nTargetTimespan / 16) nActualTimespan = nTargetTimespan / 16;
        if (nActualTimespan > nTargetTimespan * 4) nActualTimespan = nTargetTimespan * 4;
    }

    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }
    return bnNew.GetCompact();
}

unsigned int KimotoGravityWell(const CBlockIndex* pindexLast, uint64_t targetSpacingSeconds, uint64_t pastBlocksMin, uint64_t pastBlocksMax, const Consensus::Params& params)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const unsigned int nProofOfWorkLimit = bnPowLimit.GetCompact();

    if (pindexLast == nullptr || pindexLast->nHeight == 0 || static_cast<uint64_t>(pindexLast->nHeight) < pastBlocksMin) {
        return nProofOfWorkLimit;
    }

    const CBlockIndex* blockLastSolved = pindexLast;
    const CBlockIndex* blockReading = pindexLast;
    int64_t latestBlockTime = blockLastSolved->GetBlockTime();
    uint64_t pastBlocksMass = 0;
    int64_t pastRateActualSeconds = 0;
    int64_t pastRateTargetSeconds = 0;
    double pastRateAdjustmentRatio = 1.0;
    arith_uint256 pastDifficultyAverage;
    arith_uint256 pastDifficultyAveragePrev;

    for (unsigned int i = 1; blockReading && blockReading->nHeight > 0; i++) {
        if (pastBlocksMax > 0 && i > pastBlocksMax) {
            break;
        }
        pastBlocksMass++;

        if (i == 1) {
            pastDifficultyAverage.SetCompact(blockReading->nBits);
        } else {
            arith_uint256 current;
            current.SetCompact(blockReading->nBits);
            if (current > pastDifficultyAveragePrev) {
                pastDifficultyAverage = pastDifficultyAveragePrev + ((current - pastDifficultyAveragePrev) / i);
            } else {
                pastDifficultyAverage = pastDifficultyAveragePrev - ((pastDifficultyAveragePrev - current) / i);
            }
        }
        pastDifficultyAveragePrev = pastDifficultyAverage;

        if (latestBlockTime < blockReading->GetBlockTime()) {
            if (blockReading->nHeight > 67500) {
                latestBlockTime = blockReading->GetBlockTime();
            }
        }
        pastRateActualSeconds = latestBlockTime - blockReading->GetBlockTime();
        pastRateTargetSeconds = static_cast<int64_t>(targetSpacingSeconds * pastBlocksMass);
        pastRateAdjustmentRatio = 1.0;
        if (blockReading->nHeight > 67500) {
            if (pastRateActualSeconds < 1) {
                pastRateActualSeconds = 1;
            }
        } else {
            if (pastRateActualSeconds < 0) {
                pastRateActualSeconds = 0;
            }
        }
        if (pastRateActualSeconds != 0 && pastRateTargetSeconds != 0) {
            pastRateAdjustmentRatio = static_cast<double>(pastRateTargetSeconds) / static_cast<double>(pastRateActualSeconds);
        }

        const double eventHorizonDeviation = 1 + (0.7084 * std::pow((static_cast<double>(pastBlocksMass) / 144.0), -1.228));
        const double eventHorizonDeviationFast = eventHorizonDeviation;
        const double eventHorizonDeviationSlow = 1.0 / eventHorizonDeviation;

        if (pastBlocksMass >= pastBlocksMin) {
            if (pastRateAdjustmentRatio <= eventHorizonDeviationSlow || pastRateAdjustmentRatio >= eventHorizonDeviationFast) {
                break;
            }
        }
        if (blockReading->pprev == nullptr) {
            break;
        }
        blockReading = blockReading->pprev;
    }

    arith_uint256 bnNew = pastDifficultyAverage;
    if (pastRateActualSeconds != 0 && pastRateTargetSeconds != 0) {
        bnNew *= static_cast<uint64_t>(pastRateActualSeconds);
        bnNew /= static_cast<uint64_t>(pastRateTargetSeconds);
    }
    if (bnNew.GetCompact() > nProofOfWorkLimit) {
        bnNew = bnPowLimit;
    }
    if (pindexLast->nHeight + 1 == 1099915) {
        return 471071101;
    }
    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequiredMooncoinV2(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    static const int64_t kBlocksTargetSpacing = 90;
    const unsigned int timeDaySeconds = 60 * 60 * 24;
    const int64_t pastSecondsMin = static_cast<int64_t>(timeDaySeconds * 0.25);
    const int64_t pastSecondsMax = static_cast<int64_t>(timeDaySeconds * 7);
    const uint64_t pastBlocksMin = pastSecondsMin / kBlocksTargetSpacing;
    const uint64_t pastBlocksMax = pastSecondsMax / kBlocksTargetSpacing;
    return KimotoGravityWell(pindexLast, kBlocksTargetSpacing, pastBlocksMin, pastBlocksMax, params);
}

unsigned int GetNextWorkRequiredMooncoinV3(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const unsigned int nProofOfWorkLimit = bnPowLimit.GetCompact();
    constexpr bool fTestNet = false;
    constexpr int64_t nTargetSpacing = 90; // target 90 sec
    const int64_t retargetTimespan = nTargetSpacing;
    const int64_t retargetSpacing = nTargetSpacing;
    const int64_t retargetInterval = retargetTimespan / retargetSpacing;

    if (pindexLast == nullptr) return nProofOfWorkLimit;

    // Only change once per interval
    if ((pindexLast->nHeight + 1) % retargetInterval != 0) {
        if (fTestNet) {
            // Special testnet min-difficulty fallback.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + retargetSpacing * 2) {
                return nProofOfWorkLimit;
            } else {
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % retargetInterval != 0 && pindex->nBits == nProofOfWorkLimit) {
                    pindex = pindex->pprev;
                }
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    int blockstogoback = retargetInterval - 1;
    if ((pindexLast->nHeight + 1) != retargetInterval) blockstogoback = retargetInterval;

    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++) {
        pindexFirst = pindexFirst->pprev;
    }
    assert(pindexFirst != nullptr);

    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    if (nActualTimespan < (retargetTimespan - (retargetTimespan / 4))) nActualTimespan = (retargetTimespan - (retargetTimespan / 4));
    if (nActualTimespan > (retargetTimespan + (retargetTimespan / 2))) nActualTimespan = (retargetTimespan + (retargetTimespan / 2));

    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= retargetTimespan;

    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }
    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequiredMooncoinV5(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    // DUAL_KGW3 (legacy Mooncoin branch).
    const CBlockIndex* blockLastSolved = pindexLast;
    const CBlockIndex* blockReading = pindexLast;
    int64_t pastBlocksMass = 0;
    int64_t pastRateActualSeconds = 0;
    int64_t pastRateTargetSeconds = 0;
    double pastRateAdjustmentRatio = 1.0;
    arith_uint256 pastDifficultyAverage;
    arith_uint256 pastDifficultyAveragePrev;

    static const int64_t blockTime = 90;
    static const unsigned int timeDaySeconds = 86400;
    const int64_t pastSecondsMin = static_cast<int64_t>(timeDaySeconds * 0.025);
    const int64_t pastSecondsMax = static_cast<int64_t>(timeDaySeconds * 7);
    const int64_t pastBlocksMin = pastSecondsMin / blockTime;
    const int64_t pastBlocksMax = pastSecondsMax / blockTime;
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

    if (blockLastSolved == nullptr || blockLastSolved->nHeight == 0 ||
        static_cast<int64_t>(blockLastSolved->nHeight) < pastBlocksMin) {
        return bnPowLimit.GetCompact();
    }

    for (unsigned int i = 1; blockReading && blockReading->nHeight > 0; i++) {
        if (pastBlocksMax > 0 && i > static_cast<unsigned int>(pastBlocksMax)) break;
        pastBlocksMass++;
        pastDifficultyAverage.SetCompact(blockReading->nBits);
        if (i > 1) {
            if (pastDifficultyAverage >= pastDifficultyAveragePrev) {
                pastDifficultyAverage = ((pastDifficultyAverage - pastDifficultyAveragePrev) / i) + pastDifficultyAveragePrev;
            } else {
                pastDifficultyAverage = pastDifficultyAveragePrev - ((pastDifficultyAveragePrev - pastDifficultyAverage) / i);
            }
        }
        pastDifficultyAveragePrev = pastDifficultyAverage;
        pastRateActualSeconds = blockLastSolved->GetBlockTime() - blockReading->GetBlockTime();
        pastRateTargetSeconds = blockTime * pastBlocksMass;
        pastRateAdjustmentRatio = 1.0;
        if (pastRateActualSeconds < 0) pastRateActualSeconds = 0;
        if (pastRateActualSeconds != 0 && pastRateTargetSeconds != 0) {
            pastRateAdjustmentRatio = static_cast<double>(pastRateTargetSeconds) / static_cast<double>(pastRateActualSeconds);
        }
        const double eventHorizonDeviation = 1 + (0.7084 * std::pow((static_cast<double>(pastBlocksMass) / 72.0), -1.228));
        const double eventHorizonDeviationFast = eventHorizonDeviation;
        const double eventHorizonDeviationSlow = 1.0 / eventHorizonDeviation;

        if (pastBlocksMass >= pastBlocksMin) {
            if (pastRateAdjustmentRatio <= eventHorizonDeviationSlow || pastRateAdjustmentRatio >= eventHorizonDeviationFast) {
                break;
            }
        }
        if (blockReading->pprev == nullptr) break;
        blockReading = blockReading->pprev;
    }

    arith_uint256 kgwDual1(pastDifficultyAverage);
    arith_uint256 kgwDual2;
    kgwDual2.SetCompact(pindexLast->nBits);
    if (pastRateActualSeconds != 0 && pastRateTargetSeconds != 0) {
        kgwDual1 *= pastRateActualSeconds;
        kgwDual1 /= pastRateTargetSeconds;
    }
    int64_t nActualTime1 = pindexLast->GetBlockTime() - pindexLast->pprev->GetBlockTime();
    int64_t nActualTimespanShort = nActualTime1;

    if (nActualTime1 < 0) nActualTime1 = blockTime;
    if (nActualTime1 < blockTime / 3) nActualTime1 = blockTime / 3;
    if (nActualTime1 > blockTime * 3) nActualTime1 = blockTime * 3;

    kgwDual2 *= nActualTime1;
    kgwDual2 /= blockTime;

    arith_uint256 bnNew = (kgwDual2 + kgwDual1) / 2;

    // DUAL_KGW3 increases difficulty rapidly if previous block interval is too short.
    if (nActualTimespanShort < blockTime / 6) {
        bnNew *= 85;
        bnNew /= 100;
    }

    // If block generation stalls too long, reduce difficulty aggressively.
    const int nLongTimeLimit = 60 * 60; // 60 minutes
    if ((pblock->GetBlockTime() - pindexLast->GetBlockTime()) > nLongTimeLimit) {
        bnNew = bnPowLimit / 15;
    }

    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }

    // Legacy v0.17 mainnet historical compatibility points.
    const int nextHeight = pindexLast->nHeight + 1;
    if (nextHeight == 1349150) return 454022324;
    if (nextHeight == 1500773) return 469796712;
    if (nextHeight == 1511216) return 453654908;
    if (nextHeight == 1527334) return 455454971;
    if (nextHeight == 1585972) return 469833684;
    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequiredMooncoinCompat(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    // Legacy mooncoin mode switch heights:
    //   < 26850 -> V1
    //   >=26850 -> KGW (V2)
    //   >=1100000 -> DigiShield (V3)
    const int nextHeight = pindexLast->nHeight + 1;
    if (nextHeight < 26850) {
        return GetNextWorkRequiredMooncoinV1(pindexLast, pblock, params);
    }
    if (nextHeight < 1100000) {
        return GetNextWorkRequiredMooncoinV2(pindexLast, params);
    }
    if (nextHeight < kMooncoinAlgoForkHeight) {
        return GetNextWorkRequiredMooncoinV3(pindexLast, pblock, params);
    }
    if (nextHeight < kMooncoinAlgoForkHeight + kMooncoinAlgoSmoothingPeriod) {
        return 0x1e0fffff;
    }
    if (nextHeight < kMooncoinAlgoV5Height) {
        return GetNextWorkRequiredMooncoinV5(pindexLast, pblock, params);
    }
    // Keep current path for future heights.
    return GetNextWorkRequiredMooncoinV5(pindexLast, pblock, params);
}
} // namespace

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    if (pindexLast->nHeight + 1 >= kMooncoinPowMigrationHeight) {
        return GetNextWorkRequiredMooncoinCompat(pindexLast, pblock, params);
    }
    return GetNextWorkRequiredLtc(pindexLast, pblock, params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    // Litecoin: intermediate uint256 can overflow by 1 bit
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
