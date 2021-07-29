(:*******************************************************:)
(: Test: K-SeqExprCast-1258                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:QName isn't allowed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:QName