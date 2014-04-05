#include <QDateTime>
#include <QtEndian>
#include <QDir>
#include <qplatformdefs.h>
#include <zlib.h>

#include <cassert>

#include "zipwriter.h"

#if defined(Q_OS_WIN)
#  undef S_IFREG
#  define S_IFREG 0100000
#  ifndef S_IFDIR
#    define S_IFDIR 0040000
#  endif
#  ifndef S_ISDIR
#    define S_ISDIR(x) ((x) & S_IFDIR) > 0
#  endif
#  ifndef S_ISREG
#    define S_ISREG(x) ((x) & 0170000) == S_IFREG
#  endif
#  define S_IFLNK 020000
#  define S_ISLNK(x) ((x) & S_IFLNK) > 0
#  ifndef S_IRUSR
#    define S_IRUSR 0400
#  endif
#  ifndef S_IWUSR
#    define S_IWUSR 0200
#  endif
#  ifndef S_IXUSR
#    define S_IXUSR 0100
#  endif
#  define S_IRGRP 0040
#  define S_IWGRP 0020
#  define S_IXGRP 0010
#  define S_IROTH 0004
#  define S_IWOTH 0002
#  define S_IXOTH 0001
#endif

static inline uint readUInt(const uchar *data)
{
    return (data[0]) + (data[1]<<8) + (data[2]<<16) + (data[3]<<24);
}

static inline ushort readUShort(const uchar *data)
{
    return (data[0]) + (data[1]<<8);
}

static inline void writeUInt(uchar *data, uint i)
{
    data[0] = i & 0xff;
    data[1] = (i>>8) & 0xff;
    data[2] = (i>>16) & 0xff;
    data[3] = (i>>24) & 0xff;
}

static inline void writeUShort(uchar *data, ushort i)
{
    data[0] = i & 0xff;
    data[1] = (i>>8) & 0xff;
}

static inline void copyUInt(uchar *dest, const uchar *src)
{
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    dest[3] = src[3];
}

static inline void copyUShort(uchar *dest, const uchar *src)
{
    dest[0] = src[0];
    dest[1] = src[1];
}

static void writeMSDosDate(uchar *dest, const QDateTime& dt)
{
    if (dt.isValid()) {
        quint16 time =
            (dt.time().hour() << 11)    // 5 bit hour
            | (dt.time().minute() << 5)   // 6 bit minute
            | (dt.time().second() >> 1);  // 5 bit double seconds

        dest[0] = time & 0xff;
        dest[1] = time >> 8;

        quint16 date =
            ((dt.date().year() - 1980) << 9) // 7 bit year 1980-based
            | (dt.date().month() << 5)           // 4 bit month
            | (dt.date().day());                 // 5 bit day

        dest[2] = char(date);
        dest[3] = char(date >> 8);
    } else {
        dest[0] = 0;
        dest[1] = 0;
        dest[2] = 0;
        dest[3] = 0;
    }
}

static quint32 permissionsToMode(QFile::Permissions perms)
{
    quint32 mode = 0;
    if (perms & QFile::ReadOwner)
        mode |= S_IRUSR;
    if (perms & QFile::WriteOwner)
        mode |= S_IWUSR;
    if (perms & QFile::ExeOwner)
        mode |= S_IXUSR;
    if (perms & QFile::ReadUser)
        mode |= S_IRUSR;
    if (perms & QFile::WriteUser)
        mode |= S_IWUSR;
    if (perms & QFile::ExeUser)
        mode |= S_IXUSR;
    if (perms & QFile::ReadGroup)
        mode |= S_IRGRP;
    if (perms & QFile::WriteGroup)
        mode |= S_IWGRP;
    if (perms & QFile::ExeGroup)
        mode |= S_IXGRP;
    if (perms & QFile::ReadOther)
        mode |= S_IROTH;
    if (perms & QFile::WriteOther)
        mode |= S_IWOTH;
    if (perms & QFile::ExeOther)
        mode |= S_IXOTH;
    return mode;
}

static int inflate(Bytef *dest, ulong *destLen, const Bytef *source, ulong sourceLen)
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    if ((uLong)stream.avail_in != sourceLen)
        return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen)
        return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    err = inflateInit2(&stream, -MAX_WBITS);
    if (err != Z_OK)
        return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}

static int deflate (Bytef *dest, ulong *destLen, const Bytef *source, ulong sourceLen)
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (err != Z_OK) return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}

