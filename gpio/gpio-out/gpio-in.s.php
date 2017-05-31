<?php
declare(strict_types=1);

use Homegear\HomegearGpio as HomegearGpio;

class SharedData extends Threaded
{
    public $gpioIndex = 0;

    public function run() {}
}

class GpioThread extends Thread
{
    private $sharedData;

	public function __construct($sharedData)
	{
		$this->sharedData = $sharedData;
	}

	public function run()
	{
		$hg = new \Homegear\Homegear();
		if($hg->registerThread($this->sharedData->scriptId) === false)
		{
			$hg->log(2, "Could not register thread.");
			return;
		}
		$hg->subscribePeer($this->sharedData->peerId);
		while(!$hg->shuttingDown() && $hg->peerExists($this->sharedData->peerId))
		{
			$result = $hg->pollEvent();
			if($result["TYPE"] == "event" && $result["PEERID"] == $this->sharedData->peerId)
			{
				if($result["VARIABLE"] == "REQUEST")
				{
					$this->sharedData->interval = 0;
					$this->synchronized(function($thread){ $thread->notify(); }, $this);
				}
			}
			else if($result["TYPE"] == "updateDevice" && $result["PEERID"] == $this->sharedData->peerId)
			{
				$this->sharedData->interval = 0;
				$this->synchronized(function($thread){ $thread->notify(); }, $this);
			}
		}
	}
}

class HomegearNode extends HomegearNodeBase
{

private $hg;
private $nodeInfo;
private $sharedData;

function __construct()
{
	$this->hg = new \Homegear\Homegear();
}

function __destruct()
{
}

public function init(array $nodeInfo) : bool
{
	$this->nodeInfo = $nodeInfo;
	return true;
}

public function start() : bool
{
	$this->sharedData = new SharedData();
	$this->sharedData->gpioIndex = (int)$this->nodeInfo['info']['index'];
	$this->log(4, $this->sharedData->gpioIndex);
	return true;
}

public function stop()
{
}

}