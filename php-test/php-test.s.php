<?php
class HomegearNode
{

private $hg;
private $nodeInfo;

function __construct($nodeInfo)
{
	$this->hg = new \Homegear\Homegear();
	$this->nodeInfo = $nodeInfo;
	$this->hg->log(4, "Constructor");
}

public function init()
{
	$this->hg->log(4, "init ".print_v($this->nodeInfo, true));
}

public function start()
{

}

public function stop()
{

}

private function executeCode($nodeInfo, $inputIndex, $message)
{
	$code = $nodeInfo["info"]["func"];
	$this->hg = new \Homegear\Homegear();
	return eval($code);
}

public function input($nodeInfo, $inputIndex, $message)
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