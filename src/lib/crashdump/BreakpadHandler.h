/*
	Copyright (c) 2009, Aleksey Palazhchenko
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

		* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above
	copyright notice, this list of conditions and the following disclaimer
	in the documentation and/or other materials provided with the
	distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BREAKPAD_HANDLER_H
#define BREAKPAD_HANDLER_H

#include <QtCore/QString>
namespace google_breakpad { class ExceptionHandler; }

namespace BreakpadQt
{

class GlobalHandlerPrivate;

enum ReportCrashesToSystem
{
	ReportUnhandled = 1,
	AlwaysReport = 2
};

class GlobalHandler
{
public:
	static GlobalHandler* instance();

	void setDumpPath(const QString& path);
	void setReporter(const QString& reporter);
    void appendArgument(const QString &systemInfoPath);
	void setReportCrashesToSystem(ReportCrashesToSystem report);
	bool writeMinidump();

private:
	GlobalHandler();
	~GlobalHandler();
	Q_DISABLE_COPY(GlobalHandler)

	GlobalHandlerPrivate* d;
};

}	// namespace

#endif	// BREAKPAD_HANDLER_H
