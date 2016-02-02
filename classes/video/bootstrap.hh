<?hh //strict
  namespace nova\video {
    type CodecName = Map<string, ?string>;

    enum VideoExtension : int {
      MP4 = 0;
      AGif = 1;
    }
  }
