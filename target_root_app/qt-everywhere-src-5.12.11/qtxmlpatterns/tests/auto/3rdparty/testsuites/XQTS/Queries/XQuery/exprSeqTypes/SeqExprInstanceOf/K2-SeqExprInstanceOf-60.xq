(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-60                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:positiveInteger is not a child of negativeInteger. :)
(:*******************************************************:)
xs:positiveInteger("1") instance of xs:negativeInteger