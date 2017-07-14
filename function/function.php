<?php
declare(strict_types=1);

$nodeObject;

function getNodeData(string $key)
{
	global $nodeObject;
	return $nodeObject->getNodeData($key);
}

function setNodeData(string $key, $value)
{
	global $nodeObject;
	return $nodeObject->setNodeData($key, $value);
}

function getFlowData(string $key)
{
	global $nodeObject;
	return $nodeObject->getFlowData($key);
}

function setFlowData(string $key, $value)
{
	global $nodeObject;
	return $nodeObject->setFlowData($key, $value);
}

function getGlobalData(string $key)
{
	global $nodeObject;
	return $nodeObject->getGlobalData($key);
}

function setGlobalData(string $key, $value)
{
	global $nodeObject;
	return $nodeObject->setGlobalData($key, $value);
}

function output(int $outputIndex, array $message)
{
	global $nodeObject;
	return $nodeObject->output($outputIndex, $message);
}

class HomegearNode extends HomegearNodeBase
{

private $hg = NULL;
private $nodeInfo = NULL;

public function __construct()
{
	global $nodeObject;
	$nodeObject = $this;
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
	$this->nodeInfo = $nodeInfoLocal;
	$result = $this->executeCode($inputIndex, $message);
	if($result)
	{
		if(array_key_exists('payload', $result))
		{
			$this->output(0, $result);
		}
		else
		{
			$wireCount = count($this->nodeInfo['wiresOut']);
			foreach($result as $index => $value)
			{
				if(!$value || !is_numeric($index) || $index >= $wireCount) continue;
				if(array_key_exists('payload', $value))
				{
					$this->output($index, $value);
				}
				else
				{
					foreach($value as $value2)
					{
						if(!$value2) continue;
						$this->output($index, $value2);
					}
				}
			}
		}
	}
}

}
