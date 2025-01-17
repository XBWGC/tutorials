# Copyright (c) 2020 Graphcore Ltd. All rights reserved.

import os
import pytest

# NOTE: The import below is dependent on 'pytest.ini' in the root of the
# repository from tutorials_tests.testing_util import run_python_script_helper
from tutorials_tests import testing_util


generic_cmd = ["python", "phased_execution.py"]
generic_args = {}
cwd = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


@pytest.mark.ipus(2)
@pytest.mark.category1
def test_phased_execution():
    """Test that the code runs with default arguments"""
    cmd, args = generic_cmd.copy(), generic_args.copy()
    cmd = testing_util.add_args(cmd, args)
    out = testing_util.run_command_fail_explicitly(cmd, cwd)


@pytest.mark.ipus(2)
@pytest.mark.category1
def test_sharded_execution():
    """Test that the code runs in sharded mode
    (i.e. no phased execution)"""
    cmd, args = generic_cmd.copy(), generic_args.copy()
    args["--sharded-execution"] = None
    cmd = testing_util.add_args(cmd, args)
    out = testing_util.run_command_fail_explicitly(cmd, cwd)
