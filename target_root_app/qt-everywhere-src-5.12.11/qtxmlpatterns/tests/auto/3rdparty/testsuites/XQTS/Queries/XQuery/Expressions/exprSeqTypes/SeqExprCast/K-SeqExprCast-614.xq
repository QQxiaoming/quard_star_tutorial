(:*******************************************************:)
(: Test: K-SeqExprCast-614                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:integer to xs:date isn't allowed. :)
(:*******************************************************:)
xs:integer("6789") cast as xs:date