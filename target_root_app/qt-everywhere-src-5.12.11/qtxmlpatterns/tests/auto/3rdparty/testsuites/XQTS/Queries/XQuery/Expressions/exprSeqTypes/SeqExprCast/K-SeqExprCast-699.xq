(:*******************************************************:)
(: Test: K-SeqExprCast-699                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:yearMonthDuration as source type and xs:string as target type should always evaluate to true. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") castable as xs:string