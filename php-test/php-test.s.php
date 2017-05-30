<?php
declare(strict_types=1);

//Class definition is fixed. Do not change it.
class HomegearNode extends HomegearNodeBase
{

private $hg;
private $nodeInfo;

function __construct()
{
	//Create a new Homegear object to access Homegear methods
	$this->hg = new \Homegear\Homegear();
}

function __destruct()
{
}

//Init is called when the node is created directly after the constructor
public function init(array $nodeInfo) : bool
{
	//Write log entry to flows log
	$this->log(4, "init"); 
	$this->nodeInfo = $nodeInfo;
	return true; //True means: "init" was successful. When "false" is returned, the node will be unloaded.
}

//Start is called when all nodes are initialized
public function start() : bool
{
	$this->log(4, "start");
	return true; //True means: "start" was successful
}

//Executed when all config nodes are started. If you need to get settings from configuration nodes that only can return
//settings after "start" was called, do that here.
public function configNodesStarted()
{
}

//Stop is called when node is unloaded directly before the destructor. You can still call RPC functions here, but you
//shouldn't try to access other nodes anymore.
public function stop()
{
	$this->hg->log(4, "stop");
}

public function input(array $nodeInfo, int $inputIndex, array $message)
{
	
}

}