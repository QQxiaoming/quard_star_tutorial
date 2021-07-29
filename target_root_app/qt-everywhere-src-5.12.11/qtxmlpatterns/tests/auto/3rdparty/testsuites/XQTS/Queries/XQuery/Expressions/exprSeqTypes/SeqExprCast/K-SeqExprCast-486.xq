(:*******************************************************:)
(: Test: K-SeqExprCast-486                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:double constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:double(
      "3.3e3"
    ,
                                                     
      "3.3e3"
    )