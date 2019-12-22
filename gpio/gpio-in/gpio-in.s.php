<?php
declare(strict_types=1);

use Homegear\HomegearGpio;
use parallel\{Channel,Runtime,Events,Events\Event};

class SharedData
{
	public $scriptId = 0;
	public $nodeId = "";
    public $gpioIndex = 0;
    public $trueOnly = false;
    public $stop = false;
}

$gpioThread = function(object $sharedData, Channel $homegearChannel)
{
	$hg = new \Homegear\Homegear();
	if($hg->registerThread($sharedData->scriptId) === false)
	{
		$hg->log(2, "Could not register thread.");
		return;
	}

	$events = new Events();
	$events->addChannel($homegearChannel);
	$events->setBlocking(false);

	$gpio = new HomegearGpio();
	$gpio->export($sharedData->gpioIndex);
	$gpio->open($sharedData->gpioIndex);
	$gpio->setDirection($sharedData->gpioIndex, HomegearGpio::DIRECTION_IN);
	$gpio->setEdge($sharedData->gpioIndex, HomegearGpio::EDGE_BOTH);
	$result = $gpio->poll($sharedData->gpioIndex, 1);
	while(true)
	{
		$result = $gpio->poll($this->sharedData->gpioIndex, 1000, true);
		if($result == -1)
		{
			$hg->log(2, "Error reading GPIO.");
			$gpio->close($sharedData->gpioIndex);
			sleep(1);
			$gpio->open($sharedData->gpioIndex);
			$gpio->setDirection($sharedData->gpioIndex, HomegearGpio::DIRECTION_IN);
			$gpio->setEdge($sharedData->gpioIndex, HomegearGpio::EDGE_BOTH);
		}
		else if($result == -2)
		{
			//Timeout
		}
		else if($result || (!$result && !$sharedData->trueOnly)) $hg->nodeOutput($this->sharedData->nodeId, 0, array('payload' => (bool)$result));

		$event = NULL;
		do
		{
			$event = $events->poll();
			if($event && $event->source == 'gpioHomegearChannelNode'.$sharedData->nodeId)
			{
				$events->addChannel($homegearChannel);
				if($event->type == Event\Type::Read)
				{
					if(is_array($event->value) && count($event->value) > 0)
					{
						if($event->value['name'] == 'stop') return true; //Stop
					}
				}
				else if($event->type == Event\Type::Close) return true; //Stop
			}
		}
		while($event);
	}
	$gpio->close($this->sharedData->gpioIndex);
};

class HomegearNode extends HomegearNodeBase
{

private $hg = NULL;
private $nodeInfo = NULL;
private $sharedData = NULL;
private $gpioRuntime = NULL;
private $gpioFuture = NULL;
private $gpioHomegearChannel = NULL; //Channel to pass Homegear events to gpio thread

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
	return true;
}

public function start() : bool
{
	$this->sharedData = new SharedData();
	$this->sharedData->scriptId = $this->hg->getScriptId();
	$this->sharedData->gpioIndex = (int)$this->nodeInfo['info']['index'];
	$this->sharedData->nodeId = $this->nodeInfo['id'];
	$this->sharedData->trueOnly = (bool)$this->nodeInfo['info']['trueonly'];
	
	$this->gpioRuntime = new Runtime();
	$this->gpioHomegearChannel = Channel::make('gpioHomegearChannelNode'$this->sharedData->nodeId., Channel::Infinite);

	global $gpioThread;
	$this->gpioFuture = $this->gpioRuntime->run($gpioThread, [$this->sharedData, $this->gpioHomegearChannel]);

	return true;
}

public function stop()
{
	if($this->gpioHomegearChannel) $this->gpioHomegearChannel->send(['name' => 'stop', 'value' => true]);
}

public function waitForStop()
{
	if($this->gpioFuture)
	{
		$this->gpioFuture->value();
		$this->gpioFuture = NULL;
	}

	if($this->gpioHomegearChannel)
	{
		$this->gpioHomegearChannel->close();
		$this->gpioHomegearChannel = NULL;
	}

	if($this->gpioRuntime)
	{
		$this->gpioRuntime->close();
		$this->gpioRuntime = NULL;
	}
}

}
