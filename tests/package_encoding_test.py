"""Run package validation with the legacy Windows default encoding simulated."""
from pathlib import Path
import runpy
from unittest.mock import patch

original_read_text = Path.read_text

def windows_read_text(self, encoding=None, errors=None, **kwargs):
    return original_read_text(self, encoding=encoding or "cp1252", errors=errors, **kwargs)

if __name__ == "__main__":
    validator = Path(__file__).resolve().parents[1] / "tools" / "verify-package.py"
    with patch.object(Path, "read_text", windows_read_text):
        runpy.run_path(str(validator), run_name="__main__")
    print("Package validation passed with simulated Windows cp1252 defaults")
