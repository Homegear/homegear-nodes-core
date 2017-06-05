<?php
declare(strict_types=1);

use Homegear\HomegearGpio as HomegearGpio;

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
	$this->gpioIndex = (int)$this->nodeInfo['info']['index'];
	return true;
}

public function start() : bool
{
	$this->gpio = new HomegearGpio();
	$this->gpio->export($this->gpioIndex);
	$this->gpio->open($this->gpioIndex);
	$this->gpio->setDirection($this->gpioIndex, HomegearGpio::DIRECTION_OUT);

	if($this->nodeInfo['info']['initialtrue']) $this->gpio->set($this->gpioIndex, true);

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
