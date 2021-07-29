(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-80                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:nonNegativeInteger is of the right type, when casting from xs:double. :)
(:*******************************************************:)
xs:nonNegativeInteger(xs:double("4")) instance of xs:nonNegativeInteger