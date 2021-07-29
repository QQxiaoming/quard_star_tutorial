(:*******************************************************:)
(: Test: K-SeqExprCast-438                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:float constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:float(
      "3.4e5"
    ,
                                                     
      "3.4e5"
    )