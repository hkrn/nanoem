/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "AnalyticsTracker.h"
#import "Preference.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>

#if defined(NANOEM_HOCKEYSDK_APP_IDENTIFIER)
#import <HockeySDK/HockeySDK.h>
#endif /* NANOEM_HOCKEYSDK_APP_IDENTIFIER */

#include "emapp/StringUtils.h"

namespace nanoem {
namespace macos {

AnalyticsTracker::AnalyticsTracker(Preference *preference)
    : m_preference(preference)
    , m_webview(nil)
{
}

AnalyticsTracker::~AnalyticsTracker()
{
}

void
AnalyticsTracker::send(const char *screen, const char *category, const char *action, const char *label)
{
    if (m_preference->applicationPreference()->isAnalyticsEnabled()) {
        NSURL *url = [[NSURL alloc] initWithString:@"https://www.google-analytics.com/collect"];
        NSMutableCharacterSet *characterSet = [NSMutableCharacterSet alphanumericCharacterSet];
        [characterSet addCharactersInString:@"-._~"];
        NSMutableString *body = [[NSMutableString alloc] init];
        [body appendString:@"v=1&"];
#ifdef NANOEM_GA_TRACKING_ID
        [body appendFormat:@"tid=%s&", NANOEM_GA_TRACKING_ID];
#endif
        NSScreen *mainScreen = [NSScreen mainScreen];
        NSString *clientUUID = m_preference->clientUUID();
        [body appendFormat:@"cid=%@&", [clientUUID stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
        [body appendString:@"aip=1&"];
        [body appendFormat:@"ul=%@&", [NSLocale currentLocale].localeIdentifier];
        NSSize resolution = mainScreen.frame.size;
        [body appendFormat:@"sr=%dx%d&", int(resolution.width), int(resolution.height)];
        [body appendFormat:@"sd=%ld-bits&", NSBitsPerPixelFromDepth(mainScreen.depth)];
        if (screen) {
            [body appendFormat:@"cd=%@&",
                  [[[NSString alloc] initWithUTF8String:screen]
                      stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
        }
        [body appendString:@"ds=app&an=nanoem&"];
        [body appendFormat:@"av=%@&",
              [[[NSString alloc] initWithUTF8String:nanoemGetVersionString()]
                  stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
        [body appendString:@"t=event&"];
        if (category) {
            [body appendFormat:@"ec=%@&",
                  [[[NSString alloc] initWithUTF8String:category]
                      stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
        }
        if (action) {
            [body appendFormat:@"ea=%@&",
                  [[[NSString alloc] initWithUTF8String:action]
                      stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
        }
        if (label) {
            [body appendFormat:@"el=%@&",
                  [[[NSString alloc] initWithUTF8String:label]
                      stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
        }
        if (category && action && StringUtils::equals(category, "nanoem.session") &&
            StringUtils::equals(action, "begin")) {
            [body appendString:@"sc=start&"];
#if defined(NANOEM_HOCKEYSDK_APP_IDENTIFIER)
            [[BITSystemProfile sharedSystemProfile] startUsage];
#endif
        }
        if (category && action && StringUtils::equals(category, "nanoem.session") &&
            StringUtils::equals(action, "end")) {
            [body appendString:@"sc=end&"];
#if defined(NANOEM_HOCKEYSDK_APP_IDENTIFIER)
            [[BITSystemProfile sharedSystemProfile] stopUsage];
#endif
        }
        if (!m_webview) {
            m_webview = [[WebView alloc] init];
        }
        [body appendFormat:@"ua=%@",
              [[m_webview userAgentForURL:url] stringByAddingPercentEncodingWithAllowedCharacters:characterSet]];
#ifdef NANOEM_GA_TRACKING_ID
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
        request.HTTPBody = [body dataUsingEncoding:NSUTF8StringEncoding];
        request.HTTPMethod = @"POST";
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionTask *task = [session dataTaskWithRequest:request
                                            completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                                BX_UNUSED_3(data, response, error);
                                            }];
        [task resume];
#else
        NSLog(@"%@?%@", url, body);
#endif
    }
}

} /* namespace macos */
} /* namespace nanoem */
