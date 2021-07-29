(:*******************************************************:)
(: Test: K2-MatchesFunc-4                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Whitespace(in the middle) in the regexp is collapsed, but not inside a character class. :)
(:*******************************************************:)
fn:matches("hello world", "he      ll
                o[ ]worl d", "x")