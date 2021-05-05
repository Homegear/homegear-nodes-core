from homegear import Homegear
import unittest
import sys
import time
import os
from logging.handlers import TimedRotatingFileHandler
import logging
import datetime


class WriteToEmptyFile(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg, path
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

        path = os.getcwd() + "/testingDirectory"
        if not os.path.exists(path):
            os.mkdir(path)

    @classmethod
    def tearDownClass(cls):
        os.rmdir(path)

    def setUp(self):
        global filePath
        filePath = path + "/foo.txt"
        file = open(filePath, "x")
        file.close()
        testFlow = [
            {
                "id": "n1",
                "type": "tail",
                "path": filePath,
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

        for root, dirs, files in os.walk(path, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def test_write(self):
        text = "This is a test"
        file = open(filePath, "a")
        file.write(text + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")

    def test_writeMultiple(self):
        text = "This is a test"
        i = 0
        while text:
            if i < 10:
                i = i + 1
            file = open(filePath, "a")
            file.write(text + '\n')
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(len(inputHistory), i, f"There should be {i} messages, but there are {len(inputHistory)} messages")
            self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")
            text = text[:-2]

    def test_writeEmptyLine(self):
        file = open(filePath, "a")
        file.write('' + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")

    def test_writeOneLetter(self):
        text = 'a'
        file = open(filePath, "a")
        file.write(text + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")

    def test_writeMultipleLines(self):
        text = "This is a test\nThe answer to life the universe and everything\n42\nWrite something here"
        lastLine = "This is the last line"
        file = open(filePath, "a")
        file.write(text + '\n')
        file.write(lastLine + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], lastLine, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{lastLine}'")


class AppendToFile(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg, path
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

        path = os.getcwd() + "/testingDirectory"
        if not os.path.exists(path):
            os.mkdir(path)

    @classmethod
    def tearDownClass(cls):
        os.rmdir(path)

    def setUp(self):
        global filePath
        filePath = path + "/foo.txt"
        file = open(filePath, "x")
        file.write("This is the first line\nThis is the second line\n")
        file.close()
        testFlow = [
            {
                "id": "n1",
                "type": "tail",
                "path": filePath,
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

        for root, dirs, files in os.walk(path, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def test_append(self):
        text = "This is a test"
        file = open(filePath, "a")
        file.write(text + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")

    def test_appendMultiple(self):
        text = "This is a test"
        i = 0
        while text:
            if i < 10:
                i = i + 1
            file = open(filePath, "a")
            file.write(text + '\n')
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(len(inputHistory), i, f"There should be {i} messages, but there are {len(inputHistory)} messages")
            self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")
            text = text[:-2]

    def test_appendEmptyLine(self):
        file = open(filePath, "a")
        file.write('' + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")

    def test_appendOneLetter(self):
        text = 'a'
        file = open(filePath, "a")
        file.write(text + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")

    def test_appendMultipleLines(self):
        text = "This is a test\nThe answer to life the universe and everything\n42\nWrite something here"
        lastLine = "This is the last line"
        file = open(filePath, "a")
        file.write(text + '\n')
        file.write(lastLine + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], lastLine, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{lastLine}'")


class RotateLog(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg, path
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

        path = os.getcwd() + "/testingDirectory"
        if not os.path.exists(path):
            os.mkdir(path)

    @classmethod
    def tearDownClass(cls):
        os.rmdir(path)

    def setUp(self):
        global filePath
        filePath = path + "/foo.log"
        file = open(filePath, "x")
        file.close()
        testFlow = [
            {
                "id": "n1",
                "type": "tail",
                "path": filePath,
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

        self.createTimedRotatingLog()
        time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

        for root, dirs, files in os.walk(path, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def createTimedRotatingLog(self):
        global logger
        logger = logging.getLogger("Rotating Log")
        logger.setLevel(logging.INFO)
        handler = TimedRotatingFileHandler(filePath, when="s", interval=5, backupCount=5)
        logger.addHandler(handler)

    def test_rotatingLog(self):
        text = "The answer to life the universe and everything"
        logger.info(text)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")
        time.sleep(5)
        text = "42"
        logger.info(text)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")


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
