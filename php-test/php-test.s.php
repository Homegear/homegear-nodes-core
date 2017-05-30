<?php
declare(strict_types=1);

class HomegearNode extends HomegearNodeBase
{

private $hg;
private $nodeInfo;

function __construct()
{
	$this->hg = new \Homegear\Homegear();
}

public function init(array $nodeInfo) : bool
{
	$this->nodeInfo = $nodeInfo;
	return true; //True means: "init" was successful. When "false" is returned, the node will be unloaded.
}

public function start() : bool
{
	$this->log(4, "start");
	return true; //True means: "start" was successful
}

public function stop()
{
	$this->hg->log(4, "stop");
}

private function executeCode($nodeInfo, $inputIndex, $message)
{
	$code = $nodeInfo["info"]["func"];
	$this->hg = new \Homegear\Homegear();
	return eval($code);
}

public function input(array $nodeInfo, int $inputIndex, array $message)
{
	$result = executeCode($nodeInfo, $inputIndex, $message);
	if($result)
	{
		if(array_key_exists('payload', $result))
		{
			$this->hg->nodeOutput($nodeInfo['id'], 0, $result);
		}
		else
		{
			$wireCount = count($nodeInfo['wiresOut']);
			foreach($result as $index => $value)
			{
				if(!$value || !is_numeric($index) || $index >= $wireCount) continue;
				if(array_key_exists('payload', $value))
				{
					$this->hg->nodeOutput($nodeInfo['id'], $index, $value);
				}
				else
				{
					foreach($value as $value2)
					{
						if(!$value2) continue;
						$this->hg->nodeOutput($nodeInfo['id'], $index, $value2);
					}
				}
			}
		}
	}
}

}