(:*******************************************************:)
(: Test: K-SeqExprCast-1061                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:gMonthDay as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:gMonthDay("--11-13") castable as xs:untypedAtomic