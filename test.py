#!/usr/bin/env python

# pyright: strict

import csv
import shutil
import os
import subprocess
import tempfile
from pathlib import Path
from typing import override
import unittest


class TestEverything(unittest.TestCase):
    def __init__(self, methodName: str = "runTest") -> None:
        super().__init__(methodName)
        self.init_dir: Path = Path(".")

    @override
    def setUp(self) -> None:
        super().setUp()

        self.init_dir = Path(".").absolute()
        binary = Path("./main.out").absolute()
        tempdir = Path(self.enterContext(tempfile.TemporaryDirectory(delete=True)))
        os.chdir(tempdir)
        _ = shutil.copy2(binary, "./main.out")

    @override
    def tearDown(self) -> None:
        super().tearDown()
        os.chdir(self.init_dir)

    def test_missing_csv(self):
        with self.assertRaises(
            subprocess.CalledProcessError, msg="did not error on missing csv"
        ):
            output = run()
            self.assertGreater(len(output), 0, "empty output")

    def test_create_dat(self):
        with open("Employee.csv", "w+") as csv_file:
            _ = csv_file.write("1,Name,Bio,1\n")

        _ = run()
        data_file = Path("EmployeeIndex.dat")
        self.assertTrue(data_file.exists(), "data file not created")
        self.assertGreater(
            data_file.stat().st_size, 0, "data file empty with nonempty csv"
        )

    def test_find_all(self):
        with open("Employee.csv", "w+") as csv_file:
            _ = csv_file.write("1,Name,Bio,1\n")
            _ = csv_file.write("2,Another Name,Bio,1\n")
            _ = csv_file.write("3,Name,Bio,1\n")
            _ = csv_file.write("4,Name,Bio,6\n")
            _ = csv_file.write("5,Name,Some Bio,5\n")
            _ = csv_file.write("6,Name,Bio,5\n")

        ids = [1, 2, 3, 4, 5, 6]
        output = run(*ids)
        self.assertGreater(len(output), 0, "empty output")
        for id, line in zip(ids, output.splitlines()):
            self.assertTrue(
                line.startswith(f"ID: {id}"),
                f"missing or malformed find output for existing employee {id=}: {line}",
            )

    def test_missing(self):
        with open("Employee.csv", "w+") as csv_file:
            _ = csv_file.write("50,My name, my bio, 0\n")

        with self.assertRaises(
            subprocess.CalledProcessError,
            msg="did not return nonzero on missing employee",
        ):
            output = run(1)
            self.assertGreater(len(output), 0, "empty output")

    def test_provided_csv(self):
        for input_csv in ("Employee.csv", "Employee_large.csv"):
            with self.subTest(input_csv=input_csv):
                _ = shutil.copy2(self.init_dir / input_csv, "./Employee.csv")

                with open("Employee.csv", "r") as csv_file:
                    reader = csv.reader(csv_file)
                    ids = map(lambda row: row[0], reader)
                    ids = list(map(int, ids))

                try:
                    output = run(*ids)
                    self.assertGreater(len(output), 0, "empty output")
                    for id, line in zip(ids, output.splitlines()):
                        self.assertTrue(
                            line.startswith(f"ID: {id}"),
                            f"missing or malformed find output for existing employee {id=}: {line}",
                        )
                except subprocess.CalledProcessError as e:
                    print(f"error: {e.stdout}\n{e.stderr}")  # pyright: ignore [reportAny]
                    raise e


def run(*ids: int) -> str:
    arguments = " ".join(map(str, ids))
    result = subprocess.run(
        ["./main.out"], input=arguments, capture_output=True, check=True, text=True
    )
    return result.stdout + result.stderr
