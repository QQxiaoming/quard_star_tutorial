(:*******************************************************:)
(: Test: K-SeqExprCast-1334                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:hexBinary to xs:yearMonthDuration isn't allowed. :)
(:*******************************************************:)
xs:hexBinary("0FB7") cast as xs:yearMonthDuration