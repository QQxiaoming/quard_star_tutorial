(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-74                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:byte is of the right type, when casting from xs:double. :)
(:*******************************************************:)
xs:byte(xs:double("1")) instance of xs:byte