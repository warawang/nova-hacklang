<?hh //strict
  namespace nova\util;

	class Batch {
    private int $timeout;
    private string $nameOption;
    private string $batchName;

    private int $startTS = 0;
    private string $chkrunPath = "";
		private string $chktimePath = "";

    public function __construct(int $timeout = 3600, string $nameOption = "") {
      //UNSAFE
      $batchPath = pathinfo($_SERVER['PHP_SELF']);

      $this->timeout = $timeout;
      $this->nameOption = $nameOption;
      $this->batchName = $batchPath['filename'] . ($this->nameOption ? "_".$this->nameOption : "");
    }

		//잡 시작시 호출
		public function in() : bool {
			$batchPID = posix_getpid();

			$this->startTS = time();
			$this->chkrunPath = "/tmp/chkrun.$this->batchName";
			$this->chktimePath = "/tmp/chktime.$this->batchName";

			if(is_file($this->chkrunPath)) {
				$beforePID = file_get_contents($this->chkrunPath);

				$fileTS = filemtime($this->chkrunPath);
        //타임아웃 시간이 있고, 현재 타임아웃 걸림, 과거 프로세스 제거
        if($this->timeout > 0 && time()-$fileTS > $this->timeout)  {
					//이전 프로세스 타임아웃
					posix_kill((int)$beforePID,1);
				} else {
					//아직 타임아웃 아님
					if(posix_getsid($beforePID)) {
						//이전 프로세스 실행중
						echo("\033[01;31m'{$this->batchName}' is still running.\033[0m\n");
						return false;
					} else {
						//이전 프로세스 구동중 아님
						echo("\033[01;33m'{$this->batchName}' restart.\033[0m\n");
					}
				}
			} else {
        echo("\033[01;32m'{$this->batchName}' start.\033[0m\n");
      }

			//pid 기록 밎 pid 기록하여 체크아웃때 활용
			file_put_contents($this->chkrunPath,$batchPID);
      return true;
		}

		//잡 종료 호출
		public function out() : void {
			unlink($this->chkrunPath);
			file_put_contents($this->chktimePath,time()-$this->startTS);
      echo("\033[01;32m'{$this->batchName}' complete.\033[0m\n");
		}
	}
