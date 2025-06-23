#include <iostream>

#include <chainparams.h>
#include <dbwrapper.h>
#include <gridcoin/voting/vote.h>
#include <key_io.h>
#include <node/blockstorage.h>
#include <util/system.h>

std::atomic<int64_t> GLOBAL_GROUP_COUNTER(0);

struct AddressData {
    CAmount balance;
    int64_t group_id;
    uint32_t last_stake_timestamp;

    AddressData() {
        balance = 0;
        group_id = ++GLOBAL_GROUP_COUNTER;
        last_stake_timestamp = 0;
    }
};

int main(int argc, char** argv) {
    std::string error;

    ArgsManager args;
    SetupHelpOptions(args);

    args.AddArg("-testnet", "Run on testnet chain.", ArgsManager::ALLOW_BOOL, OptionsCategory::OPTIONS);

    if (!args.ParseParameters(argc, argv, error)) {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    if (HelpRequested(args)) {
        std::cout << gArgs.GetHelpMessage() << std::endl;
        return EXIT_FAILURE;
    }

    SelectParams(CBaseChainParams::MAIN);
    if (gArgs.GetBoolArg("-testnet", false)) {
        SelectParams(CBaseChainParams::TESTNET);
        fTestNet = true;
    }

    if (!gArgs.ReadConfigFiles(error, true)) {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    fs::path datadir = GetDataDir();

    if (!DirIsWritable(datadir)) {
        std::cerr << "Cannot write to data directory: " << datadir.string() << std::endl;
        return EXIT_FAILURE;
    }

    if (!LockDirectory(datadir, ".lock", false)) {
        std::cerr << "Cannot obtain a lock on data directory. Is an instance of Gridcoin running?" << std::endl;
        return EXIT_FAILURE;
    }


    std::cout << "Loading the block index..." << std::endl;
    CTxDB txdb("r");
    if (!txdb.LoadBlockIndex()) {
        std::cerr << "Failed to load block index." << std::endl;
        return EXIT_FAILURE;
    }

    std::map<std::string, AddressData> wallet_wealth_map;
    CBlock block;
    CTransaction tx2;
    CTxDestination dest;

    for (CBlockIndex* pindex = pindexGenesisBlock; pindex != nullptr; pindex = pindex->pnext) {
        std::cout << pindex->nHeight << "/" << pindexBest->nHeight << " " << ((double)pindex->nHeight / pindexBest->nHeight) * 100 << "%\r";

        if (!ReadBlockFromDisk(block, pindex, Params().GetConsensus())) {
            std::cerr << "Failure while reading block with hash: " << pindex->GetBlockHash().GetHex();
            return EXIT_FAILURE;
        }

        for (const auto& tx : block.vtx) {
            if (!tx.IsCoinBase()) {
                for (const auto& in : tx.vin) {
                    if (!ReadTxFromDisk(tx2, in.prevout)) {
                        continue;
                    }

                    if (!ExtractDestination(tx2.vout[in.prevout.n].scriptPubKey, dest)) {
                        continue;
                    }

                    wallet_wealth_map[EncodeDestination(dest)].balance -= tx2.vout[in.prevout.n].nValue;
                }
            }

            if (tx.IsCoinStake()) {
                assert(ExtractDestination(tx.vout[1].scriptPubKey, dest));

                wallet_wealth_map[EncodeDestination(dest)].last_stake_timestamp = tx.nTime;
            }

            for (const auto& out : tx.vout) {
                if (!ExtractDestination(out.scriptPubKey, dest)) {
                    continue;
                }

                wallet_wealth_map[EncodeDestination(dest)].balance += out.nValue;
            }

            for (const auto& contract : tx.GetContracts()) {
                if (contract.m_type == GRC::ContractType::VOTE) {
                    // No address claims for legacy polls.
                    if (contract.m_version < 2) {
                        continue;
                    }

                    const auto& payload = contract.SharePayloadAs<GRC::Vote>();

                    int64_t group_id = -1;
                    for (const auto& address_claim : payload->m_claim.m_balance_claim.m_address_claims) {
                        auto address = EncodeDestination(address_claim.m_public_key.GetID());

                        if (group_id == -1) {
                            group_id = wallet_wealth_map[address].group_id;
                        }

                        wallet_wealth_map[address].group_id = group_id;
                    }
                }
            }
        }
    }

    std::cout << std::fixed;
    for (const auto& [address, data] : wallet_wealth_map) {
        std::cout << data.group_id << "\t" << address << "\t" << data.last_stake_timestamp << "\t" << (double)data.balance / COIN << std::endl;
    }
}
