#ifndef __chain_builder_h__
#define __chain_builder_h__

#include <iostream>
#include <string>
#include <vector>

#include <TChain.h>
#include <TFile.h>

#include "input_config.h"

// Structure to hold all chains for the analysis
struct TreeChains {
    TChain* evtChain;
    TChain* jetChain;
    TChain* triggerChain;
    TChain* skimChain;
    TChain* photonChain;  // nullptr if not needed
    TChain* muonChain;

    Long64_t nEntries;

    TreeChains() : evtChain(nullptr), jetChain(nullptr),
                   triggerChain(nullptr), skimChain(nullptr),
                   photonChain(nullptr), muonChain(nullptr), nEntries(0) {}

    ~TreeChains() {
        // Note: ROOT manages TChain cleanup
    }
};

// Build chains from list of files
inline TreeChains* BuildChains(const std::vector<std::string>& files,
                                const std::string& jetTreeName,
                                bool needPhotonTree = false,
                                bool needMuonTree = false,
                                bool verbose = true) {

    TreeChains* chains = new TreeChains();

    // Create chains with tree paths
    chains->evtChain = new TChain("hiEvtAnalyzer/HiTree");
    chains->jetChain = new TChain(jetTreeName.c_str());
    chains->triggerChain = new TChain("hltanalysis/HltTree");
    chains->skimChain = new TChain("skimanalysis/HltTree");

    if (needPhotonTree) {
        chains->photonChain = new TChain("ggHiNtuplizer/EventTree");
    }
    if (needMuonTree) {
        chains->muonChain = new TChain("muonAnalyzer/MuonTree");
    }

    // Add files to all chains
    int nFilesAdded = 0;
    int nFilesSkipped = 0;

    for (const auto& file : files) {
        // Quick validation - try to open file
        TFile* testFile = TFile::Open(file.c_str(), "READ");
        if (!testFile || testFile->IsZombie()) {
            std::cerr << "WARNING: Cannot open file, skipping: " << file << std::endl;
            if (testFile) delete testFile;
            nFilesSkipped++;
            continue;
        }

        // Verify required trees exist
        if (!testFile->Get("hiEvtAnalyzer/HiTree")) {
            std::cerr << "WARNING: Missing event tree in: " << file << std::endl;
            delete testFile;
            nFilesSkipped++;
            continue;
        }
        if (!testFile->Get(jetTreeName.c_str())) {
            std::cerr << "WARNING: Missing jet tree (" << jetTreeName
                      << ") in: " << file << std::endl;
            delete testFile;
            nFilesSkipped++;
            continue;
        }
        if (needPhotonTree && !testFile->Get("ggHiNtuplizer/EventTree")) {
            std::cerr << "WARNING: Missing photon tree in: " << file << std::endl;
            delete testFile;
            nFilesSkipped++;
            continue;
        }
        if (needMuonTree && !testFile->Get("muonAnalyzer/MuonTree")) {  // ← ADD THIS BLOCK
            std::cerr << "WARNING: Missing muon tree in: " << file << std::endl;
            delete testFile;
            nFilesSkipped++;
            continue;
        }
        delete testFile;

        // Add to chains
        chains->evtChain->Add(file.c_str());
        chains->jetChain->Add(file.c_str());
        chains->triggerChain->Add(file.c_str());
        chains->skimChain->Add(file.c_str());

        if (chains->photonChain) {
            chains->photonChain->Add(file.c_str());
        }
        if (chains->muonChain) {
            chains->muonChain->Add(file.c_str());
        }

        nFilesAdded++;
        if (verbose && nFilesAdded % 100 == 0) {
            std::cout << "Added " << nFilesAdded << " files..." << std::endl;
        }
    }

    if (verbose) {
        std::cout << "Files added: " << nFilesAdded << ", skipped: " << nFilesSkipped << std::endl;
    }

    // Get total entries (use event tree as reference)
    chains->nEntries = chains->evtChain->GetEntries();

    if (verbose) {
        std::cout << "Total entries in chain: " << chains->nEntries << std::endl;
    }

    return chains;
}

// Build chains from InputConfig
inline TreeChains* BuildChainsFromConfig(const InputConfig& config,
                                          const std::string& jetTreeName,
                                          bool needPhotonTree = false,
                                          bool needMuonTree = false) {
    std::vector<std::string> files = GetInputFiles(config);

    if (files.empty()) {
        std::cerr << "ERROR: No input files found!" << std::endl;
        return nullptr;
    }

    std::cout << "Building chains from " << files.size() << " files" << std::endl;

    return BuildChains(files, jetTreeName, needPhotonTree, needMuonTree);
}

#endif
