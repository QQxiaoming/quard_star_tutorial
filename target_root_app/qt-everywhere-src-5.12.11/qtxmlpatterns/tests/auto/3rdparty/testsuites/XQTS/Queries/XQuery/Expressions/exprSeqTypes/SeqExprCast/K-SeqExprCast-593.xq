(:*******************************************************:)
(: Test: K-SeqExprCast-593                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:integer as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:integer("6789") castable as xs:untypedAtomic