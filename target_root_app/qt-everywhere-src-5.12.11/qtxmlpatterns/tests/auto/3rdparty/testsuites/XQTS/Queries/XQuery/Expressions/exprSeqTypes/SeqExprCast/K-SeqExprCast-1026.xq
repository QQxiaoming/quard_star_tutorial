(:*******************************************************:)
(: Test: K-SeqExprCast-1026                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYear to xs:dateTime isn't allowed. :)
(:*******************************************************:)
xs:gYear("1999") cast as xs:dateTime