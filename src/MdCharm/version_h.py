import os
import sys
from datetime import datetime, tzinfo

version_h = '''
#ifndef VERSION_H
#define VERSION_H

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

#define VERSION {0}
const char* const VERSION_STR = STRINGIFY(VERSION);

#define VERSION_MAJOR {1}
const char* const VERSION_MAJOR_STR = STRINGIFY(VERSION_MAJOR);
#define VERSION_MINOR {2}
const char* const VERSION_MINOR_STR = STRINGIFY(VERSION_MINOR);
#define VERSION_RELEASE {3}
const char* const VERSION_RELEASE_STR = STRINGIFY(VERSION_RELEASE);

#define REVISION {4}
const char* const REVISION_STR = STRINGIFY(REVISION);

#define BUILT_TIME {5}
const char* const BUILT_TIME_STR = STRINGIFY(BUILT_TIME);

#undef REVISION
#undef BUILT_TIME
#undef STRINGIFY_INTERNAL
#undef STRINGIFY

#endif
'''
mdcharm_rc = '''
IDI_ICON1	ICON	DISCARDABLE	"mdcharm.ico"
IDI_ICON2	ICON	DISCARDABLE	"markdown\markdown.ico"

1 VERSIONINFO
FILEVERSION {0},{1},{2},{3}
PRODUCTVERSION {0},{1},{2},{3}
FILEOS      0x4
FILETYPE    0x1
{{
BLOCK "StringFileInfo"
{{
	BLOCK "040904E4"
	{{
		VALUE "CompanyName", "MdCharm"
		VALUE "FileDescription", "MdCharm-Wiki Editor [msvc]"
		VALUE "FileVersion", "{4}"
		VALUE "InternalName", "MdCharm"
		VALUE "LegalCopyright", "(C) MdCharm. 2012-2013"
		VALUE "LegalTrademarks", ""
		VALUE "OriginalFilename", "MdCharm"
		VALUE "ProductName", "MdCharm"
		VALUE "ProductVersion", "{4}"
		VALUE "Comments", ""
		VALUE "Aditional Notes", "http://www.mdcharm.com/"
	}}
}}
BLOCK "VarFileInfo"
{{
	VALUE "Translation",0x0409,0x04E4
}}
}}
'''
'''
Usage: version_h.py git_bin_path version_file_path debug|release, like
        version_h.py C:/Progra~1/Git/bin/git.exe version.h ../res/mdcharm.rc debug
'''

if __name__ == '__main__':
    if len(sys.argv)!=5:
        sys.exit(-100)
    if sys.argv[4]=="debug":
        if os.path.isfile(sys.argv[2]):
            print 'Already Exist'
            sys.exit(0)
    GitPath = sys.argv[1]
    revisionOutput = os.popen(GitPath + ' log -1 --format="%H"')
    revision = None
    if revisionOutput:
        revision = str(revisionOutput.read()).strip()
    else:
        sys.exit(-99)
    tagOutput = str(os.popen(GitPath + ' tag').read()).strip()
    tag = None
    if tagOutput:
        tag = tagOutput.replace('\r\n', '\n').split('\n')[-1]
    else:
        sys.exit(-98)
    if len(tag.split('.')) != 3:
        sys.exit(-97)
    versionList = tag.split('.')
    ntimeStr = datetime.now().strftime('%B %d %Y %H:%M:%S +0800')
    RealVersionH = version_h.strip().format(tag, versionList[0], versionList[1],
                                    versionList[2], revision, ntimeStr)
    resultFile = file(sys.argv[2], 'w+')
    resultFile.write(RealVersionH)
    resultFile.close()
    RealRcFile = mdcharm_rc.format(versionList[0], versionList[1], versionList[2], 0, tag)
    rcFile = file(sys.argv[3], 'w+')
    rcFile.write(RealRcFile)
    rcFile.close()
#    print RealVersionH
#    print '--------------'
#    print tag
#    print versionList
#    print revision
