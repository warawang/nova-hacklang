<?hh //strict
namespace nova\util;

class Stopwatch {
  private float $ms;

  public function __construct() {
    $this->ms = microtime(true);
  }

  public function tick(string $task = "Stopwatch tick") {
    $tickMs = microtime(true);
    echo($task . " : " . round($tickMs-$this->ms,3)."s \n");
    $this->ms = $tickMs;
  }

}
