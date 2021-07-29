(:*******************************************************:)
(: Test: K-SeqExprCast-491                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:double that has the lexical value ' "3.3e3" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:double("3.3e3"))