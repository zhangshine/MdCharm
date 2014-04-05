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

#include "BreakpadHandler.h"

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>

#if defined(Q_OS_MAC)
#include "client/mac/handler/exception_handler.h"
#elif defined(Q_OS_LINUX)
#include "client/linux/handler/exception_handler.h"
#elif defined(Q_OS_WIN32)
#include "client/windows/handler/exception_handler.h"
#endif

namespace BreakpadQt
{

class GlobalHandlerPrivate
{
public:
	GlobalHandlerPrivate();
	~GlobalHandlerPrivate();

public:
    static QString reporter_;
    static QStringList reporterArguments_;
	static google_breakpad::ExceptionHandler* handler_;
	static ReportCrashesToSystem reportCrashesToSystem_;
};

QString GlobalHandlerPrivate::reporter_ = QString();
QStringList GlobalHandlerPrivate::reporterArguments_ = QStringList();
google_breakpad::ExceptionHandler* GlobalHandlerPrivate::handler_ = 0;
ReportCrashesToSystem GlobalHandlerPrivate::reportCrashesToSystem_ = ReportUnhandled;


bool launcher(const QString &program, const QStringList &arguments)
{
	// TODO launcher
    if(!program.isEmpty()){
        return QProcess::startDetached(program, arguments);
    }
	return false;
}


#if defined(Q_OS_WIN32)
bool DumpCallback(const wchar_t* _dump_dir,
				const wchar_t* _minidump_id,
				void* context,
				EXCEPTION_POINTERS* exinfo,
				MDRawAssertionInfo* assertion,
				bool success)
#else
bool DumpCallback(const char* _dump_dir,
				const char* _minidump_id,
				void *context, bool success)
#endif
{
    //Q_UNUSED(_dump_dir);
    //Q_UNUSED(_minidump_id);
	Q_UNUSED(context);
#if defined(Q_OS_WIN32)
	Q_UNUSED(assertion);
	Q_UNUSED(exinfo);
#endif

	/*
		NO STACK USE, NO HEAP USE THERE !!!
		Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
	*/
    //QString dumpFilePath = QString::fromWCharArray(_dump_dir).append(QChar::fromLatin1('/'))+QString::fromWCharArray(_minidump_id);
    QString dumpFilePathFormat = QLatin1String("%1/%2.dmp");
#if defined(Q_OS_WIN32)
    GlobalHandlerPrivate::reporterArguments_.prepend(dumpFilePathFormat
                                                    .arg(QString::fromWCharArray(_dump_dir))
                                                    .arg(QString::fromWCharArray(_minidump_id)));
#else
    GlobalHandlerPrivate::reporterArguments_.prepend(dumpFilePathFormat
                                                    .arg(QString::fromLocal8Bit(_dump_dir))
                                                    .arg(QString::fromLocal8Bit(_minidump_id)));
#endif
    launcher(GlobalHandlerPrivate::reporter_, GlobalHandlerPrivate::reporterArguments_);
	return (GlobalHandlerPrivate::reportCrashesToSystem_ == ReportUnhandled) ? success : false;
}


GlobalHandlerPrivate::GlobalHandlerPrivate()
{
#if defined(Q_OS_WIN32)
    handler_ = new google_breakpad::ExceptionHandler(/*DumpPath*/ QDir::tempPath().toStdWString(), /*FilterCallback*/ 0, DumpCallback, /*context*/ 0, true);
#else
    handler_ = new google_breakpad::ExceptionHandler(QDir::tempPath().toStdString(), 0, DumpCallback, 0, true);
#endif
}

GlobalHandlerPrivate::~GlobalHandlerPrivate()
{
	delete handler_;
	handler_ = 0;
}


GlobalHandler* GlobalHandler::instance()
{
	static GlobalHandler globalHandler;
	return &globalHandler;
}

GlobalHandler::GlobalHandler()
{
	d = new GlobalHandlerPrivate();
}

GlobalHandler::~GlobalHandler()
{
	delete d;
	d = 0;
}

void GlobalHandler::setDumpPath(const QString& path)
{
	QString absPath = path;
	if(!QDir::isAbsolutePath(absPath)) {
		absPath = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/") + path);
		qDebug("BreakpadQt: setDumpPath: %s -> %s", qPrintable(path), qPrintable(absPath));
	}
	Q_ASSERT(QDir::isAbsolutePath(absPath));

	QDir().mkpath(absPath);
	Q_ASSERT(QDir().exists(absPath));

#	if defined(Q_OS_WIN32)
		d->handler_->set_dump_path(absPath.toStdWString());
#	else
		d->handler_->set_dump_path(absPath.toStdString());
#	endif
}

void GlobalHandler::setReporter(const QString& reporter)
{
	QString rep = reporter;

	if(!QDir::isAbsolutePath(rep)) {
#		if defined(Q_OS_MAC)
			// TODO(AlekSi) What to do if we are not inside bundle?
			rep = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/../Resources/") + rep);
#		elif defined(Q_OS_LINUX) || defined(Q_OS_WIN32)
			// MAYBE(AlekSi) Better place for Linux? libexec? or what?
			rep = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/") + rep);
#		else
			What is this?!
#		endif

		qDebug("BreakpadQt: setReporter: %s -> %s", qPrintable(reporter), qPrintable(rep));
	}
	Q_ASSERT(QDir::isAbsolutePath(rep));

	// add .exe for Windows if needed
#	if defined(Q_OS_WIN32)
		if(!QDir().exists(rep)) {
			rep += QLatin1String(".exe");
		}
#	endif
//	Q_ASSERT(QDir().exists(rep));

    d->reporter_ = QString::fromLocal8Bit(QFile::encodeName(rep));
}

void GlobalHandler::appendArgument(const QString &systemInfoPath)
{
    GlobalHandlerPrivate::reporterArguments_.append(systemInfoPath);
}

void GlobalHandler::setReportCrashesToSystem(ReportCrashesToSystem report)
{
	d->reportCrashesToSystem_ = report;
}

bool GlobalHandler::writeMinidump()
{
	bool res = d->handler_->WriteMinidump();
	if (res) {
		qDebug("BreakpadQt: writeMinidump() successed.");
	} else {
		qWarning("BreakpadQt: writeMinidump() failed.");
	}
	return res;
}

}	// namespace
