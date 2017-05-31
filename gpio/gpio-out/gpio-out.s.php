<?php
declare(strict_types=1);

use Homegear\HomegearGpio as HomegearGpio;

class SharedData extends Threaded
{
	public $scriptId = 0;
	public $nodeId = "";
    public $gpioIndex = 0;
    public $stop = false;

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
		
		$gpio->setEdge($this->sharedData->gpioIndex, HomegearGpio::EDGE_BOTH);
		$result = $gpio->poll($this->sharedData->gpioIndex, 1);
		while(!$this->sharedData->stop)
		{
			$result = $gpio->poll($this->sharedData->gpioIndex, 1000, true);
			if($result == -1)
			{
				$hg->log(2, "Error reading GPIO.");
				$gpio->close($this->sharedData->gpioIndex);
				$this->synchronized(function($thread){ $thread->wait(1000000); }, $this);
				$gpio->open($this->sharedData->gpioIndex);
				$gpio->setDirection($this->sharedData->gpioIndex, HomegearGpio::DIRECTION_IN);
				$gpio->setEdge($this->sharedData->gpioIndex, HomegearGpio::EDGE_BOTH);
			}
			else if($result == -2)
			{
				continue;
			}

			$hg->nodeOutput($this->sharedData->nodeId, 0, array('payload' => $result));
		}
		$gpio->close($this->sharedData->gpioIndex);
	}
}

class HomegearNode extends HomegearNodeBase
{

private $hg = NULL;
private $nodeInfo = NULL;
private $gpioIndex = NULL;
private $gpio = NULL;

function __construct()
{
	$this->hg = new \Homegear\Homegear();
}

function __destruct()
{
	$this->stop();
}

public function init(array $nodeInfo) : bool
{
	$this->nodeInfo = $nodeInfo;
	$this->gpioIndex = (int)$this->nodeInfo['info']['index']
	return true;
}

public function start() : bool
{
	$this->gpio = new HomegearGpio();
	$this->gpio->open($this->gpioIndex);
	$gpio->setDirection($this->gpioIndex, HomegearGpio::DIRECTION_OUT);
	return true;
}

public function stop()
{
	if($this->gpioIndex)
	{
		$this->gpio->close($this->gpioIndex);
		$this->gpioIndex = NULL;
	}
}

public function input(array $nodeInfo, int $inputIndex, array $message)
{
	$this->gpio->set($this->gpioIndex, (bool)$message['payload']);
}

}