(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-90                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:int is of the right type, when casting from xs:float. :)
(:*******************************************************:)
xs:int(xs:float("-4")) instance of xs:int