(:*******************************************************:)
(: Test: K-SeqExprCast-560                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:decimal to xs:time isn't allowed. :)
(:*******************************************************:)
xs:decimal("10.01") cast as xs:time