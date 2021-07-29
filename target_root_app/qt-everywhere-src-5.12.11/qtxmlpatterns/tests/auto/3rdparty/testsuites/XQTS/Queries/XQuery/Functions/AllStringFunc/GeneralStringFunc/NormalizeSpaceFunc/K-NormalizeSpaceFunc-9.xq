(:*******************************************************:)
(: Test: K-NormalizeSpaceFunc-9                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke normalize-space on itself. Implementations supporting the static typing feature may raise XPTY0004. :)
(:*******************************************************:)
normalize-space(normalize-space(("foo", current-time())[1])) eq "foo"