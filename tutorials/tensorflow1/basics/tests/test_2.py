# Copyright (c) 2021 Graphcore Ltd. All rights reserved.

from pathlib import Path
import pytest

import tutorials_tests.testing_util as testing_util

working_path = Path(__file__).parents[1] / "tut2_loops_data_pipeline"


@pytest.mark.category2
@pytest.mark.ipus(1)
def test_run_complete_ipu():
    testing_util.run_command(
        "python example_2.py", working_path, "Program ran successfully"
    )
