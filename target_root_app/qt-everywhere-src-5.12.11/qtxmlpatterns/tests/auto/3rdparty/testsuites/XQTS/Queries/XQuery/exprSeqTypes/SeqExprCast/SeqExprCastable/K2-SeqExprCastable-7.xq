(:*******************************************************:)
(: Test: K2-SeqExprCastable-7                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that an invalid xs:anyURI gets flagged as not being castable. :)
(:*******************************************************:)
xs:untypedAtomic("%") castable as xs:anyURI