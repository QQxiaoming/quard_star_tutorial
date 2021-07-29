This directory contains an implementation of a simple, sandboxed, Content
Service client application which serves as a tool for manual developer testing
of the Content Service, a straightforward example browser application consuming
the Content Service, and potentially as a target for driving automated
Content Service integration tests.

To play around with simple_browser today, build Chrome with
`target_os = "chromeos"` and run with `--enable-features=Mash` and
`--launch-simple-browser`.
