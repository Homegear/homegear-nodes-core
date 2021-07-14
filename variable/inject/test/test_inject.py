import datetime

from homegear import Homegear
import unittest
import sys
import time


class NoneTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": "",
                "crontab": "",
                "once": "false",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_none(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")
        time.sleep(2)
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")


class NoneOnceTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": "",
                "crontab": "",
                "once": "true",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_none(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(len(inputHistory), 1, f"Length should be 1, but is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[0][1][testValues[0][0]]}")
        self.assertEqual(inputHistory[0][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[0][1][testValues[1][0]]}")
        self.assertEqual(inputHistory[0][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[0][1][testValues[2][0]]}")
        time.sleep(2)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"Length should be 1, but is {len(inputHistory)}")


class IntervalTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        global intervalTime
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        intervalTime = 5
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": intervalTime,
                "crontab": "",
                "once": "false",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_interval(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")
        for i in range(2):
            time.sleep(intervalTime)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(len(inputHistory), 1 + i, f"Length should be 1, but is {inputHistory}")
            self.assertEqual(inputHistory[i][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[i][1][testValues[0][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[i][1][testValues[1][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[i][1][testValues[2][0]]}")


class IntervalOnceTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        global intervalTime
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        intervalTime = 5
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": intervalTime,
                "crontab": "",
                "once": "true",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_interval(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on")
        for i in range(2):
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertEqual(len(inputHistory), 1 + i, f"Length should be 1, but is {inputHistory}")
            self.assertEqual(inputHistory[i][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[i][1][testValues[0][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[i][1][testValues[1][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[i][1][testValues[2][0]]}")
            if i < 2:
                time.sleep(intervalTime)


class IntervalTimeTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        global intervalTime
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        intervalTime = 60
        now = time.localtime()
        hour = time.strftime("%H", now)
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": "",
                "crontab": "*/1 " + hour + " * * *",
                "once": "false",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_interval_time(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")
        for i in range(2):
            time.sleep(intervalTime)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertEqual(len(inputHistory), 1 + i, f"Length should be 1, but is {inputHistory}")
            self.assertEqual(inputHistory[i][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[i][1][testValues[0][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[i][1][testValues[1][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[i][1][testValues[2][0]]}")


class IntervalTimeOnceTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        global intervalTime
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        intervalTime = 60
        now = time.localtime()
        hour = time.strftime("%H", now)
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": "",
                "crontab": "*/1 " + hour + " * * *",
                "once": "true",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_interval_time(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on")
        for i in range(2):
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertEqual(len(inputHistory), 1 + i, f"Length should be 1, but is {inputHistory}")
            self.assertEqual(inputHistory[i][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[i][1][testValues[0][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[i][1][testValues[1][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[i][1][testValues[2][0]]}")
            if i < 2:
                time.sleep(intervalTime)


class TimeTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        global intervalTime
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        now = time.localtime()
        hour = time.strftime("%H", now)
        minute = time.strftime("%M", now)
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": "",
                "crontab": minute + " " + hour + " * * *",
                "once": "true",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_time(self):
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(len(inputHistory), 1, f"Length should be 1, but is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[0][1][testValues[0][0]]}")
        self.assertEqual(inputHistory[0][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[0][1][testValues[1][0]]}")
        self.assertEqual(inputHistory[0][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[0][1][testValues[2][0]]}")


class TimeOnceTest(unittest.TestCase):
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

    @classmethod
    def setUp(self):
        global testValues
        global intervalTime
        testValues = [['payload', 'This is a test'], ['topic', 'true'], ['test', 42]]
        now = datetime.datetime.now()
        now = now + datetime.timedelta(minutes=1)
        hour = now.strftime("%H")
        minute = now.strftime("%M")
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
                "repeat": "",
                "crontab": minute + " " + hour + " * * *",
                "once": "true",
                "onceDelay": "1",
                "props": [{"p": "payload"},
                          {"p": "topic", "vt": "bool"},
                          {"p": testValues[2][0], "v": testValues[2][1], "vt": "num"}],
                "payload": testValues[0][1],
                "payloadType": "str",
                "topic": testValues[1][1],
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
        nodeIds = hg.addNodesToFlow("Inject Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Inject Unit test", "unit-test")

    def test_time(self):
        for i in range(1):
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(len(inputHistory), 1 + i, f"Length should be 1, but is {inputHistory}")
            self.assertEqual(inputHistory[i][1][testValues[0][0]], testValues[0][1], f"Input should be '{testValues[0][1]}' at '{testValues[0][0]}', but is {inputHistory[i][1][testValues[0][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[1][0]], testValues[1][1], f"Input should be '{testValues[1][1]}' at '{testValues[1][0]}', but is {inputHistory[i][1][testValues[1][0]]}")
            self.assertEqual(inputHistory[i][1][testValues[2][0]], testValues[2][1], f"Input should be '{testValues[2][1]}' at '{testValues[2][0]}', but is {inputHistory[i][1][testValues[2][0]]}")
            if i < 1:
                time.sleep(60)


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
