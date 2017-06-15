<?php
declare(strict_types=1);

$nodeInfo;

function getNodeData(string $key)
{
	global $nodeInfo;
	return \Homegear\Homegear::getNodeData($nodeInfo['id'], $key);
}

function setNodeData(string $key, $value)
{
	global $nodeInfo;
	\Homegear\Homegear::setNodeData($nodeInfo['id'], $key, $value);
}

function output(int $outputIndex, array $message)
{
	global $nodeInfo;
	\Homegear\Homegear::nodeOutput($nodeInfo['id'], $outputIndex, $message);
}

class HomegearNode extends HomegearNodeBase
{

private $hg = NULL;
private $nodeInfo = NULL;

public function __construct()
{
	$this->hg = new \Homegear\Homegear();
}

public function executeCode(int $inputIndex, array $message)
{
	$code = $this->nodeInfo["info"]["func"];
	$hg = new \Homegear\Homegear();
	return eval($code);
}

public function input(array $nodeInfoLocal, int $inputIndex, array $message)
{
	global $nodeInfo;
	$nodeInfo = $nodeInfoLocal;
	$this->nodeInfo = $nodeInfoLocal;
	$result = $this->executeCode($inputIndex, $message);
	if($result)
	{
		if(array_key_exists('payload', $result))
		{
			$this->hg->nodeOutput($this->nodeInfo['id'], 0, $result);
		}
		else
		{
			$wireCount = count($this->nodeInfo['wiresOut']);
			foreach($result as $index => $value)
			{
				if(!$value || !is_numeric($index) || $index >= $wireCount) continue;
				if(array_key_exists('payload', $value))
				{
					$this->hg->nodeOutput($this->nodeInfo['id'], $index, $value);
				}
				else
				{
					foreach($value as $value2)
					{
						if(!$value2) continue;
						$this->hg->nodeOutput($this->nodeInfo['id'], $index, $value2);
					}
				}
			}
		}
	}
}

}
