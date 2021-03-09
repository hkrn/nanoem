/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "AnalyticsTracker.h"

@class WebView;

namespace nanoem {
namespace macos {

class Preference;

class AnalyticsTracker {
public:
    AnalyticsTracker(Preference *preferences);
    ~AnalyticsTracker();

    void send(const char *screen, const char *category, const char *action, const char *label);

private:
    Preference *m_preference;
    WebView *m_webview;
};

} /* namespace macos */
} /* namespace nanoem */
