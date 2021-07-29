(:*******************************************************:)
(: Test: K-SeqExprCast-443                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:float that has the lexical value ' "3.4e5" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:float("3.4e5"))