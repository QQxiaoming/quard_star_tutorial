(:*******************************************************:)
(: Test: K-SeqExprCast-1314                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:hexBinary constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:hexBinary(
      "0FB7"
    ,
                                                     
      "0FB7"
    )