static QFile::Permissions modeToPermissions(quint32 mode)
{
    QFile::Permissions ret;
    if (mode & S_IRUSR)
        ret |= QFile::ReadOwner;
    if (mode & S_IWUSR)
        ret |= QFile::WriteOwner;
    if (mode & S_IXUSR)
        ret |= QFile::ExeOwner;
    if (mode & S_IRUSR)
        ret |= QFile::ReadUser;
    if (mode & S_IWUSR)
        ret |= QFile::WriteUser;
    if (mode & S_IXUSR)
        ret |= QFile::ExeUser;
    if (mode & S_IRGRP)
        ret |= QFile::ReadGroup;
    if (mode & S_IWGRP)
        ret |= QFile::WriteGroup;
    if (mode & S_IXGRP)
        ret |= QFile::ExeGroup;
    if (mode & S_IROTH)
        ret |= QFile::ReadOther;
    if (mode & S_IWOTH)
        ret |= QFile::WriteOther;
    if (mode & S_IXOTH)
        ret |= QFile::ExeOther;
    return ret;
}

static QDateTime readMSDosDate(const uchar *src)
{
    uint dosDate = readUInt(src);
    quint64 uDate;
    uDate = (quint64)(dosDate >> 16);
    uint tm_mday = (uDate & 0x1f);
    uint tm_mon =  ((uDate & 0x1E0) >> 5);
    uint tm_year = (((uDate & 0x0FE00) >> 9) + 1980);
    uint tm_hour = ((dosDate & 0xF800) >> 11);
    uint tm_min =  ((dosDate & 0x7E0) >> 5);
    uint tm_sec =  ((dosDate & 0x1f) << 1);

    return QDateTime(QDate(tm_year, tm_mon, tm_mday), QTime(tm_hour, tm_min, tm_sec));
}

struct LocalFileHeader
{
    uchar signature[4]; //  0x04034b50
    uchar version_needed[2];
    uchar general_purpose_bits[2];
    uchar compression_method[2];
    uchar last_mod_file[4];
    uchar crc_32[4];
    uchar compressed_size[4];
    uchar uncompressed_size[4];
    uchar file_name_length[2];
    uchar extra_field_length[2];
};

struct DataDescriptor
{
    uchar crc_32[4];
    uchar compressed_size[4];
    uchar uncompressed_size[4];
};

struct CentralFileHeader
{
    uchar signature[4]; // 0x02014b50
    uchar version_made[2];
    uchar version_needed[2];
    uchar general_purpose_bits[2];
    uchar compression_method[2];
    uchar last_mod_file[4];
    uchar crc_32[4];
    uchar compressed_size[4];
    uchar uncompressed_size[4];
    uchar file_name_length[2];
    uchar extra_field_length[2];
    uchar file_comment_length[2];
    uchar disk_start[2];
    uchar internal_file_attributes[2];
    uchar external_file_attributes[4];
    uchar offset_local_header[4];
    LocalFileHeader toLocalHeader() const;
};

struct EndOfDirectory
{
    uchar signature[4]; // 0x06054b50
    uchar this_disk[2];
    uchar start_of_directory_disk[2];
    uchar num_dir_entries_this_disk[2];
    uchar num_dir_entries[2];
    uchar directory_size[4];
    uchar dir_start_offset[4];
    uchar comment_length[2];
};

struct FileHeader
{
    CentralFileHeader h;
    QByteArray file_name;
    QByteArray extra_field;
    QByteArray file_comment;
};

LocalFileHeader CentralFileHeader::toLocalHeader() const
{
    LocalFileHeader h;
    writeUInt(h.signature, 0x04034b50);
    copyUShort(h.version_needed, version_needed);
    copyUShort(h.general_purpose_bits, general_purpose_bits);
    copyUShort(h.compression_method, compression_method);
    copyUInt(h.last_mod_file, last_mod_file);
    copyUInt(h.crc_32, crc_32);
    copyUInt(h.compressed_size, compressed_size);
    copyUInt(h.uncompressed_size, uncompressed_size);
    copyUShort(h.file_name_length, file_name_length);
    copyUShort(h.extra_field_length, extra_field_length);
    return h;
}

ZipWriter::ZipWriter(const QString &fileName)
{
    zipFile = new QFile(fileName);
    zipFile->open(QIODevice::WriteOnly);
    if (zipFile->error() == QFile::NoError)
        status = ZipWriter::NoError;
    else
    {
        if(zipFile->error() == QFile::WriteError)
            status = ZipWriter::FileWriteError;
        else if (zipFile->error() == QFile::OpenError)
            status = ZipWriter::FileOpenError;
        else if (zipFile->error() == QFile::PermissionsError)
            status = ZipWriter::FilePermissionsError;
        else
            status = ZipWriter::FileError;
    }
    start_of_directory = 0;
}

ZipWriter::~ZipWriter()
{
}

void ZipWriter::addFile(const QString &fileName, const QByteArray &data)
{
    addEntry(ZipWriter::File, fileName, data);
}

void ZipWriter::addDirectory(const QString &dirName)
{
    QString name = dirName;
    if (!name.endsWith(QDir::separator()))
        name.append(QDir::separator());
    addEntry(ZipWriter::Directory, name, QByteArray());
}

void ZipWriter::addSymLink(const QString &fileName, const QString &destination)
{
    addEntry(ZipWriter::Symlink, fileName, QFile::encodeName(destination));
}

