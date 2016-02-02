<?hh //strict
  namespace nova\video;

  use nova\util\Config;
  use nova\io\FileUtils;
  use nova\video\exception\VideoFileException;
  use nova\video\exception\VideoException;

  class Video {
    private ?string $filePath;
    private array<string, mixed> $info = array();

    public function __construct(?string $filePath = null) {
      $this->filePath = $filePath;
    }

    public function setFilePath(string $filePath) : void {
      $this->filePath = $filePath;
    }

    public function getFilePath() : string {
      if($this->filePath === null) {
        throw new VideoFileException("File Path is null.");
      }

      return $this->filePath;
    }

    private function loadInfo() : void {
      if(!is_file($this->getFilePath())) {
        throw new VideoFileException("File not found : ".$this->getFilePath());
      }

      $info = $this->run("ffprobe -print_format json -show_format -show_streams ".$this->getFilePath());
      $this->info = json_decode($info["result"],true);
      if(!is_array($this->info)) {
        throw new VideoFileException("failed to get infomation from ".$this->getFilePath());
      }
    }

    public function getInfo() : array<string, mixed> {
      $this->loadInfo();
      return $this->info;
    }

    public function makeThumbnail(int $second = 1) : string {
      if($second > 60) {
        throw new \InvalidArgumentException("second must be fewer than 60 second.");
      }
      $destFile = FileUtils::getTempFilePath().".png";
      $info = $this->run("ffmpeg -i ".$this->getFilePath()." -ss 00:00:".sprintf("%02d",$second)." -vframes 1 {$destFile}");

      if(!is_file($destFile) || filesize($destFile)<=0) {
        unlink($destFile);
        throw new VideoFileException("failed to make thumbnail.");
      }

      return $destFile;
    }

    public function getDuration() : int {
      $this->loadInfo();
      $format = $this->info["format"];
      $duration = is_array($format) ? intval($format["duration"]) : 0;
      return $duration;
    }

    public function getSize() : Vector<int> {
      $this->loadInfo();

      $streams = $this->info["streams"];
      if(is_array($streams)) {
        foreach($streams as $k=>$v) {
          if($v["codec_type"] == "video") {
            return Vector{$v["width"],$v["height"]};
          }
        }
      }
      return Vector{0,0};
    }

    public function getCodecName() : CodecName {
      $this->loadInfo();

      $video = NULL;
      $audio = NULL;
      $streams = $this->info["streams"];
      if(is_array($streams)) {
        foreach($streams as $k=>$v) {
          if($v["codec_type"] == "video") {
            $video = $v["codec_name"];
          } else if($v["codec_type"] == "audio") {
            $audio = $v["codec_name"];
          }
        }
      }

      return Map {
        "video" => $video,
        "audio" => $audio
      };
    }

    public function isWebUseable(?CodecName $codecName = NULL) : bool {
      if($codecName === NULL) $codecName = $this->getCodecName();

      $video = $codecName["video"];
      $audio = $codecName["audio"];
      if($video === NULL) return false;
      if(!in_array($video, Array("mpeg4","h264"))) return false;

      if($audio !== NULL) {
        if(!in_array($audio, Array("aac","mp3"))) return false;
      }

      return true;
    }

    public function run(string $opt) : Map<string, string> {

      $path = Config::getInstance()->getFFMpegPath();
      $cmd = $path."/{$opt}";
      $pipes = Array();
      $descriptors = Array(
        0 => Array("pipe", "r"),
        1 => Array("pipe", "w"),
        2 => Array("pipe", "w")
      );

      $proc = proc_open($cmd, $descriptors, $pipes);
      $result = "";
      $error = "";
      if(is_resource($proc)) {
        while($str = fgets($pipes[1], 1024)) {
          $result .= $str;
        }

        while($str = fgets($pipes[2], 1024)) {
          $error .= $str;
        }

        fclose($pipes[0]);
        fclose($pipes[1]);
        fclose($pipes[2]);
        proc_close($proc);
      } else {
        throw new VideoException("failed to open process : ".$cmd);
      }

      return Map {
        "result" => $result,
        "error" => $error
      };
    }

    public function makeGifPalette(string $src, string $dest, int $maxWidth = 450, int $start = 1, int $duration = 15, int $fps = 8) : string {
      $palettePath = FileUtils::getTempFilePath().".png";
      $info = $this->run("ffmpeg -ss $start -t $duration -i $src -vf \"fps={$fps},scale='min(trunc(iw/2)*2,{$maxWidth})':trunc(ow/a/2)*2:flags=lanczos,palettegen\" -y $palettePath");

      if(!is_file($palettePath)) {
        throw new VideoException("failed to create palette file.");
      }

      return $palettePath;
    }

    public function video2gif(?string $dest = NULL, int $maxWidth = 450, int $start = 0, int $duration = 20, int $fps = 8) : string {
      if($dest === NULL) {
        $dest = FileUtils::getTempFilePath().".gif";
      }

      $palettePath = $this->makeGifPalette($this->getFilePath(), $dest, $maxWidth, $start, $duration, $fps);
      $info = $this->run("ffmpeg -ss $start -t $duration -i ".$this->getFilePath()." -i $palettePath -lavfi \"fps={$fps},scale='min(trunc(iw/2)*2,{$maxWidth})':trunc(ow/a/2)*2:flags=lanczos [x]; [x][1:v] paletteuse\" -y $dest");
      unlink($palettePath);

      if(!is_file($dest)) {
        throw new VideoException("failed to converting file : ".$info["error"]);
      }

      return $dest;
    }

    public function gif2video(string $src, ?string $dest = NULL, int $maxWidth = 800, string $maxBitrate = "3000K") : string {
      if($dest === NULL) {
        $dest = FileUtils::getTempFilePath().".mp4";
      }

      $info = $this->run("ffmpeg -f gif -i {$src} -b:v {$maxBitrate} -c:v h264 -profile:v baseline -level 3.0 -pix_fmt yuv420p -lavfi \"scale='min(trunc(iw/2)*2,{$maxWidth})':trunc(ow/a/2)*2:flags=lanczos\" -y {$dest}");


      if(!is_file($dest)) {
        throw new VideoException("failed to converting file.");
      }

      return $dest;
    }
  }
