#ifndef ZIPWRITER_H
#define ZIPWRITER_H

#include <QFile>

struct FileHeader;

class ZipWriter
{
public:
    ZipWriter(const QString &fileName);
    ~ZipWriter();

    bool isWritable() const;
    bool exists() const;

    enum Status {
        NoError,
        FileWriteError,
        FileOpenError,
        FilePermissionsError,
        FileError
    };

    Status getStatus() const;

    enum CompressionPolicy {
        AlwaysCompress,
        NeverCompress,
        AutoCompress
    };

    enum EntryType {
        Directory,
        File,
        Symlink
    };

    void setCompressionPolicy(CompressionPolicy policy);
    CompressionPolicy compressionPolicy() const;

    void setCreationPermissions(QFile::Permissions permissions);
    QFile::Permissions creationPermissions() const;

    void addEntry(EntryType type, const QString &fileName, const QByteArray &content);

    void addFile(const QString &fileName, const QByteArray &data);

    void addDirectory(const QString &dirName);

    void addSymLink(const QString &fileName, const QString &destination);

    void close();
private:
    Status status;
    CompressionPolicy cp;
    QFile::Permissions permissions;
    QFile *zipFile;
    QList<FileHeader> fileHeaders;
    uint start_of_directory;
    Q_DISABLE_COPY(ZipWriter)
};

#endif // ZIPWRITER_H
