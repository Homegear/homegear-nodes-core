<?php
function input($nodeInfo, $message)
{
	\Homegear\Homegear::log(2, print_v($nodeInfo, true), print_v($message, true));
}