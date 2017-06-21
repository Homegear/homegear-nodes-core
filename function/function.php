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

function getFlowData(string $key)
{
	global $nodeInfo;
	return \Homegear\Homegear::getNodeData($nodeInfo['info']['z'], $key);
}

function setFlowData(string $key, $value)
{
	global $nodeInfo;
	\Homegear\Homegear::setNodeData($nodeInfo['info']['z'], $key, $value);
}

function getGlobalData(string $key)
{
	global $nodeInfo;
	return \Homegear\Homegear::getNodeData('global', $key);
}

function setGlobalData(string $key, $value)
{
	global $nodeInfo;
	\Homegear\Homegear::setNodeData('global', $key, $value);
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
	try
	{
		$code = $this->nodeInfo["info"]["func"];
		$hg = new \Homegear\Homegear();
		return eval($code);
	}
	catch(Exception $e)
	{
		$this->log(2, $e->message);
	}
	return NULL;
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
