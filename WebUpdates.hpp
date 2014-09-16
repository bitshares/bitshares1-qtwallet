#pragma once

#include <fc/crypto/elliptic.hpp>

#include <set>
#include <vector>

const static std::string                                    WEB_UPDATES_REPOSITORY = "http://localhost:8888/";
const static uint8_t                                        WEB_UPDATES_SIGNATURE_REQUIREMENT = 2;
const static std::unordered_set<fc::ecc::public_key>        WEB_UPDATES_SIGNING_KEYS =
{
    //Add update keys here
};

struct WebUpdateManifest {
    struct UpdateDetails {
        //The version number; must be unique within the manifest.
        uint8_t majorVersion;
        uint8_t forkVersion;
        uint8_t minorVersion;
        uint8_t patchVersion;

        //Set of signatures for this update; must contain signatures
        //corresponding to at least WEB_UPDATES_SIGNATURE_REQUIREMENT unique
        //keys in WEB_UPDATES_SIGNING_KEYS.
        std::unordered_set<fc::ecc::compact_signature> signatures;

        //Human-readable description of update. May include description,
        //changelog, known issues, etc.
        std::string releaseNotes;

        //Full URL (i.e. https://bitshares.org/toolkit/updates/0.2.4-c.pak)
        //to update package
        std::string updatePackageUrl;
    };
};
