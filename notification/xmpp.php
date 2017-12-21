<?php
declare(strict_types=1);
require_once('vendor-libs/autoload.php');

use Fabiang\Xmpp\Options;
use Fabiang\Xmpp\Client;
use Fabiang\Xmpp\Protocol\Message;

class HomegearNode extends HomegearNodeBase
{
    private $hg = NULL;
    private $nodeInfo = NULL;

    public function __construct()
    {
	    $this->hg = new \Homegear\Homegear();
    }

    public function input(array $nodeInfoLocal, int $inputIndex, array $message)
    {
        $recipient = (isset($message['xmpp-recipient']) ? $message['xmpp-recipient'] : $nodeInfoLocal['info']['recipient']);
        $msg = (isset($message['xmpp-message']) ? $message['xmpp-message'] : $nodeInfoLocal['info']['message']);
        $config = $this->getConfigParameter($nodeInfoLocal['info']['server'],'port');
        $this->_sendMessage($config,$recipient,$msg);
    }

    private function _sendMessage($config = false, $recipient = false, $msg = false)
    {
        if ($config == false) { $this->log(4,"xmpp-server: config parameters missing!"); return; }
        if ($recipient == false) { $this->log(4,"xmpp-server: recipient missing!"); return; }
        if ($msg == false) { $this->log(4,"xmpp-server: message missing!"); return; }
        $options = new Options('tcp://'.$config['server'].':'.$config['port']);
        $options->setUsername($config['username']);
        $options->setPassword($config['password']);
        $client = new Client($options);
        $message = new Message;
        $message->setMessage($msg)->setTo($recipient);
        $client->connect();
        $client->send($message);
        $client->disconnect();
    }
}
?>