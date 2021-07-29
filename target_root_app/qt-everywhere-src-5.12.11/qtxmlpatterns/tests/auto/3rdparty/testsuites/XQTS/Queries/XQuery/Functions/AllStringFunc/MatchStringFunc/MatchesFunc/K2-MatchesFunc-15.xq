(:*******************************************************:)
(: Test: K2-MatchesFunc-15                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A negative character class never match a non-character. :)
(:*******************************************************:)
fn:matches("a", "a[^b]"), fn:matches("a", "a[^b]")