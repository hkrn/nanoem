/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

extern "C" int nanoemTestSuiteRun(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    int result = nanoemTestSuiteRun(argc, argv);
    return (result < 0xff ? result : 0xff);
}
