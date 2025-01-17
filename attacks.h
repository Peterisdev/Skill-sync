#ifndef ATTACKS_H
#define ATTACKS_H

enum AttackType {
    NONE,
    DEAUTH,
    BEACON_SPAM,
    PROBE_SPAM,
    RICKROLL,
    EVIL_TWIN,
    PROBE_SNIFF,
    RICKROLL_BEACON,
    CAPTIVE_PORTAL
};

inline String getAttackName(AttackType type) {
    switch(type) {
        case DEAUTH: return "Deauth";
        case BEACON_SPAM: return "Beacon Spam";
        case PROBE_SPAM: return "Probe Spam";
        case RICKROLL: return "Rickroll";
        case EVIL_TWIN: return "Evil Twin";
        case PROBE_SNIFF: return "Probe Sniff";
        case RICKROLL_BEACON: return "Rickroll Beacon";
        case CAPTIVE_PORTAL: return "Captive Portal";
        default: return "None";
    }
}

#endif 