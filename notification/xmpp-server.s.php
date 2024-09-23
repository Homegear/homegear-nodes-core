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
	    $this->stop();
    }

    //Init is called when the node is created directly after the constructor
    public function init(array $nodeInfo) : bool
    {
	    //Write log entry to flows log
	    $this->nodeInfo = $nodeInfo;
	    return true; //True means: "init" was successful. When "false" is returned, the node will be unloaded.
    }

    //Start is called when all nodes are initialized
    public function start() : bool
    {
    	return true; //True means: "start" was successful
    }

    public function waitForStop()
    {
    }
    
    //Stop is called when node is unloaded directly before the destructor. You can still call RPC functions here, but you
    //shouldn't try to access other nodes anymore.
    public function stop()
    {
    }

    public function input(array $nodeInfo, int $inputIndex, array $message)
    {
    }

    public function getConfigParameterIncoming($data = false)
    {
        $data = array('server' => $this->nodeInfo['info']['server'], 'port' => $this->nodeInfo['info']['port'], 'username' => $this->nodeInfo['info']['user'], 'password' => $this->getNodeData('password'));
        return $data;
    }

}
