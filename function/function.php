<?php
function executeCode($nodeInfo, $inputIndex, $message)
{
	$code = $nodeInfo["info"]["func"];
	$hg = new \Homegear\Homegear();
	return eval($code);
}

function input($nodeInfo, $inputIndex, $message)
{
	$result = executeCode($nodeInfo, $inputIndex, $message);
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
