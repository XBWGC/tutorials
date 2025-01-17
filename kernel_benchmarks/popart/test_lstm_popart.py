# Copyright (c) 2020 Graphcore Ltd. All rights reserved.
import os
import sys

import pytest
import tutorials_tests.testing_util as testing_util


def run_lstm(batch_size, timesteps, hidden_size, extra_args=None):
    py_version = "python" + str(sys.version_info[0])
    cmd = [
        py_version,
        "lstm.py",
        "--batch-size",
        str(batch_size),
        "--timesteps",
        str(timesteps),
        "--hidden-size",
        str(hidden_size),
    ]

    # Accommodate framework-specific args
    if extra_args:
        cmd.extend(extra_args)

    out = testing_util.run_command_fail_explicitly(cmd, os.path.dirname(__file__))

    return out


"""Tests for the popART LSTM synthetic benchmarks"""


# LSTM inference
@pytest.mark.ipus(1)
@pytest.mark.category1
def test_lstm_inference_b256_s25_h1024():
    out = run_lstm(batch_size=256, timesteps=25, hidden_size=1024)


@pytest.mark.ipus(1)
@pytest.mark.category1
def test_lstm_inference_b128_s50_h1536():
    out = run_lstm(
        batch_size=128,
        timesteps=50,
        hidden_size=1536,
        extra_args=['--lstm-options={"availableMemoryProportion":"0.55"}'],
    )


@pytest.mark.ipus(1)
@pytest.mark.category1
def test_lstm_inference_b64_s25_h2048():
    out = run_lstm(batch_size=64, timesteps=25, hidden_size=2048)


@pytest.mark.ipus(1)
@pytest.mark.category1
def test_lstm_inference_b1024_s150_h256():
    out = run_lstm(batch_size=1024, timesteps=150, hidden_size=256)


@pytest.mark.ipus(1)
@pytest.mark.category1
def test_lstm_inference_b1024_s25_h512():
    out = run_lstm(batch_size=1024, timesteps=25, hidden_size=512)
