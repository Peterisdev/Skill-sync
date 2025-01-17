#ifndef TEMPLATES_H
#define TEMPLATES_H

enum PortalTemplate {
    NO_TEMPLATE,
    INSTAGRAM,
    FACEBOOK,
    GOOGLE,
    TWITTER,
    LINKEDIN,
    APPLE,
    NETFLIX,
    SPOTIFY,
    MICROSOFT
};

inline String getTemplateName(PortalTemplate tmpl) {
    switch(tmpl) {
        case INSTAGRAM: return "Instagram";
        case FACEBOOK: return "Facebook";
        case GOOGLE: return "Google";
        case TWITTER: return "Twitter";
        case LINKEDIN: return "LinkedIn";
        case APPLE: return "Apple";
        case NETFLIX: return "Netflix";
        case SPOTIFY: return "Spotify";
        case MICROSOFT: return "Microsoft";
        default: return "None";
    }
}

inline String getTemplateHTML(PortalTemplate tmpl) {
    switch(tmpl) {
        case INSTAGRAM: return "instagram.html";
        case FACEBOOK: return "facebook.html";
        case GOOGLE: return "google.html";
        case TWITTER: return "twitter.html";
        case LINKEDIN: return "linkedin.html";
        case APPLE: return "apple.html";
        case NETFLIX: return "netflix.html";
        case SPOTIFY: return "spotify.html";
        case MICROSOFT: return "microsoft.html";
        default: return "default.html";
    }
}

#endif