#include <iostream>

#include <base58.h>
#include <chainparams.h>
#include <dbwrapper.h>
#include <gridcoin/voting/vote.h>
#include <node/blockstorage.h>
#include <util/system.h>

int main(int argc, char** argv) {
    std::string error;

    ArgsManager args;
    SetupHelpOptions(args);

    args.AddArg("-testnet", "Run on testnet chain.", ArgsManager::ALLOW_BOOL, OptionsCategory::OPTIONS);
    args.AddArg("-poll_txid=<poll txid>", "The poll txid to scan the votes for.", ArgsManager::ALLOW_STRING, OptionsCategory::OPTIONS);

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

    uint256 poll_txid;

    if (args.GetArg("-poll_txid", "invalid") == "invalid" || (poll_txid = uint256S(args.GetArg("-poll_txid", ""))).IsNull()) {
        std::cerr << "Invalid poll txid." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Configured to run for poll: " << poll_txid.GetHex() << std::endl;

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


    std::map<std::string, CAmount> wallet_wealth_map;
    for (CBlockIndex* pindex = pindexGenesisBlock; pindex != nullptr; pindex = pindex->pnext) {
        std::cout << pindex->nHeight << "/" << pindexBest->nHeight << " " << ((double)pindex->nHeight / pindexBest->nHeight) * 100 << "%\r";
        if (pindex->IsContract()) {
            CBlock block;
            if (!ReadBlockFromDisk(block, pindex, Params().GetConsensus())) {
                std::cerr << "Failure while reading block with hash: " << pindex->GetBlockHash().GetHex();
                return EXIT_FAILURE;
            }

            for (const auto& tx : block.vtx) {
                for (const auto& contract : tx.GetContracts()) {
                    if (contract.m_type == GRC::ContractType::VOTE) {
                        const auto& payload = contract.SharePayloadAs<GRC::Vote>();
                        if (payload->m_poll_txid == poll_txid) {
                            CAmount amount = 0;
                            for (const auto& address_claim : payload->m_claim.m_balance_claim.m_address_claims) {
                                for (const auto& outpoint : address_claim.m_outpoints) {
                                    CTxIndex tx_index;

                                    if (!txdb.ReadTxIndex(outpoint.hash, tx_index)) {
                                        std::cerr << "Error while reading the tx index." << std::endl;
                                        return EXIT_FAILURE;
                                    }

                                    CAutoFile file(OpenBlockFile(tx_index.pos.nFile, tx_index.pos.nBlockPos, "rb"), SER_DISK, CLIENT_VERSION);

                                    if (file.IsNull()) {
                                        std::cerr << "Error while opening the block file." << std::endl;
                                        return EXIT_FAILURE;
                                    }

                                    fseek(file.Get(), tx_index.pos.nTxPos, SEEK_SET);

                                    CTransaction tx;
                                    file >> tx;

                                    amount += tx.vout[outpoint.n].nValue;
                                }
                            }

                            std::string cpid = payload->m_claim.m_magnitude_claim.m_mining_id.ToString();
                            if (cpid != "INVESTOR") {
                                // User has a CPID.
                                wallet_wealth_map[cpid] = amount;
                            } else {
                                // Use the first address.
                                CBitcoinAddress address;
                                // TODO: balance check?
                                address.Set(payload->m_claim.m_balance_claim.m_address_claims[0].m_public_key.GetID());
                                wallet_wealth_map[address.ToString()] = amount;
                            }
                        }
                    }
                }
            }
        }
    }

    std::cout << std::fixed;
    for (const auto& [id, amount] : wallet_wealth_map) {
        std::cout << id << "\t" << (double)amount / COIN << std::endl;
    }
}
