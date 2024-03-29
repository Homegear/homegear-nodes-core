from homegear import Homegear
import unittest
import sys
import time
import os
import datetime
import shutil
import stat


class CreateNotRecursive(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "false",
                "allEvents": "true",
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

    def test_createDirectory(self):
        os.mkdir((path + "/foo"))
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_CREATE":
                self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                checked = True
        self.assertTrue(True, checked)

    def test_createDirectories(self):
        for i in range(10):
            os.mkdir((path + "/foo" + str(i)))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_CREATE":
                    self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                    checked = True
            self.assertTrue(True, checked)

    def test_createFile(self):
        file = open((path + "/foo.txt"), "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        for input in inputHistory:
            if input[1]['payload'] == "IN_CREATE":
                self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                checked = True
        self.assertTrue(True, checked)

    def test_createFiles(self):
        for i in range(10):
            file = open((path + "/foo" + str(i) + ".txt"), "x")
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            for input in inputHistory:
                if input[1]['payload'] == "IN_CREATE":
                    self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                    checked = True
            self.assertTrue(True, checked)


class CreateRecursive(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "true",
                "allEvents": "true",
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

    def test_createDirectory(self):
        os.mkdir((path + "/foo"))
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_CREATE":
                self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                checked = True
        self.assertTrue(True, checked)

    def test_createDirectories(self):
        for i in range(10):
            os.mkdir((path + "/foo" + str(i)))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_CREATE":
                    self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                    checked = True
            self.assertTrue(True, checked)

    def test_createDirectoryRecursive(self):
        p = path + "/foo"
        for i in range(5):
            os.mkdir(p)
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_CREATE":
                    self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                    checked = True
            self.assertTrue(True, checked)
            p = p + "/foo"

    def test_createFile(self):
        file = open((path + "/foo.txt"), "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_CREATE":
                self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                checked = True
        self.assertTrue(True, checked)

    def test_createFiles(self):
        for i in range(10):
            file = open((path + "/foo" + str(i) + ".txt"), "x")
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_CREATE":
                    self.assertEqual(input[1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
                    checked = True
            self.assertTrue(True, checked)


class WatchFile(unittest.TestCase):
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
        file = open(path + "/foo.txt", "x")
        file.close()
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path + "/foo.txt",
                "recursive": "true",
                "allEvents": "true",
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

    def test_openWatchedFile(self):
        file = open(path + "/foo.txt", "r")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_OPEN":
                self.assertEqual(input[1]['payload'], "IN_OPEN", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_OPEN'")
                checked = True
        self.assertTrue(True, checked)

    def test_writeWatchedFile(self):
        file = open(path + "/foo.txt", "a")
        file.write("This is a test")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MODIFY":
                self.assertEqual(input[1]['payload'], "IN_MODIFY", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MODIFY'")
                checked = True
        self.assertTrue(True, checked)


class DeleteNotRecursive(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "false",
                "allEvents": "true",
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

    def test_deleteDirectory(self):
        p = path + "/foo"
        os.mkdir(p)
        time.sleep(1)
        os.rmdir(p)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_DELETE":
                self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                checked = True
        self.assertTrue(True, checked)

    def test_deleteDirectories(self):
        p = path + "/foo"
        for i in range(10):
            os.mkdir(p + str(i))
        time.sleep(1)
        for i in range(10):
            os.rmdir(p + str(i))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_DELETE":
                    self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                    checked = True
            self.assertTrue(True, checked)

    def test_deleteFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        os.remove(path + "/foo.txt")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_DELETE":
                self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                checked = True
        self.assertTrue(True, checked)

    def test_deleteFiles(self):
        for i in range(10):
            file = open(path + "/foo" + str(i) + ".txt", "x")
            file.close()
        time.sleep(1)
        for i in range(10):
            os.remove(path + "/foo" + str(i) + ".txt")
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_DELETE":
                    self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                    checked = True
            self.assertTrue(True, checked)


class DeleteRecursive(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "true",
                "allEvents": "true",
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

    def test_deleteDirectory(self):
        p = path + "/foo"
        os.mkdir(p)
        time.sleep(1)
        os.rmdir(p)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_DELETE":
                self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                checked = True
        self.assertTrue(True, checked)

    def test_deleteDirectories(self):
        p = path + "/foo"
        for i in range(10):
            os.mkdir(p + str(i))
        time.sleep(1)
        for i in range(10):
            os.rmdir(p + str(i))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_DELETE":
                    self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                    checked = True
            self.assertTrue(True, checked)

    def test_deleteDirectoriesRecursive(self):
        p = path + "/foo"
        for i in range(5):
            os.mkdir(p)
            p = p + "/foo"
            time.sleep(1)
        for i in range(5):
            p = p[:-4]
            os.rmdir(p)
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_DELETE":
                    self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                    checked = True
            self.assertTrue(True, checked)

    def test_deleteFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        os.remove(path + "/foo.txt")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_DELETE":
                self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                checked = True
        self.assertTrue(True, checked)

    def test_deleteFiles(self):
        for i in range(10):
            file = open(path + "/foo" + str(i) + ".txt", "x")
            file.close()
        time.sleep(1)

        for i in range(10):
            os.remove(path + "/foo" + str(i) + ".txt")
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_DELETE":
                    self.assertEqual(input[1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")
                    checked = True
            self.assertTrue(True, checked)


class OpenFile(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "true",
                "allEvents": "true",
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

    def test_openFile(self):
        p = path + "/foo.txt"
        file = open(p, "x")
        file.close()
        time.sleep(1)

        file = open(p, "a")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_OPEN":
                self.assertEqual(input[1]['payload'], "IN_OPEN", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_OPEN'")
                checked = True
        self.assertTrue(True, checked)
        file.close()

    def test_openFiles(self):
        for i in range(10):
            file = open(path + "/foo" + str(i) + ".txt", "x")
            file.close()
        time.sleep(1)

        file = []
        for i in range(10):
            file.append(open(path + "/foo" + str(i) + ".txt", "a"))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_OPEN":
                    self.assertEqual(input[1]['payload'], "IN_OPEN", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_OPEN'")
                    checked = True
            self.assertTrue(True, checked)

        for f in file:
            f.close()


class WriteFile(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "true",
                "allEvents": "true",
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

    def test_writeFile(self):
        file = open(path + "/foo.txt", "x")
        file.write("This is a test")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_CLOSE_WRITE":
                self.assertEqual(input[1]['payload'], "IN_CLOSE_WRITE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CLOSE_WRITE'")
                checked = True
        self.assertTrue(True, checked)
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MODIFY":
                self.assertEqual(input[1]['payload'], "IN_MODIFY", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MODIFY'")
                checked = True
        self.assertTrue(True, checked)

    def test_noWriteFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_CLOSE_WRITE":
                self.assertEqual(input[1]['payload'], "IN_CLOSE_WRITE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CLOSE_WRITE'")
                checked = True
        self.assertTrue(True, checked)


class Modify(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "true",
                "allEvents": "true",
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

    def test_renameDirectory(self):
        os.mkdir(path + "/foo")
        time.sleep(1)
        os.rename(path + "/foo", path + "/bar")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVED_TO":
                self.assertEqual(input[1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
                checked = True
        self.assertTrue(True, checked)
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVE_FROM":
                self.assertEqual(input[1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")
                checked = True
        self.assertTrue(True, checked)

    def test_renameDirectories(self):
        directory = []
        for i in range(10):
            directory.append(path + "/foo" + str(i))
            os.mkdir(directory[i])
        time.sleep(1)
        for i in range(10):
            os.rename(directory[i], path + "/bar" + str(i))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_MOVED_TO":
                    self.assertEqual(input[1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
                    checked = True
            self.assertTrue(True, checked)
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_MOVE_FROM":
                    self.assertEqual(input[1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")
                    checked = True
            self.assertTrue(True, checked)

    def test_renameFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        os.rename(path + "/foo.txt", path + "/bar.txt")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVED_TO":
                self.assertEqual(input[1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
                checked = True
        self.assertTrue(True, checked)
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVE_FROM":
                self.assertEqual(input[1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")
                checked = True
        self.assertTrue(True, checked)

    def test_renameFiles(self):
        files = []
        for i in range(10):
            files.append(path + "/foo" + str(i) + ".txt")
            file = open(files[i], "x")
            file.close()
        time.sleep(1)
        for i in range(10):
            os.rename(files[i], path + "/bar" + str(i) + ".txt")
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_MOVED_TO":
                    self.assertEqual(input[1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
                    checked = True
            self.assertTrue(True, checked)
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_MOVE_FROM":
                    self.assertEqual(input[1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")
                    checked = True
            self.assertTrue(True, checked)

    def test_moveDirectory(self):
        os.mkdir(path + "/foo")
        os.mkdir(path + "/foo/foo")
        os.mkdir(path + "/bar")
        time.sleep(1)
        shutil.move(path + "/foo/foo", path + "/bar")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVED_TO":
                self.assertEqual(input[1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
                checked = True
        self.assertTrue(True, checked)
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVE_FROM":
                self.assertEqual(input[1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")
                checked = True
        self.assertTrue(True, checked)

    def test_moveFile(self):
        os.mkdir(path + "/foo")
        os.mkdir(path + "/bar")
        file = open(path + "/foo/foo.txt", "x")
        file.close()
        time.sleep(1)
        shutil.move(path + "/foo/foo.txt", path + "/bar")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVED_TO":
                self.assertEqual(input[1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
                checked = True
        self.assertTrue(True, checked)
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MOVE_FROM":
                self.assertEqual(input[1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")
                checked = True
        self.assertTrue(True, checked)

    def test_modifyMetaDataTime(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        date = datetime.datetime(year=2017, month=4, day=2, hour=19, minute=5, second=42)
        modTime = time.mktime(date.timetuple())
        os.utime(path + "/foo.txt", (modTime, modTime))
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_ATTRIB":
                self.assertEqual(input[1]['payload'], "IN_ATTRIB", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_ATTRIB'")
                checked = True
        self.assertTrue(True, checked)

    def test_modifyMetaDataRights(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        os.chmod(path + "/foo.txt", stat.S_IREAD)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_ATTRIB":
                self.assertEqual(input[1]['payload'], "IN_ATTRIB", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_ATTRIB'")
                checked = True
        self.assertTrue(True, checked)


class DeleteWatchedDirectory(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "true",
                "allEvents": "true",
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

    def test_deleteWatchedDirectory(self):
        os.rmdir(path)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_DELETE_SELF":
                self.assertEqual(input[1]['payload'], "IN_DELETE_SELF", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE_SELF'")
                checked = True
        self.assertTrue(True, checked)
        os.mkdir(path)

    def test_deleteWatchedDirectoryRecursive(self):
        p = path + "/foo"
        for i in range(5):
            os.mkdir(p)
            p = p + "/foo"
            time.sleep(1)
        for i in range(5):
            p = p[:-4]
            os.rmdir(p)
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            checked = False
            for input in inputHistory:
                if input[1]['payload'] == "IN_DELETE_SELF":
                    self.assertEqual(input[1]['payload'], "IN_DELETE_SELF", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE_SELF'")
                    checked = True
            self.assertTrue(True, checked)

        os.rmdir(path)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_DELETE_SELF":
                self.assertEqual(input[1]['payload'], "IN_DELETE_SELF", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE_SELF'")
                checked = True
        self.assertTrue(True, checked)
        os.mkdir(path)


class Selector(unittest.TestCase):
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
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "false",
                "allEvents": "false",
                "modifyEvent": "true",
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

    def test_createFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"Input should be None, but was {inputHistory}")

    def test_writeFile(self):
        file = open(path + "/foo.txt", "x")
        file.write("This is a test")
        file.close()
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        checked = False
        for input in inputHistory:
            if input[1]['payload'] == "IN_MODIFY":
                self.assertEqual(input[1]['payload'], "IN_MODIFY", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MODIFY'")
                checked = True
        self.assertTrue(True, checked)


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
