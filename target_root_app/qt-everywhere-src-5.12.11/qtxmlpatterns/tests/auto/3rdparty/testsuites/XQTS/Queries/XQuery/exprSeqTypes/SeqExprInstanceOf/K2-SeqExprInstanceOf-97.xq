(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-97                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:positiveInteger is of the right type, when casting from xs:float. :)
(:*******************************************************:)
xs:positiveInteger(xs:float("4")) instance of xs:positiveInteger