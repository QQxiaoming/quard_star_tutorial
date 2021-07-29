(:*******************************************************:)
(: Test: K-SeqExprCast-1230                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:yearMonthDuration isn't allowed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:yearMonthDuration