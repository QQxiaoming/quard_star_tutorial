(:*******************************************************:)
(: Test: K2-BaseURIProlog-5                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an empty use as base-uri. Since the implementation may fail with determining the base-uri, XPST0001 is allowed. :)
(:*******************************************************:)
declare base-uri "";
fn:static-base-uri()