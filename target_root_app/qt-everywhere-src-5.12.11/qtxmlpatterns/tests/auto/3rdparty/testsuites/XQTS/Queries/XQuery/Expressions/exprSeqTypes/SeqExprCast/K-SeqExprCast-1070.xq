(:*******************************************************:)
(: Test: K-SeqExprCast-1070                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gMonthDay to xs:integer isn't allowed. :)
(:*******************************************************:)
xs:gMonthDay("--11-13") cast as xs:integer