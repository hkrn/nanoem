/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <Foundation/Foundation.h>

#include "emapp/Error.h"

@interface
NSError (Extension)

@property (readonly) nanoem::Error nanoem;

@end

@implementation
NSError (Extension)

- (nanoem::Error)nanoem
{
    NSString *reason = self.localizedFailureReason;
    NSString *suggestion = self.localizedRecoverySuggestion;
    return nanoem::Error(
        reason ? reason.UTF8String : "", suggestion ? suggestion.UTF8String : "", nanoem::Error::kDomainTypeOS);
}

@end
