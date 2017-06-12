<?php
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
	return \Homegear\Homegear::getNodeData($nodeInfo['z'], $key);
}

function setFlowData(string $key, $value)
{
	global $nodeInfo;
	\Homegear\Homegear::setNodeData($nodeInfo['z'], $key, $value);
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

function executeCode(int $inputIndex, array $message)
{
	global $nodeInfo;
	$code = $nodeInfo["info"]["func"];
	$hg = new \Homegear\Homegear();
	return eval($code);
}

function input(array $localNodeInfo, int $inputIndex, array $message)
{
	global $nodeInfo;
	$nodeInfo = $localNodeInfo;
	$result = executeCode($inputIndex, $message);
	if($result)
	{
		if(array_key_exists('payload', $result))
		{
			\Homegear\Homegear::nodeOutput($nodeInfo['id'], 0, $result);
		}
		else
		{
			$wireCount = count($nodeInfo['wiresOut']);
			foreach($result as $index => $value)
			{
				if(!$value || !is_numeric($index) || $index >= $wireCount) continue;
				if(array_key_exists('payload', $value))
				{
					\Homegear\Homegear::nodeOutput($nodeInfo['id'], $index, $value);
				}
				else
				{
					foreach($value as $value2)
					{
						if(!$value2) continue;
						\Homegear\Homegear::nodeOutput($nodeInfo['id'], $index, $value2);
					}
				}
			}
		}
	}
}
