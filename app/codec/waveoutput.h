#ifndef WAVEAUDIO_H
#define WAVEAUDIO_H

#include <QByteArray>
#include <QFile>

#include "audio/sampleformat.h"
#include "common/constructors.h"
#include "render/audioparams.h"

class WaveOutput
{
public:
  WaveOutput(const QString& f,
             const AudioRenderingParams& params);

  ~WaveOutput();

  DISABLE_COPY_MOVE(WaveOutput)

  bool open();

  void write(const QByteArray& bytes);
  void write(const char* bytes, int length);

  void close();

  const int& data_length() const;

  const AudioRenderingParams& params() const;

private:
  template<typename T>
  void write_int(QFile* file, T integer);

  void switch_endianness(QByteArray &array);

  QFile file_;

  AudioRenderingParams params_;

  int data_length_;

};

#endif // WAVEAUDIO_H
