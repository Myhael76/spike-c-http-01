# Spike Feature: Minimal C HTTP Server

Spike feature for a C server used for probes and performance tests on middleware.

- [Spike Feature: Minimal C HTTP Server](#spike-feature-minimal-c-http-server)
  - [Organization](#organization)
    - [Plan from a feedback cycles perspective](#plan-from-a-feedback-cycles-perspective)
    - [File System Organization](#file-system-organization)
  - [Decisions](#decisions)

## Organization

### Plan from a feedback cycles perspective

The software delivery plan looks like the following:

1. Code Authoring

    - Feedback is provided by linters directly in the IDE
    - Use "run" and "debug" buttons
    - Use TDD to explore and make informed decisions

2. Pre-commit phase
  1. Local build
  2. Unit testing: test for regressions using parallel harnesses
  3. Perimeter testing: test for regressions using the compiled artifact

3. CI phase
   1. CI build
   2. CI unit testing
   3. CI perimeter test



### File System Organization

The folders are organized to follow the above plan:

- `01.src` - contains only the source code of the target application.
  - Any client interested in the sources only may sparse checkout this folder
- `02.build` - build code for the application, with all its flavors
  `01.local` - build in the local OS that runs the IDE

## Decisions

- IDE is Visual Studio Code with .devcontainers IaC style environment build
- We want to build both statically and dynamically
- We want to build both with musl and glibc
- We want to package the resulting binary in minimalistic containers
- Target Operating System is linux.
