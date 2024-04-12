"""
This package supports automated fingerprint testing.

The main function is :py:func:`run_fingerprint_tests <inet.test.fingerprint.task.run_fingerprint_tests>`. It allows running multiple fingerprint tests matching
the provided filter criteria.
"""

from inet.test.fingerprint.old import *
from inet.test.fingerprint.store import *
from inet.test.fingerprint.task import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
