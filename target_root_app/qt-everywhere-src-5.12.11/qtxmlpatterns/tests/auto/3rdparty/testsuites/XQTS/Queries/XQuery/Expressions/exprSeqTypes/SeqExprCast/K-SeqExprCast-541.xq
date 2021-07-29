(:*******************************************************:)
(: Test: K-SeqExprCast-541                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: 'castable as' involving xs:decimal as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:decimal("10.01") castable as xs:untypedAtomic