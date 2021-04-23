from homegear import Homegear
import unittest
import time
import datetime
import os
import shutil
import stat


class CreateNotRecursiveTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")

    def test_createDirectories(self):
        for i in range(10):
            os.mkdir((path + "/foo" + str(i)))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")

    def test_createFile(self):
        file = open((path + "/foo.txt"), "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[2][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[2][1]['payload']}', but should be 'IN_CREATE'")

    def test_createFiles(self):
        for i in range(10):
            file = open((path + "/foo" + str(i) + ".txt"), "x")
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[2][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[2][1]['payload']}', but should be 'IN_CREATE'")


class CreateRecursiveTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")

    def test_createDirectories(self):
        for i in range(10):
            os.mkdir((path + "/foo" + str(i)))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")

    def test_createDirectoryRecursive(self):
        p = path + "/foo"
        for i in range(5):
            os.mkdir(p)
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")
            p = p + "/foo"

    def test_createFile(self):
        file = open((path + "/foo.txt"), "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[2][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")

    def test_createFiles(self):
        for i in range(10):
            file = open((path + "/foo" + str(i) + ".txt"), "x")
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[2][1]['payload'], "IN_CREATE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CREATE'")


class DeleteNotRecursiveTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

    def test_deleteDirectories(self):
        p = path + "/foo"
        for i in range(10):
            os.mkdir(p + str(i))
        time.sleep(1)
        for i in range(10):
            os.rmdir(p + str(i))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

    def test_deleteFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        os.remove(path + "/foo.txt")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

    def test_deleteFiles(self):
        for i in range(10):
            file = open(path + "/foo" + str(i) + ".txt", "x")
            file.close()
        time.sleep(1)
        for i in range(10):
            os.remove(path + "/foo" + str(i) + ".txt")
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")


class DeleteRecursiveTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[1][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

    def test_deleteDirectories(self):
        p = path + "/foo"
        for i in range(10):
            os.mkdir(p + str(i))
        time.sleep(1)
        for i in range(10):
            os.rmdir(p + str(i))
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[1][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

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
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[1][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

    def test_deleteFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        os.remove(path + "/foo.txt")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")

    def test_deleteFiles(self):
        for i in range(10):
            file = open(path + "/foo" + str(i) + ".txt", "x")
            file.close()
        time.sleep(1)

        for i in range(10):
            os.remove(path + "/foo" + str(i) + ".txt")
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE'")


class OpenFileTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_OPEN", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_OPEN'")
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
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_OPEN", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_OPEN'")

        for f in file:
            f.close()


class WriteFileTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_CLOSE_WRITE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CLOSE_WRITE'")
        self.assertEqual(inputHistory[1][1]['payload'], "IN_MODIFY", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MODIFY'")

    def test_noWriteFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_CLOSE_WRITE", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_CLOSE_WRITE'")


class ModifyTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[1][1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
        self.assertEqual(inputHistory[2][1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")

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
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[1][1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
            self.assertEqual(inputHistory[2][1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")

    def test_renameFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        os.rename(path + "/foo.txt", path + "/bar.txt")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
        self.assertEqual(inputHistory[1][1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")

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
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
            self.assertEqual(inputHistory[1][1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")

    def test_moveDirectory(self):
        os.mkdir(path + "/foo")
        os.mkdir(path + "/foo/foo")
        os.mkdir(path + "/bar")
        time.sleep(1)
        shutil.move(path + "/foo/foo", path + "/bar")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
        self.assertEqual(inputHistory[1][1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")

    def test_moveFile(self):
        os.mkdir(path + "/foo")
        os.mkdir(path + "/bar")
        file = open(path + "/foo/foo.txt", "x")
        file.close()
        time.sleep(1)
        shutil.move(path + "/foo/foo.txt", path + "/bar")
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_MOVED_TO", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_MOVED_TO'")
        self.assertEqual(inputHistory[1][1]['payload'], "IN_MOVE_FROM", f"Payload is '{inputHistory[1][1]['payload']}', but should be 'IN_MOVE_FROM'")

    def test_modifyMetaDataTime(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        date = datetime.datetime(year=2017, month=4, day=2, hour=19, minute=5, second=42)
        modTime = time.mktime(date.timetuple())
        os.utime(path + "/foo.txt", (modTime, modTime))
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_ATTRIB", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_ATTRIB'")

    def test_modifyMetaDataRights(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        os.chmod(path + "/foo.txt", stat.S_IREAD)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_ATTRIB", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_ATTRIB'")


class DeleteWatchedDirectoryTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE_SELF", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE_SELF'")
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
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE_SELF", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE_SELF'")

        os.rmdir(path)
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "IN_DELETE_SELF", f"Payload is '{inputHistory[0][1]['payload']}', but should be 'IN_DELETE_SELF'")
        os.mkdir(path)


if __name__ == '__main__':
    unittest.main()
