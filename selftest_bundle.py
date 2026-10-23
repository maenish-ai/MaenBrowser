#!/usr/bin/env python3
"""Self-test patch logic on synthetic fixtures; NOT a full repo integration test."""
from pathlib import Path
import importlib.util
import json
import subprocess
import tempfile
import shutil
import sys

here=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('maen_bundle',here/'integrate_maenbrowser.py')
bundle=importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundle)
with tempfile.TemporaryDirectory() as tmp:
    repo=Path(tmp)/'MaenBrowser'
    for rel,changes in bundle.REPLACEMENTS.items():
        path=repo/rel
        path.parent.mkdir(parents=True,exist_ok=True)
        # Intermediate anchors introduced by earlier replacements do not occur
        # in the original source and must not be duplicated in a fixture.
        originals = [old for i,(old,_) in enumerate(changes)
                     if not any(old in previous_new for _,previous_new in changes[:i])]
        path.write_text('\n// separate exact review anchors\n'.join(originals),encoding='utf-8')
    (repo/'src/protection/domain_rules.h').write_text('// minimal fixture domain rules\n')
    before,after=bundle.expected_changes(repo)
    assert len(before)==len(bundle.REPLACEMENTS)
    assert len(after)==len(before)+len(bundle.NEW_FILES)
    assert b'BlockKnownSecurityThreat(url)' in after['src/protection/protection.cpp']
    assert b'protection::BlockUnsolicitedPopup' in after['src/app/maen_client.cpp']
    assert b'MaenAdFectionTests' in after['CMakeLists.txt']
    assert b'securityShieldEnabled' in after['assets/companion/background.js']
    bundle.write_transaction(repo,before,after)
    assert all((repo/f).read_bytes()==data for f,data in after.items())
    with (repo/'src/protection/protection.cpp').open('a', encoding='utf-8') as f:
        f.write('\nstd::string Rewrite(const std::string& url) { return url; }\n')
    with (repo/'src/app/maen_client.cpp').open('a', encoding='utf-8') as f:
        f.write('\n// Existing original callback path: callback->Continue(download_path, ask);\n')
    # Structural checks use the synthetic patched source; these are not Windows runtime checks.
    (repo/'src/app/resource_mode.cpp').write_text(
        'disable-background-mode disk-cache-size',encoding='utf-8')
    with (repo/'CMakeLists.txt').open('a',encoding='utf-8') as f:
        f.write('\nUSE_SANDBOX\n')
    for test in ('always_on_security_policy_test.py','php_compromise_security_test.py'):
        tested=subprocess.run([sys.executable,str(repo/'tests'/test)],capture_output=True,text=True)
        if tested.returncode: print(tested.stdout, tested.stderr)
        assert tested.returncode==0, f'Integrated structural test failed: {test}'
        print('PASS: integrated structural test',test)
    try:
        bundle.expected_changes(repo)
    except RuntimeError:
        pass
    else:
        raise AssertionError('Double application incorrectly permitted')
    # Pack full source and full replacement changed-files for GitHub upload.
    exported=Path(tmp)/'EXPORT'
    bundle.write_export(repo,before,after,exported)
    assert (exported/'MaenBrowser-Full-Source-Merged.zip').exists()
    assert (exported/'MaenBrowser-Changed-Files-Upload.zip').exists()
    man=json.loads((exported/'SHA256-MANIFEST.json').read_text(encoding='utf-8'))
    assert len(man['sha256'])==len(after)
    assert len(man['added'])==len(bundle.NEW_FILES)
    assert subprocess.run([sys.executable,str(here/'integrate_maenbrowser.py'),'--repo',str(repo),'--check'],capture_output=True).returncode != 0
    print('PASS: exact anchor checking, atomic write, duplicate protection, export ZIP and SHA manifest')

# Compile actual new C++ tests against an authentic algorithm-equivalent
# local header fixture; does not claim to compile the CEF/Windows application.
header='''#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>
namespace maenbrowser::protection {
inline bool ValidDomain(std::string_view s) {
  if (s.empty() || s.size() > 253 || s.front() == '.' || s.back() == '.') return false;
  size_t label = 0;
  for (unsigned char c : s) {
    if (c == '.') { if (!label || label > 63) return false; label = 0; }
    else { if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) return false; ++label; }
  }
  return label > 0 && label <= 63;
}
inline std::string CanonicalHost(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  while (!s.empty() && s.back() == '.') s.pop_back();
  return s;
}
inline bool HostMatches(std::string_view host, std::string_view domain) {
  return host == domain || (host.size() > domain.size() && host[host.size() - domain.size() - 1] == '.' && host.ends_with(domain));
}
class DomainRules {
 public:
  void Assign(std::vector<std::string> values) {
    values.erase(std::remove_if(values.begin(), values.end(), [](const auto& s) { return !ValidDomain(s); }), values.end());
    if (!std::is_sorted(values.begin(), values.end())) std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    values_ = std::move(values);
  }
  bool Contains(std::string_view host) const {
    while (!host.empty()) {
      auto it = std::lower_bound(values_.begin(), values_.end(), host,
          [](const std::string& a, std::string_view b) { return a < b; });
      if (it != values_.end() && *it == host) return true;
      const auto dot = host.find('.');
      if (dot == std::string_view::npos) break;
      host.remove_prefix(dot + 1);
    }
    return false;
  }
  size_t Size() const { return values_.size(); }
 private: std::vector<std::string> values_;
};
}
'''
compiler=shutil.which('g++') or shutil.which('clang++')
if compiler:
    with tempfile.TemporaryDirectory() as tmp:
        fixture=Path(tmp)
        (fixture/'src/protection').mkdir(parents=True)
        (fixture/'src/protection/domain_rules.h').write_text(header,encoding='utf-8')
        for file in ('adfection_guard.h','always_on_shield.h','download_guard.h'):
            shutil.copy(here/'files/src/protection'/file,fixture/'src/protection'/file)
        for source in ('adfection_guard_test.cpp','always_on_shield_test.cpp','download_guard_test.cpp'):
            exe=fixture/source.replace('.cpp','')
            c=subprocess.run([compiler,'-std=c++20','-Wall','-Wextra','-Werror','-I',str(fixture),str(here/'files/tests'/source),'-o',str(exe)],capture_output=True,text=True)
            if c.returncode:
                print(c.stderr)
            assert c.returncode == 0, f'Compile failed: {source}'
            assert subprocess.run([str(exe)],check=False).returncode==0
            print('PASS: C++20 scoped unit test',source)
else:
    print('WARNING: no portable C++ compiler available')