void ZipWriter::addEntry(EntryType type, const QString &fileName, const QByteArray &content)
{
    if (!(zipFile->isOpen()||zipFile->open(QIODevice::WriteOnly)))
    {
        status = ZipWriter::FileOpenError;
        return;
    }
    zipFile->seek(start_of_directory);

    ZipWriter::CompressionPolicy c = cp;
    if (cp == ZipWriter::AutoCompress)
    {
        if (content.length() < 64)
            c = ZipWriter::NeverCompress;
        else
            c = ZipWriter::AlwaysCompress;
    }

    FileHeader header;
    memset(&header.h, 0, sizeof(CentralFileHeader));
    writeUInt(header.h.signature, 0x02014b50);

    writeUShort(header.h.version_needed, 0x14);
    writeUInt(header.h.uncompressed_size, content.length());
    writeMSDosDate(header.h.last_mod_file, QDateTime::currentDateTime());
    QByteArray data = content;
    if (c == ZipWriter::AlwaysCompress)
    {
        writeUShort(header.h.compression_method, 8);

        ulong len = content.length();
        len += (len >> 12) + (len >> 14) + 11;
        int res;
        do
        {
            data.resize(len);
            res = deflate((uchar *)data.data(), &len, (const uchar*) content.constData(), content.length());

            switch(res)
            {
                case Z_OK:
                    data.resize(len);
                    break;
                case Z_MEM_ERROR:
                    qWarning("ZipWriter: Not Enough memory to compress the file, skipping");
                    data.resize(0);
                    break;
                case Z_BUF_ERROR:
                    len *= 2;
                    break;
            }
        } while(res == Z_BUF_ERROR);
    }

    writeUInt(header.h.compressed_size, data.length());
    uint crc_32 = ::crc32(0, 0, 0);
    crc_32 = ::crc32(crc_32, (const uchar*)content.constData(), content.length());
    writeUInt(header.h.crc_32, crc_32);

    header.file_name = fileName.toLocal8Bit();
    if(header.file_name.size() > 0xffff)
    {
        qWarning("Filename too long");
        header.file_name = header.file_name.left(0xffff);
    }

    writeUShort(header.h.file_name_length, header.file_name.length());

    writeUShort(header.h.version_made, 3 << 8);

    quint32 mode = permissionsToMode(permissions);
    switch(type)
    {
        case File: mode |= S_IFREG; break;
        case Directory: mode |= S_IFDIR; break;
        case Symlink: mode |= S_IFLNK; break;
    }
    writeUInt(header.h.external_file_attributes, mode << 16);
    writeUInt(header.h.offset_local_header, start_of_directory);

    fileHeaders.append(header);

    LocalFileHeader h = header.h.toLocalHeader();
    zipFile->write((const char*)&h, sizeof(LocalFileHeader));
    zipFile->write(header.file_name);
    zipFile->write(data);
    start_of_directory = zipFile->pos();

}

bool ZipWriter::isWritable() const
{
    return zipFile->isWritable();
}

bool ZipWriter::exists() const
{
    return zipFile->exists();
}

ZipWriter::Status ZipWriter::getStatus() const
{
    return status;
}

void ZipWriter::setCompressionPolicy(CompressionPolicy policy)
{
    cp = policy;
}

ZipWriter::CompressionPolicy ZipWriter::compressionPolicy() const
{
    return cp;
}

void ZipWriter::setCreationPermissions(QFile::Permissions permissions)
{
    this->permissions = permissions;
}

QFile::Permissions ZipWriter::creationPermissions() const
{
    return permissions;
}

void ZipWriter::close()
{
    if( !(zipFile->openMode() &QIODevice::WriteOnly) )
    {
        zipFile->close();
        delete zipFile;
        return;
    }

    zipFile->seek(start_of_directory);

    for (int i=0; i<fileHeaders.size(); ++i)
    {
        const FileHeader &header = fileHeaders.at(i);
        zipFile->write((const char*)&header.h, sizeof(CentralFileHeader));
        zipFile->write(header.file_name);
        zipFile->write(header.extra_field);
        zipFile->write(header.file_comment);
    }
    int dir_size = zipFile->pos() - start_of_directory;
    EndOfDirectory eod;
    memset(&eod, 0, sizeof(EndOfDirectory));
    writeUInt(eod.signature, 0x06054b50);
    writeUShort(eod.num_dir_entries_this_disk, fileHeaders.size());
    writeUShort(eod.num_dir_entries, fileHeaders.size());
    writeUInt(eod.directory_size, dir_size);
    writeUInt(eod.dir_start_offset, start_of_directory);
    writeUShort(eod.comment_length, 0);//no comment

    zipFile->write((const char*)&eod, sizeof(EndOfDirectory));
    zipFile->write(QByteArray());
    zipFile->close();
    delete zipFile;
}
