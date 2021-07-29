(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-91                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:short is of the right type, when casting from xs:float. :)
(:*******************************************************:)
xs:short(xs:float("-4")) instance of xs:short