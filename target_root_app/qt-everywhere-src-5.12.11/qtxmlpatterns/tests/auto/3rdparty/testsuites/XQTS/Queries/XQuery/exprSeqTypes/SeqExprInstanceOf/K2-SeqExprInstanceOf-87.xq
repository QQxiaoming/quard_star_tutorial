(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-87                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:nonPositiveInteger is of the right type, when casting from xs:float. :)
(:*******************************************************:)
xs:nonPositiveInteger(xs:float("0")) instance of xs:nonPositiveInteger