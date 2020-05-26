<?php
declare(strict_types=1);

use Homegear\HomegearGpio;
use parallel\{Channel,Runtime,Events,Events\Event};

$gpioThread = function(string $scriptId, string $nodeId, int $gpioIndex, bool $trueOnly, bool $debounce, Channel $homegearChannel)
{
    $hg = new \Homegear\Homegear();
    if($hg->registerThread($scriptId) === false)
    {
        $hg->log(2, "Could not register thread.");
        return;
    }
    $events = new Events();
    $events->addChannel($homegearChannel);
    $events->setBlocking(false);
    $gpio = new HomegearGpio();
    $gpio->export($gpioIndex);
    $gpio->open($gpioIndex);
    $gpio->setDirection($gpioIndex, HomegearGpio::DIRECTION_IN);
    $gpio->setEdge($gpioIndex, HomegearGpio::EDGE_BOTH);
    $result = $gpio->poll($gpioIndex, 1);
    while(true)
    {
        $result = $gpio->poll($gpioIndex, 1000, $debounce);
        if($result == -1)
        {
            $hg->log(2, "Error reading GPIO.");
            $gpio->close($gpioIndex);
            sleep(1);
            $gpio->open($gpioIndex);
            $gpio->setDirection($gpioIndex, HomegearGpio::DIRECTION_IN);
            $gpio->setEdge($gpioIndex, HomegearGpio::EDGE_BOTH);
        }
        else if($result == -2)
        {
            //Timeout
        }
        else if($result || (!$result && !$trueOnly)) $hg->nodeOutput($nodeId, 0, array('payload' => (bool)$result));
        $event = NULL;
        do
        {
            $event = $events->poll();
            if($event && $event->source == 'gpioHomegearChannelNode'.$nodeId)
            {
                $events->addChannel($homegearChannel);
                if($event->type == Event\Type::Read)
                {
                    if(is_array($event->value) && count($event->value) > 0)
                    {
                        if($event->value['name'] == 'stop') return true; //Stop
                    }
                }
                else if($event->type == Event\Type::Close) return true; //Stop
            }
        }
        while($event);
    }
    $gpio->close($gpioIndex);
};

class HomegearNode extends HomegearNodeBase
{
    private $hg = NULL;
    private $nodeInfo = NULL;
    private $gpioRuntime = NULL;
    private $gpioFuture = NULL;
    private $gpioHomegearChannel = NULL; //Channel to pass Homegear events to gpio thread

    function __construct()
    {
            $this->hg = new \Homegear\Homegear();
    }

    function __destruct()
    {
            $this->stop();
    }

    public function init(array $nodeInfo) : bool
    {
            $this->nodeInfo = $nodeInfo;
            return true;
    }

    public function start() : bool
    {
            $scriptId = $this->hg->getScriptId();
            $gpioIndex = (int)$this->nodeInfo['info']['index'];
            $nodeId = $this->nodeInfo['id'];
            $trueOnly = (bool)$this->nodeInfo['info']['trueonly'];
            $debounce = (bool)$this->nodeInfo['info']['debounce'];
            $this->gpioRuntime = new Runtime();
            $this->gpioHomegearChannel = Channel::make('gpioHomegearChannelNode'.$nodeId, Channel::Infinite);
            global $gpioThread;
            $this->gpioFuture = $this->gpioRuntime->run($gpioThread, [$scriptId, $nodeId, $gpioIndex, $trueOnly, $debounce, $this->gpioHomegearChannel]);
            return true;
    }

    public function stop()
    {
            if($this->gpioHomegearChannel) $this->gpioHomegearChannel->send(['name' => 'stop', 'value' => true]);
    }

    public function waitForStop()
    {
            if($this->gpioFuture)
            {
                    $this->gpioFuture->value();
                    $this->gpioFuture = NULL;
            }
            if($this->gpioHomegearChannel)
            {
                    $this->gpioHomegearChannel->close();
                    $this->gpioHomegearChannel = NULL;
            }
            if($this->gpioRuntime)
            {
                    $this->gpioRuntime->close();
                    $this->gpioRuntime = NULL;
            }
    }
}