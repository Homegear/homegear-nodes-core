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

function output(int $outputIndex, array $msg)
{
	global $nodeObject;
	return $nodeObject->output($outputIndex, $msg);
}

function eventLog(string $message)
{
	global $nodeObject;
	return $nodeObject->frontendEventLog($message);
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

public function executeCode(int $inputIndex, array $msg)
{
	try
	{
            $message = &$msg;
	    $nodeInfo = $this->nodeInfo;
		$code = $this->nodeInfo["info"]["func"];
		if(array_key_exists('info', $this->nodeInfo) && array_key_exists('env', $this->nodeInfo['info']))
		{
			foreach($this->nodeInfo['info']['env'] as $name => $value)
			{
				putenv($name.'='.$value);
			}
		}
		$hg = new \Homegear\Homegear();
		return eval($code);
	}
	catch(Exception $e)
	{
		$this->log(2, $e->getMessage());
	}
	return NULL;
}

public function start() : bool
{
    $this->log(4, "In start()");
    return true;
}

public function stop()
{
    $this->log(4, "In stop()");
}

public function input(array $nodeInfoLocal, int $inputIndex, array $msg)
{
	$this->nodeInfo = $nodeInfoLocal;
	$result = $this->executeCode($inputIndex, $msg);
	if($result)
	{
		if(gettype(array_key_first($result)) === 'string')
		{
			$this->output(0, $result);
		}
		else
		{
			$wireCount = count($this->nodeInfo['wiresOut']);
			foreach($result as $index => $value)
			{
				if(!$value || !is_numeric($index) || $index >= $wireCount) continue;
				if(gettype(array_key_first($value)) === 'string')
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
