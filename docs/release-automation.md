# Native source release automation

Only successful `KloksCPP portable native release` push runs whose SHA equals
the *current main branch head* may prepare a Release Please version PR
or issue a versioned GitHub source release. CMake reads `version.txt`
as its project version. The release workflow builds and installs the
exported package before any release is created.

Normal releases need no registry token or local credentials. Merge a
Release Please pull request after CI; its merged-main CI run issues
the GitHub tag and source archive. For fully automatic, independently
validated PR merge, configure GitHub App authentication, required status
checks and GitHub auto-merge; GITHUB_TOKEN-generated PRs cannot themselves
trigger new CI runs.

This does not publish a vcpkg registry port or a Conan package. Those
need a separately integrated registry and consumer verification.
