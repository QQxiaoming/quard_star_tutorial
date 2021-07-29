(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-86                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:byte is of the right type, when casting from xs:float. :)
(:*******************************************************:)
xs:byte(xs:float("1")) instance of xs:byte