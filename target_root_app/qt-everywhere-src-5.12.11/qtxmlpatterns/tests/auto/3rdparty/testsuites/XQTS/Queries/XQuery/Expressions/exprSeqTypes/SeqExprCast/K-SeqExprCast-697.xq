(:*******************************************************:)
(: Test: K-SeqExprCast-697                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:yearMonthDuration as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") castable as xs:untypedAtomic