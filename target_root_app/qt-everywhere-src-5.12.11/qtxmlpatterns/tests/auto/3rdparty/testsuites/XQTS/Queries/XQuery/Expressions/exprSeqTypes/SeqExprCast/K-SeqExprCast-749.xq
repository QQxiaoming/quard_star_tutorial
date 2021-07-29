(:*******************************************************:)
(: Test: K-SeqExprCast-749                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:dayTimeDuration as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT2H") castable as xs:untypedAtomic