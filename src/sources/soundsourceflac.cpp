#include "sources/soundsourceflac.h"

#include "metadata/trackmetadatataglib.h"
#include "sources/audiosourceflac.h"

#include <taglib/flacfile.h>

#include <QtDebug>

QList<QString> SoundSourceFLAC::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("flac");
    return list;
}

SoundSourceFLAC::SoundSourceFLAC(QUrl url)
        : SoundSource(url, "flac") {
}

Result SoundSourceFLAC::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    TagLib::FLAC::File f(getLocalFileNameBytes().constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph(f.xiphComment());
    if (xiph) {
        readXiphComment(pMetadata, *xiph);
    } else {
        TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
        if (id3v2) {
            readID3v2Tag(pMetadata, *id3v2);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
    }

    return OK;
}

QImage SoundSourceFLAC::parseCoverArt() const {
    TagLib::FLAC::File f(getLocalFileNameBytes().constData());
    QImage coverArt;
    TagLib::Ogg::XiphComment *xiph(f.xiphComment());
    if (xiph) {
        coverArt = Mixxx::readXiphCommentCover(*xiph);
    }
    if (coverArt.isNull()) {
        TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::List<TagLib::FLAC::Picture*> covers = f.pictureList();
            if (!covers.isEmpty()) {
                std::list<TagLib::FLAC::Picture*>::iterator it = covers.begin();
                TagLib::FLAC::Picture* cover = *it;
                coverArt = QImage::fromData(
                        QByteArray(cover->data().data(), cover->data().size()));
            }
        }
    }
    return coverArt;
}

Mixxx::AudioSourcePointer SoundSourceFLAC::open() const {
    return Mixxx::AudioSourceFLAC::create(getUrl());
}
