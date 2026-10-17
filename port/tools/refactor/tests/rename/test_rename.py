"""Tests of rename.py on a small tree (port/tools/refactor/tests/rename/tree). Run with pytest."""
import json
import os
import shutil
import subprocess
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.path.join(HERE, "..", "..", "rename.py")
FILES = ["members.cpp", "macros.cpp", "templates.cpp", "inactive.cpp"]


@pytest.fixture()
def tree(tmp_path):
    root = tmp_path / "root"
    shutil.copytree(os.path.join(HERE, "tree"), root)
    game = root / "port" / "game"
    build = root / "build"
    build.mkdir()
    db = [
        {
            "directory": str(build),
            "file": str(game / f),
            "command": f"clang++-18 -std=c++17 -c {game / f} -I{game}",
        }
        for f in FILES
    ]
    (build / "compile_commands.json").write_text(json.dumps(db))
    return root


def run(tree, rows, *flags, only=None):
    csv_path = tree / "renames.csv"
    csv_path.write_text("old,new\n" + "".join(f"{o},{n}\n" for o, n in rows))
    cmd = [sys.executable, TOOL, str(csv_path), "--build-dir", str(tree / "build"), "-j", "2", *flags]
    for o in only or []:
        cmd += ["--only", o]
    env = dict(os.environ, RENAME_ROOT=str(tree))
    return subprocess.run(cmd, capture_output=True, text=True, env=env)


def read(tree, name):
    return (tree / "port" / "game" / name).read_text()


def test_member_in_two_classes(tree):
    r = run(tree, [("A::field_0_x", "x")], "--residual", "--templates")
    assert r.returncode == 0, r.stdout + r.stderr
    hpp = read(tree, "members.hpp")
    assert "int x;" in hpp and "return x;" in hpp
    assert hpp.count("field_0_x") == 1  # B's
    assert "a.x + b.field_0_x" in read(tree, "members.cpp")


def test_overload_set(tree):
    r = run(tree, [("Draw_401000", "Draw")], only=["members.cpp"])
    assert r.returncode == 0, r.stdout + r.stderr
    assert "Draw_401000" not in read(tree, "members.hpp") + read(tree, "members.cpp")
    assert read(tree, "members.cpp").count("Draw(") == 4


def test_method_through_base(tree):
    r = run(tree, [("Base::Update_402000", "Update")], only=["members.cpp"])
    assert r.returncode == 0, r.stdout + r.stderr
    cpp = read(tree, "members.cpp")
    assert "d.Update()" in cpp and "p->Update()" in cpp


def test_type_with_constructor_destructor_friend_template(tree):
    r = run(tree, [("Widget_10", "Widget")], only=["members.cpp"])
    assert r.returncode == 0, r.stdout + r.stderr
    hpp, cpp = read(tree, "members.hpp"), read(tree, "members.cpp")
    assert "Widget_10" not in hpp + cpp
    assert "Widget::Widget()" in cpp and "Widget::~Widget()" in cpp
    assert "Holder<Widget> gHolder" in cpp
    assert "friend struct Widget;" in hpp
    assert "Widget(const Widget& other) : Widget()" in hpp


def test_name_in_macro_body_is_edited_where_spelled(tree):
    r = run(tree, [("A::field_0_x", "x")], "--residual", "--templates")
    assert r.returncode == 0, r.stdout + r.stderr
    assert "#define GET_X(o) ((o).x)" in read(tree, "macros.cpp")


def test_name_made_by_token_pasting_refuses(tree):
    r = run(tree, [("Draw_401000", "Draw")], only=["macros.cpp"])
    assert r.returncode == 2, r.stdout + r.stderr
    assert "REFUSED" in r.stdout
    assert "Draw_401000" in read(tree, "members.hpp")  # nothing was written


def test_collision_refuses(tree):
    r = run(tree, [("A::field_0_x", "y")], only=["members.cpp"])
    assert r.returncode == 2 and "already exists" in r.stdout, r.stdout + r.stderr
    assert "field_0_x" in read(tree, "members.hpp")


def test_two_old_names_one_new_name_refuses(tree):
    r = run(tree, [("A::field_0_x", "z"), ("A::y", "z")], only=["members.cpp"])
    assert r.returncode == 2 and "would both be called" in r.stdout, r.stdout + r.stderr


def test_virtual_function_refuses(tree):
    r = run(tree, [("V::Tick_403000", "Tick")], only=["members.cpp"])
    assert r.returncode == 2 and "virtual" in r.stdout, r.stdout + r.stderr


def test_unknown_symbol_refuses(tree):
    r = run(tree, [("Nope", "Yes")], only=["members.cpp"])
    assert r.returncode == 2 and "no such symbol" in r.stdout


def test_dependent_name_in_template(tree):
    rows = [("Node::mpNext", "next")]
    r = run(tree, rows, only=["templates.cpp"])
    assert r.returncode == 3, r.stdout + r.stderr  # the residual in the template is listed, nothing written
    assert "RESIDUAL (template)" in r.stdout and "mpNext" in read(tree, "templates.cpp")
    r = run(tree, rows, "--templates", only=["templates.cpp"])
    assert r.returncode == 0, r.stdout + r.stderr
    text = read(tree, "templates.cpp")
    assert "mpNext" not in text and "n->next = head" in text and "Node* next;" in text


def test_inactive_preprocessor_branch(tree):
    rows = [("A::field_0_x", "x")]
    r = run(tree, rows, "--templates", only=["inactive.cpp"])
    assert r.returncode == 3, r.stdout + r.stderr
    assert "RESIDUAL" in r.stdout and "inactive.cpp:7" in r.stdout
    r = run(tree, rows, "--templates", "--residual", only=["inactive.cpp"])
    assert r.returncode == 0, r.stdout + r.stderr
    text = read(tree, "inactive.cpp")
    assert "field_0_x" not in text and text.count("a.x") == 2


def test_dry_run_writes_nothing_and_lists_files(tree):
    r = run(tree, [("Widget_10", "Widget")], "--dry-run", only=["members.cpp"])
    assert r.returncode == 0
    assert "would edit port/game/members.cpp" in r.stdout
    assert "Widget_10" in read(tree, "members.hpp")
