from homegear import Homegear
import unittest
import sys
import time


class BlockIfValueChangeGreaterEqualLastOutputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreaterEqual",
                "inputValue": "10",
                "inputValueType": "flatValue",
                "compareTo": "lastOutput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterEqualLastOutputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreaterEqual",
                "inputValue": "10",
                "inputValueType": "percent",
                "compareTo": "lastOutput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterEqualLastInputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreaterEqual",
                "inputValue": "10",
                "inputValueType": "flatValue",
                "compareTo": "lastInput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterEqualLastInputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreaterEqual",
                "inputValue": "10",
                "inputValueType": "percent",
                "compareTo": "lastInput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterLastOutputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreater",
                "inputValue": "10",
                "inputValueType": "flatValue",
                "compareTo": "lastOutput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterLastOutputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreater",
                "inputValue": "10",
                "inputValueType": "percent",
                "compareTo": "lastOutput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterLastInputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreater",
                "inputValue": "10",
                "inputValueType": "flatValue",
                "compareTo": "lastInput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


class BlockIfValueChangeGreaterLastInputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockIfValueChangeGreater",
                "inputValue": "10",
                "inputValueType": "percent",
                "compareTo": "lastInput",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_(self):
        pass


if __name__ == '__main__':
    global socketPath
    socketPath = ''
    if len(sys.argv) > 1:
        for arg in sys.argv:
            if arg.startswith("/") and not arg == sys.argv[0]:
                socketPath = arg
                sys.argv.remove(arg)
                break

    unittest.main()